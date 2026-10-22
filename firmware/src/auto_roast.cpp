#include "auto_roast.h"
#include "auto_roast_profile.h"
#include "dfplayer_control.h"
#include "config.h"
#include <Arduino.h>
#include <math.h>

static const AutoProfile *s_prof = nullptr;
static AutoRoastPhase s_phase = AUTO_PHASE_IDLE;
static uint32_t s_elapsedSec = 0;
static uint32_t s_lastAudioMs = 0;
static uint8_t s_lastAirPct = 0;
static bool s_sessionActive = false;

static ProfileSegment segmentAt(const AutoProfile &p, uint32_t t) {
    for (uint8_t i = 0; i < p.segmentCount; i++) {
        const ProfileSegment &seg = p.segments[i];
        if (t >= seg.startSec && t < seg.endSec) {
            return seg;
        }
    }
    if (p.segmentCount > 0) {
        return p.segments[p.segmentCount - 1];
    }
    ProfileSegment empty = {0, 0, 0, 0};
    return empty;
}

static void playChargeAudio(uint32_t nowMs) {
    if (s_lastAudioMs != 0 && (nowMs - s_lastAudioMs) < AUTO_AUDIO_INTERVAL_MS) {
        return;
    }
    s_lastAudioMs = nowMs;
    dfplayerPlayTrack(DF_TRACK_PREPARE_CHARGE);
}

static void playDropAudio(uint32_t nowMs) {
    if (s_lastAudioMs != 0 && (nowMs - s_lastAudioMs) < AUTO_AUDIO_INTERVAL_MS) {
        return;
    }
    s_lastAudioMs = nowMs;
    dfplayerPlayTrack(DF_TRACK_PREPARE_DROP);
}

void autoRoastInit() {
    autoRoastUnload();
}

void autoRoastLoadPage(uint8_t pageNum) {
    s_prof = autoProfileGetByPage(pageNum);
    autoRoastOnReset();
    if (s_prof) {
        Serial.printf("[AUTO] profile page%u target=%us charge=%uC\n",
                      (unsigned)pageNum,
                      (unsigned)s_prof->targetSec,
                      (unsigned)s_prof->chargeBtC);
    } else {
        Serial.printf("[AUTO] unknown page %u\n", (unsigned)pageNum);
    }
}

void autoRoastUnload() {
    s_prof = nullptr;
    autoRoastOnReset();
}

bool autoRoastHasProfile() {
    return s_prof != nullptr;
}

uint8_t autoRoastPage() {
    return s_prof ? s_prof->pageNum : 0;
}

AutoRoastPhase autoRoastPhase() {
    return s_phase;
}

RoastUiStatus autoRoastGetUiStatus(float btC) {
    if (!s_prof || s_phase == AUTO_PHASE_IDLE) {
        return ROAST_UI_IDLE;
    }

    switch (s_phase) {
    case AUTO_PHASE_PREHEAT_COOL:
        return ROAST_UI_COOLING_DOWN;

    case AUTO_PHASE_PREHEAT_HEAT: {
        const uint16_t charge = s_prof->chargeBtC;
        if (charge > 0 && !isnan(btC)) {
            float warn = (float)charge - (float)AUTO_CHARGE_WARN_C;
            if (btC >= warn && btC < (float)charge) {
                return ROAST_UI_INPUT_BEAN;
            }
        }
        return ROAST_UI_WARMING_UP;
    }

    case AUTO_PHASE_ROASTING:
        return ROAST_UI_ROASTING;

    case AUTO_PHASE_FINISHED:
        return ROAST_UI_DONE;

    default:
        return ROAST_UI_IDLE;
    }
}

void autoRoastOnStart(float btC) {
    if (!s_prof) {
        return;
    }
    s_sessionActive = true;
    s_elapsedSec = 0;
    s_lastAudioMs = 0;
    dfplayerStop();

    if (s_prof->chargeBtC == 0) {
        s_phase = AUTO_PHASE_ROASTING;
        Serial.println(F("[AUTO] START — roast timer (no charge BT)"));
        return;
    }

    if (!isnan(btC) && btC > (float)s_prof->chargeBtC) {
        s_phase = AUTO_PHASE_PREHEAT_COOL;
        Serial.printf("[AUTO] START — cool to charge %uC (BT=%.0f)\n",
                      (unsigned)s_prof->chargeBtC, btC);
    } else {
        s_phase = AUTO_PHASE_PREHEAT_HEAT;
        Serial.printf("[AUTO] START — preheat to charge %uC\n",
                      (unsigned)s_prof->chargeBtC);
    }
}

void autoRoastOnStop() {
    s_sessionActive = false;
    s_phase = AUTO_PHASE_IDLE;
    dfplayerStop();
    Serial.println(F("[AUTO] STOP"));
}

void autoRoastOnReset() {
    s_sessionActive = false;
    s_phase = AUTO_PHASE_IDLE;
    s_elapsedSec = 0;
    s_lastAudioMs = 0;
    s_lastAirPct = 0;
    dfplayerStop();
}

uint32_t autoRoastElapsedSec() {
    return s_elapsedSec;
}

AutoRoastOutputs autoRoastTick(float btC, uint32_t nowMs) {
    AutoRoastOutputs out;
    if (!s_prof || !s_sessionActive) {
        return out;
    }

    out.sessionActive = true;
    const AutoProfile &p = *s_prof;
    const uint16_t charge = p.chargeBtC;

    if (s_phase == AUTO_PHASE_PREHEAT_COOL) {
        out.heaterPct = 0;
        out.airPct = 100;
        if (!isnan(btC) && btC <= (float)charge) {
            s_phase = AUTO_PHASE_ROASTING;
            s_elapsedSec = 0;
            s_lastAudioMs = 0;
            dfplayerStop();
            Serial.println(F("[AUTO] charge temp reached — roast timer start"));
        }
        return out;
    }

    if (s_phase == AUTO_PHASE_PREHEAT_HEAT) {
        ProfileSegment seg = p.segments[0];
        out.heaterPct = seg.heaterPct;
        out.airPct = seg.airPct;

        if (charge > 0 && !isnan(btC)) {
            float warn = (float)charge - (float)AUTO_CHARGE_WARN_C;
            if (btC >= warn && btC < (float)charge) {
                playChargeAudio(nowMs);
            }
            if (btC >= (float)charge) {
                s_phase = AUTO_PHASE_ROASTING;
                s_elapsedSec = 0;
                s_lastAudioMs = 0;
                dfplayerStop();
                Serial.println(F("[AUTO] charge BT reached — roast timer start"));
            }
        }
        return out;
    }

    if (s_phase == AUTO_PHASE_ROASTING) {
        if (s_elapsedSec >= p.targetSec) {
            s_phase = AUTO_PHASE_FINISHED;
            out.heaterPct = 0;
            out.airPct = s_lastAirPct;
            out.coolingFan = true;
            out.finished = true;
            s_sessionActive = false;
            dfplayerStop();
            Serial.println(F("[AUTO] profile complete — heater OFF, cooling ON"));
            return out;
        }

        ProfileSegment seg = segmentAt(p, s_elapsedSec);
        out.heaterPct = seg.heaterPct;
        out.airPct = seg.airPct;
        s_lastAirPct = seg.airPct;

        if (s_elapsedSec + AUTO_DROP_WARN_SEC >= p.targetSec) {
            out.coolingFan = true;
            playDropAudio(nowMs);
        }

        s_elapsedSec++;
        return out;
    }

    if (s_phase == AUTO_PHASE_FINISHED) {
        out.heaterPct = 0;
        out.airPct = s_lastAirPct;
        out.coolingFan = true;
        return out;
    }

    return out;
}

bool autoRoastPostFinishActive() {
    return s_prof != nullptr && s_phase == AUTO_PHASE_FINISHED;
}

AutoRoastOutputs autoRoastHoldOutputs() {
    AutoRoastOutputs out;
    if (s_phase == AUTO_PHASE_FINISHED) {
        out.heaterPct = 0;
        out.airPct = s_lastAirPct;
        out.coolingFan = true;
    }
    return out;
}
