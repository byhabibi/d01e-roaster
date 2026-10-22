#include "modbus_artisan.h"
#include "modbus_slave.h"
#include "heater_control.h"
#include "air_control.h"
#include "cooling_control.h"
#include "panel_interlock.h"
#include "config.h"
#include <algorithm>

static HardwareSerial s_mbSerial(1);
static uint16_t s_regs[64];
static volatile bool s_cmdStart = false;
static volatile bool s_cmdStop = false;
static volatile bool s_cmdReset = false;
static volatile bool s_heaterUpdated = false;
static volatile bool s_airUpdated = false;
static volatile bool s_coolingUpdated = false;

static uint16_t tempToReg10(float c) {
    if (isnan(c)) {
        return 0;
    }
    int v = (int)lroundf(c * 10.0f);
    if (v < 0) {
        v = 0;
    }
    if (v > 65535) {
        v = 65535;
    }
    return (uint16_t)v;
}

static uint16_t rorToReg10(float ror) {
    if (isnan(ror)) {
        return 0;
    }
    int v = (int)lroundf(ror * 10.0f);
    if (v < -32768) {
        v = -32768;
    }
    if (v > 32767) {
        v = 32767;
    }
    return (uint16_t)(int16_t)v;
}

static uint16_t onRegRead(uint16_t reg) {
    if (reg < 64) {
        return s_regs[reg];
    }
    return 0;
}

static void onRegWrite(uint16_t reg, uint16_t value) {
    if (reg >= 64) {
        return;
    }
    s_regs[reg] = value;

    switch (reg) {
    case MB_REG_AIR:
        s_airUpdated = true;
        if (value > 0) {
            s_regs[MB_REG_MODE] = 1;
        }
        break;
    case MB_REG_HEATER:
        s_heaterUpdated = true;
        if (value > 0) {
            s_regs[MB_REG_MODE] = 1;
        }
        break;
    case MB_REG_COOLING:
        s_coolingUpdated = true;
        break;
    case MB_REG_MODE:
        if (value > 1) {
            s_regs[MB_REG_MODE] = 1;
        }
        break;
    case MB_REG_CMD:
        if (value == MB_CMD_START) {
            s_cmdStart = true;
            s_regs[MB_REG_MODE] = 1;
        } else if (value == MB_CMD_STOP) {
            s_cmdStop = true;
        } else if (value == MB_CMD_RESET) {
            s_cmdReset = true;
        }
        s_regs[MB_REG_CMD] = 0;
        break;
    default:
        break;
    }
}

void modbusArtisanBegin() {
    memset(s_regs, 0, sizeof(s_regs));
    s_mbSerial.begin(MB_BAUD, SERIAL_8N1, MB_RX_PIN, MB_TX_PIN);
    modbusSlaveBegin(s_mbSerial, MB_SLAVE_ID);
    modbusSlaveSetCallbacks(onRegRead, onRegWrite);
    Serial.printf("[MB] RTU slave id=%u %u baud GPIO%d/%d\n",
                  (unsigned)MB_SLAVE_ID, (unsigned)MB_BAUD,
                  (int)MB_RX_PIN, (int)MB_TX_PIN);
}

void modbusArtisanPoll() {
    modbusSlavePoll();
}

void modbusArtisanUpdateTelemetry(float btC, float etC, float btRorCMin,
                                  float etRorCMin, uint32_t roastSec,
                                  uint16_t statusBits) {
    (void)etRorCMin;
    s_regs[MB_REG_BT] = tempToReg10(btC);
    s_regs[MB_REG_ET] = tempToReg10(etC);
    s_regs[MB_REG_BT_ROR] = rorToReg10(btRorCMin);
    if (roastSec > 65535UL) {
        roastSec = 65535UL;
    }
    s_regs[MB_REG_TIME_SEC] = (uint16_t)roastSec;
    s_regs[MB_REG_HEATER] = heaterGetPct();
    s_regs[MB_REG_AIR] = airGetPct();
    s_regs[MB_REG_DRUM] = panelDrumEnabled() ? 1 : 0;
    s_regs[MB_REG_COOLING] = coolingIsOn() ? 1 : 0;
    s_regs[MB_REG_STATUS] = statusBits;
    if (s_regs[MB_REG_MODE] != 0) {
        s_regs[MB_REG_STATUS] |= MB_STATUS_REMOTE;
    }
}

ModbusCmdState modbusArtisanTakeCommands() {
    ModbusCmdState out = {};
    out.remoteActive = (s_regs[MB_REG_MODE] != 0);
    out.heaterPct = (uint8_t)min<uint16_t>(100, s_regs[MB_REG_HEATER]);
    out.airPct = (uint8_t)min<uint16_t>(100, s_regs[MB_REG_AIR]);
    out.coolingOn = (s_regs[MB_REG_COOLING] != 0);
    out.cmdStart = s_cmdStart;
    out.cmdStop = s_cmdStop;
    out.cmdReset = s_cmdReset;
    out.heaterUpdated = s_heaterUpdated;
    out.airUpdated = s_airUpdated;
    out.coolingUpdated = s_coolingUpdated;

    s_cmdStart = false;
    s_cmdStop = false;
    s_cmdReset = false;
    s_heaterUpdated = false;
    s_airUpdated = false;
    s_coolingUpdated = false;
    return out;
}

bool modbusArtisanRemoteActive() {
    return s_regs[MB_REG_MODE] != 0;
}
