#pragma once

#include <stdint.h>

struct ProfileSegment {
    uint16_t startSec;
    uint16_t endSec;
    uint8_t heaterPct;
    uint8_t airPct;
};

struct AutoProfile {
    uint8_t pageNum;
    uint16_t chargeBtC;   // 0 = no preheat; timer starts at START
    uint16_t targetSec;   // exact stop time
    const ProfileSegment *segments;
    uint8_t segmentCount;
};

const AutoProfile *autoProfileGetByPage(uint8_t pageNum);
