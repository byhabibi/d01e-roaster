#pragma once

#include <stdint.h>

enum AutoRoastPhase : uint8_t {
    AUTO_PHASE_IDLE = 0,
    AUTO_PHASE_PREHEAT_COOL,
    AUTO_PHASE_PREHEAT_HEAT,
    AUTO_PHASE_ROASTING,
    AUTO_PHASE_FINISHED,
};

/** HMI status bar — mutual exclusive (bt2..bt6 + p3 banner). */
enum RoastUiStatus : uint8_t {
    ROAST_UI_IDLE = 0,
    ROAST_UI_COOLING_DOWN,
    ROAST_UI_WARMING_UP,
    ROAST_UI_INPUT_BEAN,
    ROAST_UI_ROASTING,
    ROAST_UI_DONE,
};

struct AutoRoastOutputs {
    uint8_t heaterPct = 0;
    uint8_t airPct = 0;
    bool coolingFan = false;
    bool sessionActive = false;
    bool finished = false;
};

void autoRoastInit();
void autoRoastLoadPage(uint8_t pageNum);
void autoRoastUnload();
bool autoRoastHasProfile();
uint8_t autoRoastPage();
AutoRoastPhase autoRoastPhase();
RoastUiStatus autoRoastGetUiStatus(float btC);

void autoRoastOnStart(float btC);
void autoRoastOnStop();
void autoRoastOnReset();

AutoRoastOutputs autoRoastTick(float btC, uint32_t nowMs);
uint32_t autoRoastElapsedSec();
bool autoRoastPostFinishActive();
AutoRoastOutputs autoRoastHoldOutputs();
