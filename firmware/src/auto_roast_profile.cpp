#include "auto_roast_profile.h"

static const ProfileSegment kFast1000Light[] = {
    {0, 480, 100, 50},
    {480, 540, 100, 100},
};
static const ProfileSegment kFast1000Med[] = {
    {0, 360, 100, 50},
    {360, 540, 90, 60},
    {540, 630, 80, 100},
};
static const ProfileSegment kFast1000Dark[] = {
    {0, 300, 100, 50},
    {300, 540, 85, 60},
    {540, 660, 75, 80},
    {660, 720, 65, 100},
};
static const ProfileSegment kFast500Light[] = {
    {0, 300, 90, 50},
    {300, 420, 80, 60},
    {420, 480, 70, 100},
};
static const ProfileSegment kFast500Med[] = {
    {0, 300, 90, 50},
    {300, 480, 75, 60},
    {480, 570, 65, 100},
};
static const ProfileSegment kFast500Dark[] = {
    {0, 240, 90, 50},
    {240, 480, 70, 60},
    {480, 600, 60, 80},
    {600, 660, 55, 100},
};
static const ProfileSegment kSlow1000Light[] = {
    {0, 360, 90, 40},
    {360, 540, 80, 50},
    {540, 660, 70, 100},
};
static const ProfileSegment kSlow1000Med[] = {
    {0, 360, 90, 40},
    {360, 600, 75, 50},
    {600, 750, 65, 100},
};
static const ProfileSegment kSlow1000Dark[] = {
    {0, 300, 90, 40},
    {300, 540, 75, 50},
    {540, 720, 65, 70},
    {720, 840, 55, 100},
};
static const ProfileSegment kSlow500Light[] = {
    {0, 300, 80, 40},
    {300, 480, 70, 50},
    {480, 600, 60, 100},
};
static const ProfileSegment kSlow500Med[] = {
    {0, 300, 80, 40},
    {300, 540, 70, 50},
    {540, 690, 60, 100},
};
static const ProfileSegment kSlow500Dark[] = {
    {0, 300, 80, 40},
    {300, 540, 65, 50},
    {540, 660, 55, 70},
    {660, 780, 50, 100},
};

static const AutoProfile kProfiles[] = {
    {16, 205, 480, kFast500Light, 3},
    {17, 210, 540, kFast1000Light, 2},
    {18, 205, 570, kFast500Med, 3},
    {19, 0, 630, kFast1000Med, 3},
    {20, 0, 660, kFast500Dark, 4},
    {21, 0, 720, kFast1000Dark, 4},
    {22, 195, 600, kSlow500Light, 3},
    {23, 200, 660, kSlow1000Light, 3},
    {24, 0, 690, kSlow500Med, 3},
    {25, 0, 750, kSlow1000Med, 3},
    {26, 0, 840, kSlow1000Dark, 4},
    {27, 0, 780, kSlow500Dark, 4},
};

const AutoProfile *autoProfileGetByPage(uint8_t pageNum) {
    for (const AutoProfile &p : kProfiles) {
        if (p.pageNum == pageNum) {
            return &p;
        }
    }
    return nullptr;
}
