/**
 * D01E Roaster — Roast pages (page2 + page16..27), warm-up, settings
 */
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_MAX31856.h>
#include "config.h"
#include "nextion_roast.h"
#include "heater_control.h"
#include "air_control.h"
#include "cooling_control.h"
#include "color_sensor_control.h"
#include "panel_interlock.h"
#include "dfplayer_control.h"
#include "temp_limit_storage.h"
#include "settings_storage.h"
#include "auto_roast.h"
#include "modbus_artisan.h"

Adafruit_MAX31856 tcBean(TC_CS_BEAN, &SPI);
Adafruit_MAX31856 tcEnv(TC_CS_ENV, &SPI);
NextionRoast nextion;

// --- roast state (page2 + page16..27) ---
static bool roastRunning = false;
static uint32_t roastElapsedSec = 0;
static uint32_t lastTickMs = 0;

// --- warm-up state (page4) ---
static bool page4Active = false;
static bool warmupRunning = false;
static uint32_t warmupElapsedSec = 0;
static uint32_t lastWarmupAudioMs = 0;

// --- RoR ring buffer (15 s window) ---
struct TempSample {
    uint32_t ms;
    float c;
};
static constexpr size_t ROR_MAX = 32;
static TempSample btHist[ROR_MAX];
static TempSample etHist[ROR_MAX];
static size_t btCount = 0;
static size_t etCount = 0;
static bool heaterSafetyOk = false;
static bool roastPageActive = false;
static bool manualRoastPage = false;
static bool batchCountedThisRoast = false;
static AutoRoastOutputs s_autoOut;
static bool manualCoolOn = false;
static bool autoCoolOn = false;
static bool modbusCoolOn = false;
static bool modbusRemoteActive = false;
static RoastUiStatus s_lastUiStatus = ROAST_UI_IDLE;
static bool s_autoCoolFanUiOn = false;

static const int Y_AXIS_C_VALS[6] = {0, 50, 100, 150, 200, 250};
static const char *Y_AXIS_OBJS[6] = {
    NX_OBJ_Y0, NX_OBJ_Y1, NX_OBJ_Y2, NX_OBJ_Y3, NX_OBJ_Y4, NX_OBJ_Y5
};

static float celsiusToFahrenheit(float c) {
    return c * 9.0f / 5.0f + 32.0f;
}

static void pushSample(TempSample *buf, size_t &count, float tempC) {
    uint32_t now = millis();
    if (count < ROR_MAX) {
        buf[count++] = {now, tempC};
    } else {
        for (size_t i = 1; i < count; i++) {
            buf[i - 1] = buf[i];
        }
        buf[count - 1] = {now, tempC};
    }
}

static float calcRorCPerMin(const TempSample *buf, size_t count) {
    if (count < 2) return 0.0f;
    const TempSample &latest = buf[count - 1];
    uint32_t cutoff = (latest.ms > ROR_WINDOW_MS) ? (latest.ms - ROR_WINDOW_MS) : 0;

    size_t i = 0;
    while (i < count && buf[i].ms < cutoff) i++;
    if (i >= count - 1) i = (count > 2) ? count - 3 : 0;

    const TempSample &older = buf[i];
    float dtMin = (latest.ms - older.ms) / 60000.0f;
    if (dtMin < 0.05f) return 0.0f;
    return (latest.c - older.c) / dtMin;
}

static void clearHistory() {
    btCount = etCount = 0;
}

static void formatTempCelsius(char *out, size_t len, float c) {
    if (isnan(c)) {
        snprintf(out, len, "---");
    } else {
        snprintf(out, len, "%d", (int)lroundf(c));
    }
}

static void formatTemp(char *out, size_t len, float c) {
    if (isnan(c)) {
        snprintf(out, len, "---");
        return;
    }
    if (settingsGetUseFahrenheit()) {
        snprintf(out, len, "%d", (int)lroundf(celsiusToFahrenheit(c)));
    } else {
        snprintf(out, len, "%d", (int)lroundf(c));
    }
}

static void formatRor(char *out, size_t len, float rorCPerMin) {
    if (isnan(rorCPerMin) || fabsf(rorCPerMin) < 0.5f) {
        snprintf(out, len, "0");
        return;
    }
    float ror = rorCPerMin;
    if (settingsGetUseFahrenheit()) {
        ror = rorCPerMin * 9.0f / 5.0f;
    }
    snprintf(out, len, "%+d", (int)lroundf(ror));
}

static void updateYAxisLabels() {
    char buf[8];
    bool useF = settingsGetUseFahrenheit();
    for (int i = 0; i < 6; i++) {
        if (useF) {
            snprintf(buf, sizeof(buf), "%d",
                     (int)lroundf(celsiusToFahrenheit((float)Y_AXIS_C_VALS[i])));
        } else {
            snprintf(buf, sizeof(buf), "%d", Y_AXIS_C_VALS[i]);
        }
        nextion.setText(Y_AXIS_OBJS[i], buf);
    }
}

static void syncBatchDisplay() {
    char buf[8];
    settingsFormatBatch(buf, sizeof(buf));
    nextion.setText(NX_OBJ_BATCH, buf);
}

static void syncUnitCheckboxes() {
    bool useF = settingsGetUseFahrenheit();
    nextion.setVal(NX_OBJ_UNIT_C, useF ? 0 : 1);
    nextion.setVal(NX_OBJ_UNIT_F, useF ? 1 : 0);
}

static float readTcC(Adafruit_MAX31856 &chip) {
    float c = chip.readThermocoupleTemperature();
    if (chip.readFault()) {
        chip.clearFault();
        return NAN;
    }
    return c;
}

static uint8_t scaleTempToY(float tempC) {
    if (isnan(tempC)) {
        return 0;
    }
    float plotVal = tempC;
    int plotMax = PLOT_TEMP_MAX_C;
    if (settingsGetUseFahrenheit()) {
        plotVal = celsiusToFahrenheit(tempC);
        plotMax = PLOT_TEMP_MAX_F;
    }
    int t = (int)lroundf(plotVal);
    if (t < 0) {
        t = 0;
    }
    if (t > plotMax) {
        t = plotMax;
    }
    return (uint8_t)((uint32_t)t * 255U / (uint32_t)plotMax);
}

static uint8_t scaleRorToY(float rorCPerMin) {
    if (isnan(rorCPerMin)) {
        return 0;
    }
    float plotVal = rorCPerMin;
    int plotMax = PLOT_ROR_MAX;
    if (settingsGetUseFahrenheit()) {
        plotVal = rorCPerMin * 9.0f / 5.0f;
        plotMax = PLOT_ROR_MAX_F;
    }
    int r = (int)lroundf(plotVal);
    if (r < 0) {
        r = 0;
    }
    if (r > plotMax) {
        r = plotMax;
    }
    return (uint8_t)((uint32_t)r * 255U / (uint32_t)plotMax);
}

static void plotRoastCurves(float bt, float et, float btRor) {
    nextion.addWaveform(WAVE_CH_BT, scaleTempToY(bt));
    nextion.addWaveform(WAVE_CH_ET, scaleTempToY(et));
    nextion.addWaveform(WAVE_CH_ROR_BT, scaleRorToY(btRor));
}

static void updateDisplay(float bt, float et, float btRor, float etRor) {
    char buf[16];
    uint32_t displaySec = roastElapsedSec;

    if (autoRoastHasProfile()) {
        if (autoRoastPhase() == AUTO_PHASE_ROASTING ||
            autoRoastPhase() == AUTO_PHASE_FINISHED) {
            displaySec = autoRoastElapsedSec();
        } else {
            displaySec = 0;
        }
    }

    if (roastRunning || autoRoastPostFinishActive()) {
        nextion.setTimerMmSs(NX_OBJ_TIMER, displaySec);
    }

    formatTemp(buf, sizeof(buf), bt);
    nextion.setText(NX_OBJ_BT, buf);

    formatRor(buf, sizeof(buf), btRor);
    nextion.setText(NX_OBJ_BT_ROR, buf);

    formatTemp(buf, sizeof(buf), et);
    nextion.setText(NX_OBJ_ET, buf);

    formatRor(buf, sizeof(buf), etRor);
    nextion.setText(NX_OBJ_ET_ROR, buf);
}

static void updateWarmupDisplay(float bt) {
    char buf[16];

    nextion.setTimerMmSs(NX_OBJ_WU_TIMER, warmupElapsedSec);
    formatTempCelsius(buf, sizeof(buf), bt);
    nextion.setText(NX_OBJ_WU_BT, buf);
}

static void onAirDisplayPct(uint8_t pct) {
    (void)pct;
    // t5 = sumbu Y pada layout baru; pot air tetap PWM tanpa label HMI
}

static void stopWarmupOutputs() {
    warmupRunning = false;
    heaterSetPct(0);
    heaterApply(false);
    airSetOverride(false, 0);
    dfplayerStop();
}

static void onWarmupComplete() {
    Serial.println(F("[WARMUP] target reached — page2"));
    page4Active = false;
    stopWarmupOutputs();
    nextion.setPage("page2");
}

static void onPage4Leave() {
    page4Active = false;
    stopWarmupOutputs();
    Serial.println(F("[PAGE4] leave — heater/air OFF"));
}

static void onPage4Enter() {
    roastPageActive = false;
    manualRoastPage = false;
    autoRoastUnload();
    page4Active = true;
    coolingSet(false);
    colorSensorSet(false);
    airSetEnabled(false);
    warmupElapsedSec = 0;
    lastWarmupAudioMs = 0;

    float bt = readTcC(tcBean);
    nextion.setTimerMmSs(NX_OBJ_WU_TIMER, 0);
    updateWarmupDisplay(bt);

    if (!isnan(bt) && bt >= (float)WARMUP_TARGET_C) {
        Serial.println(F("[WARMUP] BT already hot — skip to page2"));
        page4Active = false;
        nextion.setPage("page2");
        return;
    }

    warmupRunning = true;
    heaterSetPct(WARMUP_HEATER_PCT);
    airSetOverride(true, WARMUP_AIR_PCT);
    Serial.println(F("[PAGE4] warm-up started heater=100% air=60%"));
}

static void onSettingsPageEnter() {
    syncBatchDisplay();
    syncUnitCheckboxes();
    Serial.println(F("[PAGE6] settings sync batch + unit"));
}

static void onUnitSet(bool fahrenheit) {
    settingsSetUseFahrenheit(fahrenheit);
    if (roastPageActive) {
        updateYAxisLabels();
    }
    Serial.printf("[PAGE6] unit -> %s\n", fahrenheit ? "F" : "C");
}

static void onLimitPageEnter() {
    char buf[8];
    snprintf(buf, sizeof(buf), "%u", (unsigned)tempLimitGet());
    nextion.setText(NX_OBJ_LIMIT, buf);
    Serial.printf("[PAGE5] limit display %s C (NVS)\n", buf);
}

static void applyCoolingFan() {
    bool on = manualCoolOn || autoCoolOn || modbusCoolOn;
    coolingSet(on);
    if (roastPageActive && autoCoolOn) {
        if (!s_autoCoolFanUiOn) {
            nextion.setVal(NX_OBJ_COOL_FAN, 1);
            s_autoCoolFanUiOn = true;
        }
    } else if (s_autoCoolFanUiOn) {
        s_autoCoolFanUiOn = false;
    }
}

static void clearAutoRoastStatusUi() {
    s_lastUiStatus = ROAST_UI_IDLE;
    nextion.setVal(NX_OBJ_STATUS_COOL, 0);
    nextion.setVal(NX_OBJ_STATUS_WARM, 0);
    nextion.setVal(NX_OBJ_STATUS_CHARGE, 0);
    nextion.setVal(NX_OBJ_STATUS_ROAST, 0);
    nextion.setVal(NX_OBJ_STATUS_DONE, 0);
    nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_IDLE);
    nextion.setEn(NX_OBJ_TM_BLINK_CHARGE, 0);
    nextion.setEn(NX_OBJ_TM_BLINK_DONE, 0);
}

static void applyAutoRoastStatusUi(RoastUiStatus st) {
    if (st == s_lastUiStatus) {
        return;
    }
    s_lastUiStatus = st;

    nextion.setVal(NX_OBJ_STATUS_COOL, 0);
    nextion.setVal(NX_OBJ_STATUS_WARM, 0);
    nextion.setVal(NX_OBJ_STATUS_CHARGE, 0);
    nextion.setVal(NX_OBJ_STATUS_ROAST, 0);
    nextion.setVal(NX_OBJ_STATUS_DONE, 0);
    nextion.setEn(NX_OBJ_TM_BLINK_CHARGE, 0);
    nextion.setEn(NX_OBJ_TM_BLINK_DONE, 0);

    switch (st) {
    case ROAST_UI_IDLE:
        nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_IDLE);
        break;
    case ROAST_UI_COOLING_DOWN:
        nextion.setVal(NX_OBJ_STATUS_COOL, 1);
        nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_COOL);
        break;
    case ROAST_UI_WARMING_UP:
        nextion.setVal(NX_OBJ_STATUS_WARM, 1);
        nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_WARM);
        break;
    case ROAST_UI_INPUT_BEAN:
        nextion.setVal(NX_OBJ_STATUS_CHARGE, 1);
        nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_CHARGE);
        nextion.setEn(NX_OBJ_TM_BLINK_CHARGE, 1);
        break;
    case ROAST_UI_ROASTING:
        nextion.setVal(NX_OBJ_STATUS_ROAST, 1);
        nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_ROAST);
        break;
    case ROAST_UI_DONE:
        nextion.setVal(NX_OBJ_STATUS_DONE, 1);
        nextion.setPic(NX_OBJ_STATUS_BANNER, NX_PIC_BANNER_DONE);
        nextion.setEn(NX_OBJ_TM_BLINK_DONE, 1);
        break;
    }
}

static void updateAutoRoastStatusUi(float bt) {
    if (!autoRoastHasProfile() || manualRoastPage) {
        return;
    }
    if (!roastRunning && !autoRoastPostFinishActive()) {
        applyAutoRoastStatusUi(ROAST_UI_IDLE);
        return;
    }
    applyAutoRoastStatusUi(autoRoastGetUiStatus(bt));
}

static void resetCoolingState() {
    manualCoolOn = false;
    autoCoolOn = false;
    modbusCoolOn = false;
    s_autoCoolFanUiOn = false;
    coolingSet(false);
}

static uint16_t buildModbusStatus() {
    uint16_t st = 0;
    if (roastRunning) {
        st |= MB_STATUS_ROASTING;
    }
    if (panelIsSafe()) {
        st |= MB_STATUS_ESTOP_OK;
    }
    if (panelDrumEnabled()) {
        st |= MB_STATUS_DRUM_BTN;
    }
    if (panelHeaterEnabled()) {
        st |= MB_STATUS_HEAT_BTN;
    }
    if (panelAirEnabled()) {
        st |= MB_STATUS_AIR_BTN;
    }
    if (roastPageActive) {
        st |= MB_STATUS_ROAST_PG;
    }
    return st;
}

static void applyModbusCommands() {
    ModbusCmdState mb = modbusArtisanTakeCommands();
    modbusRemoteActive = mb.remoteActive;

    if (!roastPageActive) {
        return;
    }

    if (mb.cmdStart) {
        onStart();
    }
    if (mb.cmdStop) {
        onStop();
    }
    if (mb.cmdReset) {
        onReset();
    }
    if (mb.coolingUpdated) {
        modbusCoolOn = mb.coolingOn;
        applyCoolingFan();
    }
    if (mb.remoteActive && mb.heaterUpdated && (manualRoastPage || !autoRoastHasProfile())) {
        heaterSetPct(mb.heaterPct);
    }
    if (mb.remoteActive && mb.airUpdated && (manualRoastPage || !autoRoastHasProfile())) {
        airSetEnabled(true);
        airSetOverride(true, mb.airPct);
    }
}

static void stopAllRoastOutputs() {
    heaterSetPct(0);
    heaterApply(false);
    airSetOverride(false, 0);
    resetCoolingState();
    colorSensorSet(false);
    airSetEnabled(false);
    dfplayerStop();
}

static void onRoastPageLeave() {
    roastPageActive = false;
    manualRoastPage = false;
    onStop();
    autoRoastOnStop();
    autoRoastUnload();
    clearAutoRoastStatusUi();
    stopAllRoastOutputs();
    Serial.println(F("[ROAST] leave — all outputs OFF"));
}

static void onManualRoastEnter() {
    page4Active = false;
    roastPageActive = true;
    manualRoastPage = true;
    autoRoastUnload();
    resetCoolingState();
    airSetEnabled(true);
    updateYAxisLabels();
    Serial.println(F("[ROAST] page2 manual enter"));
}

static void onAutoRoastEnter(uint8_t pageNum) {
    page4Active = false;
    roastPageActive = true;
    manualRoastPage = false;
    resetCoolingState();
    clearAutoRoastStatusUi();
    autoRoastLoadPage(pageNum);
    airSetEnabled(false);
    updateYAxisLabels();
    Serial.printf("[ROAST] auto page%u enter\n", (unsigned)pageNum);
}

static void onStart() {
    roastRunning = true;
    batchCountedThisRoast = false;
    lastTickMs = millis();

    if (autoRoastHasProfile()) {
        float bt = readTcC(tcBean);
        autoRoastOnStart(bt);
        updateAutoRoastStatusUi(bt);
        Serial.println(F("[ROAST] AUTO START"));
    } else {
        Serial.println(F("[ROAST] MANUAL START"));
    }
}

static void onStop() {
    roastRunning = false;
    autoCoolOn = false;
    if (autoRoastHasProfile()) {
        autoRoastOnStop();
        if (!autoRoastPostFinishActive()) {
            airSetOverride(false, 0);
            clearAutoRoastStatusUi();
        }
    }
    applyCoolingFan();
    Serial.println(F("[ROAST] STOP"));
}

static void onReset() {
    roastRunning = false;
    batchCountedThisRoast = false;
    roastElapsedSec = 0;
    if (autoRoastHasProfile()) {
        autoRoastOnReset();
    }
    clearHistory();
    nextion.clearWaveform(255);
    nextion.setTimerMmSs(NX_OBJ_TIMER, 0);
    nextion.setText(NX_OBJ_BT, "0");
    nextion.setText(NX_OBJ_BT_ROR, "0");
    nextion.setText(NX_OBJ_ET, "0");
    nextion.setText(NX_OBJ_ET_ROR, "0");
    if (!autoRoastPostFinishActive()) {
        airSetOverride(false, 0);
    }
    resetCoolingState();
    if (autoRoastHasProfile()) {
        clearAutoRoastStatusUi();
    }
    Serial.println(F("[ROAST] RESET"));
}

static void tickBatchCount() {
    if (batchCountedThisRoast) {
        return;
    }
    uint32_t sec = roastElapsedSec;
    if (autoRoastHasProfile()) {
        if (autoRoastPhase() != AUTO_PHASE_ROASTING &&
            autoRoastPhase() != AUTO_PHASE_FINISHED) {
            return;
        }
        sec = autoRoastElapsedSec();
    } else if (!roastRunning) {
        return;
    }
    if (sec >= BATCH_MIN_SEC) {
        batchCountedThisRoast = true;
        settingsIncrementBatch();
        syncBatchDisplay();
    }
}

static void tickWarmupAudio(float bt, uint32_t nowMs) {
    if (!warmupRunning || isnan(bt)) {
        return;
    }
    if (bt < (float)WARMUP_NEAR_C || bt >= (float)WARMUP_TARGET_C) {
        return;
    }
    if (lastWarmupAudioMs != 0 &&
        (nowMs - lastWarmupAudioMs) < WARMUP_AUDIO_INTERVAL_MS) {
        return;
    }
    lastWarmupAudioMs = nowMs;
    dfplayerPlayTrack(DF_TRACK_WARMUP_NEAR);
}

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.printf("D01E Roaster v%u.%02u — roast pages 2/16-27, warm-up, settings\n",
                  (unsigned)((FW_VERSION_BCD >> 8) & 0xFF),
                  (unsigned)(FW_VERSION_BCD & 0xFF));

    nextion.begin(NX_SERIAL, NX_BAUD);
    tempLimitBegin();
    settingsBegin();
    heaterBegin();
    airBegin();
    coolingBegin();
    panelBegin();
    colorSensorBegin();
    dfplayerBegin();
    autoRoastInit();
    modbusArtisanBegin();

    SPI.begin(TC_SPI_SCK, TC_SPI_MISO, TC_SPI_MOSI);

    if (!tcBean.begin()) {
        Serial.println(F("MAX31856 Bean (U4) not found"));
    } else {
        tcBean.setThermocoupleType(MAX31856_TCTYPE_K);
    }

    if (!tcEnv.begin()) {
        Serial.println(F("MAX31856 Env (U5) not found"));
    } else {
        tcEnv.setThermocoupleType(MAX31856_TCTYPE_K);
    }

    nextion.clearWaveform(255);
    nextion.setTimerMmSs(NX_OBJ_TIMER, 0);
    nextion.setText(NX_OBJ_BT, "0");
    nextion.setText(NX_OBJ_BT_ROR, "0");
    nextion.setText(NX_OBJ_ET, "0");
    nextion.setText(NX_OBJ_ET_ROR, "0");
    airPoll(millis(), nullptr);
}

void loop() {
    nextion.poll();
    panelPoll();
    modbusArtisanPoll();
    applyModbusCommands();
    airApplyPanelGate();

    if (nextion.pageLeavePressed()) {
        onRoastPageLeave();
        nextion.clearPageFlags();
    }
    if (nextion.pageEnterPressed()) {
        onManualRoastEnter();
        nextion.clearPageFlags();
    }
    if (nextion.autoPageEnterPressed()) {
        onAutoRoastEnter(nextion.takeAutoPageNum());
        nextion.clearPageFlags();
    }
    if (nextion.warmupLeavePressed()) {
        onPage4Leave();
        nextion.clearPageFlags();
    }
    if (nextion.warmupEnterPressed()) {
        onPage4Enter();
        nextion.clearPageFlags();
    }
    if (nextion.limitPageEnterPressed()) {
        onLimitPageEnter();
        nextion.clearPageFlags();
    }
    if (nextion.tempLimitSavePressed()) {
        tempLimitSet(nextion.takeTempLimitC());
    }
    if (nextion.settingsPageEnterPressed()) {
        onSettingsPageEnter();
        nextion.clearPageFlags();
    }
    if (nextion.unitSetUpdated()) {
        onUnitSet(nextion.takeUnitFahrenheit());
    }

    if (roastPageActive && nextion.startPressed()) {
        onStart();
        nextion.clearFlags();
    }
    if (roastPageActive && nextion.stopPressed()) {
        onStop();
        nextion.clearFlags();
    }
    if (roastPageActive && nextion.resetPressed()) {
        onReset();
        nextion.clearFlags();
    }
    if (roastPageActive && nextion.heatPctUpdated() && manualRoastPage) {
        heaterSetPct(nextion.takeHeatPct());
        heaterApply(heaterSafetyOk);
    }
    if (roastPageActive && nextion.coolStateUpdated()) {
        manualCoolOn = nextion.takeCoolOn();
        applyCoolingFan();
    }
    if (roastPageActive && nextion.colorStateUpdated()) {
        colorSensorSet(nextion.takeColorOn());
    }

    uint32_t now = millis();
    bool autoSession = autoRoastHasProfile() &&
                       (roastRunning || autoRoastPostFinishActive());

    if (now - lastTickMs >= ROAST_TICK_MS) {
        lastTickMs = now;

        float bt = readTcC(tcBean);
        float et = readTcC(tcEnv);

        heaterSafetyOk = !isnan(bt) && tempLimitIsSafe(bt);

        pushSample(btHist, btCount, bt);
        pushSample(etHist, etCount, et);

        float btRor = calcRorCPerMin(btHist, btCount);
        float etRor = calcRorCPerMin(etHist, etCount);

        if (autoRoastHasProfile() && roastRunning) {
            s_autoOut = autoRoastTick(bt, now);
            if (s_autoOut.finished) {
                roastRunning = false;
            }
            tickBatchCount();
        } else if (autoRoastPostFinishActive()) {
            s_autoOut = autoRoastHoldOutputs();
            tickBatchCount();
        } else if (roastRunning && manualRoastPage) {
            roastElapsedSec++;
            tickBatchCount();
        }

        if (warmupRunning) {
            warmupElapsedSec++;
        }

        if (roastPageActive) {
            updateDisplay(bt, et, btRor, etRor);
            updateAutoRoastStatusUi(bt);
        }

        bool plotActive = (roastRunning && manualRoastPage) ||
                          (autoRoastHasProfile() &&
                           (roastRunning || autoRoastPostFinishActive()));
        if (roastPageActive && plotActive) {
            plotRoastCurves(bt, et, btRor);
        }

        if (page4Active && warmupRunning) {
            updateWarmupDisplay(bt);
            tickWarmupAudio(bt, now);
            if (!isnan(bt) && bt >= (float)WARMUP_TARGET_C) {
                onWarmupComplete();
            }
        }

        uint32_t mbTimeSec = roastElapsedSec;
        if (autoRoastHasProfile() &&
            (autoRoastPhase() == AUTO_PHASE_ROASTING ||
             autoRoastPhase() == AUTO_PHASE_FINISHED)) {
            mbTimeSec = autoRoastElapsedSec();
        }
        modbusArtisanUpdateTelemetry(bt, et, btRor, etRor, mbTimeSec, buildModbusStatus());
    }

    if (page4Active && warmupRunning) {
        heaterApply(true);
    } else if (autoSession) {
        if (autoRoastPostFinishActive()) {
            s_autoOut = autoRoastHoldOutputs();
        }
        autoCoolOn = s_autoOut.coolingFan;
        heaterSetPct(s_autoOut.heaterPct);
        airSetOverride(true, s_autoOut.airPct);
        applyCoolingFan();
        heaterApply(heaterSafetyOk && s_autoOut.heaterPct > 0);
    } else if (roastPageActive && (manualRoastPage || modbusRemoteActive) && !autoSession) {
        autoCoolOn = false;
        applyCoolingFan();
        heaterApply(heaterSafetyOk);
        if (manualRoastPage && !modbusRemoteActive) {
            airPoll(now, onAirDisplayPct);
        }
    } else {
        heaterApply(false);
        if (!autoRoastPostFinishActive()) {
            airPoll(now, nullptr);
        }
    }
}
