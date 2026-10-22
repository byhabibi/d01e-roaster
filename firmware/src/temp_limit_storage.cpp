#include "temp_limit_storage.h"
#include "config.h"
#include <Preferences.h>

static Preferences s_prefs;
static uint16_t s_limitC = TEMP_LIMIT_DEFAULT_C;

static uint16_t clampLimit(uint16_t c) {
    if (c < TEMP_LIMIT_MIN_C) {
        return TEMP_LIMIT_MIN_C;
    }
    if (c > TEMP_LIMIT_MAX_C) {
        return TEMP_LIMIT_MAX_C;
    }
    return c;
}

void tempLimitBegin() {
    s_prefs.begin(TEMP_LIMIT_NVS_NS, false);
    uint16_t stored = (uint16_t)s_prefs.getUShort(TEMP_LIMIT_NVS_KEY, TEMP_LIMIT_DEFAULT_C);
    s_limitC = clampLimit(stored);
    if (stored != s_limitC) {
        s_prefs.putUShort(TEMP_LIMIT_NVS_KEY, s_limitC);
    }
    Serial.printf("[LIMIT] NVS %u C\n", (unsigned)s_limitC);
}

uint16_t tempLimitGet() {
    return s_limitC;
}

bool tempLimitSet(uint16_t celsius) {
    uint16_t v = clampLimit(celsius);
    s_limitC = v;
    s_prefs.putUShort(TEMP_LIMIT_NVS_KEY, v);
    Serial.printf("[LIMIT] saved %u C\n", (unsigned)v);
    return true;
}

bool tempLimitIsSafe(float btC) {
    if (isnan(btC)) {
        return true;
    }
    return btC < (float)s_limitC;
}
