#include "settings_storage.h"
#include "config.h"
#include <Preferences.h>

static Preferences s_prefs;
static bool s_useFahrenheit = false;
static uint32_t s_batchCount = 0;

void settingsBegin() {
    s_prefs.begin(SETTINGS_NVS_NS, false);
    s_useFahrenheit = s_prefs.getBool(SETTINGS_NVS_KEY_UNIT_F, false);
    s_batchCount = s_prefs.getUInt(SETTINGS_NVS_KEY_BATCH, 0);
    Serial.printf("[SETTINGS] unit=%s batch=%lu\n",
                  s_useFahrenheit ? "F" : "C",
                  (unsigned long)s_batchCount);
}

bool settingsGetUseFahrenheit() {
    return s_useFahrenheit;
}

void settingsSetUseFahrenheit(bool fahrenheit) {
    s_useFahrenheit = fahrenheit;
    s_prefs.putBool(SETTINGS_NVS_KEY_UNIT_F, fahrenheit);
    Serial.printf("[SETTINGS] unit set %s\n", fahrenheit ? "Fahrenheit" : "Celsius");
}

uint32_t settingsGetBatchCount() {
    return s_batchCount;
}

void settingsIncrementBatch() {
    s_batchCount++;
    s_prefs.putUInt(SETTINGS_NVS_KEY_BATCH, s_batchCount);
    Serial.printf("[SETTINGS] batch count %lu\n", (unsigned long)s_batchCount);
}

void settingsFormatBatch(char *out, size_t len) {
    snprintf(out, len, "%04lu", (unsigned long)s_batchCount);
}
