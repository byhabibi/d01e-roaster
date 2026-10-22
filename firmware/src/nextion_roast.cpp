#include "nextion_roast.h"
#include "config.h"
#include <cstring>

namespace {

constexpr uint8_t TEXT_CACHE_MAX = 12;
constexpr uint8_t VAL_CACHE_MAX = 12;

struct TextCacheEntry {
    const char *name;
    char text[16];
};

struct ValCacheEntry {
    const char *name;
    uint32_t val;
};

TextCacheEntry s_textCache[TEXT_CACHE_MAX];
uint8_t s_textCacheLen = 0;
ValCacheEntry s_valCache[VAL_CACHE_MAX];
uint8_t s_valCacheLen = 0;

void resetTextValCache() {
    s_textCacheLen = 0;
    s_valCacheLen = 0;
}

const char *findCachedText(const char *component) {
    for (uint8_t i = 0; i < s_textCacheLen; i++) {
        if (strcmp(s_textCache[i].name, component) == 0) {
            return s_textCache[i].text;
        }
    }
    return nullptr;
}

void rememberText(const char *component, const char *text) {
    for (uint8_t i = 0; i < s_textCacheLen; i++) {
        if (strcmp(s_textCache[i].name, component) == 0) {
            strncpy(s_textCache[i].text, text, sizeof(s_textCache[i].text) - 1);
            s_textCache[i].text[sizeof(s_textCache[i].text) - 1] = '\0';
            return;
        }
    }
    if (s_textCacheLen >= TEXT_CACHE_MAX) {
        return;
    }
    s_textCache[s_textCacheLen].name = component;
    strncpy(s_textCache[s_textCacheLen].text, text, sizeof(s_textCache[s_textCacheLen].text) - 1);
    s_textCache[s_textCacheLen].text[sizeof(s_textCache[s_textCacheLen].text) - 1] = '\0';
    s_textCacheLen++;
}

bool findCachedVal(const char *component, uint32_t *out) {
    for (uint8_t i = 0; i < s_valCacheLen; i++) {
        if (strcmp(s_valCache[i].name, component) == 0) {
            *out = s_valCache[i].val;
            return true;
        }
    }
    return false;
}

void rememberVal(const char *component, uint32_t val) {
    for (uint8_t i = 0; i < s_valCacheLen; i++) {
        if (strcmp(s_valCache[i].name, component) == 0) {
            s_valCache[i].val = val;
            return;
        }
    }
    if (s_valCacheLen >= VAL_CACHE_MAX) {
        return;
    }
    s_valCache[s_valCacheLen].name = component;
    s_valCache[s_valCacheLen].val = val;
    s_valCacheLen++;
}

} // namespace

void NextionRoast::begin(HardwareSerial &serial, uint32_t baud) {
    resetTextValCache();
    _nx = &serial;
    _nx->begin(baud, SERIAL_8N1, NX_RX_PIN, NX_TX_PIN);
}

void NextionRoast::sendEnd() {
    _nx->write(0xFF);
    _nx->write(0xFF);
    _nx->write(0xFF);
}

void NextionRoast::setText(const char *component, const char *text) {
    const char *prev = findCachedText(component);
    if (prev && strcmp(prev, text) == 0) {
        return;
    }
    rememberText(component, text);
    _nx->print(component);
    _nx->print(".txt=\"");
    _nx->print(text);
    _nx->print("\"");
    sendEnd();
}

void NextionRoast::setVal(const char *component, uint32_t val) {
    uint32_t prev = 0;
    if (findCachedVal(component, &prev) && prev == val) {
        return;
    }
    rememberVal(component, val);
    _nx->print(component);
    _nx->print(".val=");
    _nx->print(val);
    sendEnd();
}

void NextionRoast::setPic(const char *component, uint32_t picId) {
    _nx->print(component);
    _nx->print(".pic=");
    _nx->print(picId);
    sendEnd();
}

void NextionRoast::setEn(const char *component, uint32_t en) {
    _nx->print(component);
    _nx->print(".en=");
    _nx->print(en);
    sendEnd();
}

void NextionRoast::setTimerMmSs(const char *component, uint32_t elapsedSec) {
    char buf[8];
    uint32_t mm = elapsedSec / 60;
    uint32_t ss = elapsedSec % 60;
    snprintf(buf, sizeof(buf), "%02u:%02u", (unsigned)mm, (unsigned)ss);
    setText(component, buf);
}

void NextionRoast::setPage(const char *pageName) {
    resetTextValCache();
    _nx->print("page ");
    _nx->print(pageName);
    sendEnd();
}

void NextionRoast::addWaveform(uint8_t channel, uint8_t valueY) {
    _nx->print("add ");
    _nx->print(NX_WAVE_OBJ);
    _nx->print(".id,");
    _nx->print(channel);
    _nx->print(',');
    _nx->print(valueY);
    sendEnd();
}

void NextionRoast::clearWaveform(uint8_t channel) {
    _nx->print("cle ");
    _nx->print(NX_WAVE_OBJ);
    _nx->print(".id,");
    _nx->print(channel);
    sendEnd();
}

void NextionRoast::clearFlags() {
    _flagStart = _flagStop = _flagReset = false;
}

uint8_t NextionRoast::takeHeatPct() {
    _flagHeatSet = false;
    return _heatPct;
}

bool NextionRoast::takeCoolOn() {
    _flagCoolSet = false;
    return _coolOn;
}

bool NextionRoast::takeColorOn() {
    _flagColorSet = false;
    return _colorOn;
}

void NextionRoast::clearPageFlags() {
    _flagPageLeave = false;
    _flagPageEnter = false;
    _flagWarmupLeave = false;
    _flagWarmupEnter = false;
    _flagLimitPageEnter = false;
    _flagSettingsPageEnter = false;
    _flagAutoPageEnter = false;
}

uint8_t NextionRoast::takeAutoPageNum() {
    _flagAutoPageEnter = false;
    return _autoPageNum;
}

uint16_t NextionRoast::takeTempLimitC() {
    _flagTempLimitSave = false;
    return _tempLimitC;
}

bool NextionRoast::takeUnitFahrenheit() {
    _flagUnitSet = false;
    return _unitFahrenheit;
}

// Map Nextion Editor component id → action (page 2 roast screen)
// If you re-order components in Editor, update this table or use printh A5 xx.
void NextionRoast::handleStandardTouch(uint8_t page, uint8_t compId, uint8_t event) {
    if (page != NX_PAGE_ROAST || event != 0x00) {
        return; // release only
    }
    // Default IDs from typical Nextion export order on page2:
    // b0=START, b1=STOP, b2=RESET — IDs vary per HMI file; printh method preferred.
    switch (compId) {
        case 4:  _flagStart = true; break;  // fallback — adjust if needed
        case 5:  _flagStop = true; break;
        case 6:  _flagReset = true; break;
        default: break;
    }
}

void NextionRoast::handleByte(uint8_t b) {
    _rxBuf[_rxLen++] = b;

    // Custom: A5 cmd [data] FF FF FF
    if (_rxLen >= 2 && _rxBuf[0] == NX_CMD_MAGIC) {
        if (_rxBuf[1] == NX_CMD_HEAT_SET || _rxBuf[1] == NX_CMD_COOL_SET ||
            _rxBuf[1] == NX_CMD_COLOR_SET || _rxBuf[1] == NX_CMD_UNIT_SET ||
            _rxBuf[1] == NX_CMD_AUTO_ENTER) {
            if (_rxLen >= 6 && _rxBuf[3] == 0xFF && _rxBuf[4] == 0xFF && _rxBuf[5] == 0xFF) {
                if (_rxBuf[1] == NX_CMD_HEAT_SET) {
                    _heatPct = _rxBuf[2];
                    if (_heatPct > 100) {
                        _heatPct = 100;
                    }
                    _flagHeatSet = true;
                } else if (_rxBuf[1] == NX_CMD_COOL_SET) {
                    _coolOn = (_rxBuf[2] != 0);
                    _flagCoolSet = true;
                } else if (_rxBuf[1] == NX_CMD_COLOR_SET) {
                    _colorOn = (_rxBuf[2] != 0);
                    _flagColorSet = true;
                } else if (_rxBuf[1] == NX_CMD_AUTO_ENTER) {
                    _autoPageNum = _rxBuf[2];
                    _flagAutoPageEnter = true;
                } else {
                    _unitFahrenheit = (_rxBuf[2] != 0);
                    _flagUnitSet = true;
                }
                _rxLen = 0;
                return;
            }
            if (_rxLen >= 16) {
                _rxLen = 0;
            }
            return;
        }

        if (_rxBuf[1] == NX_CMD_TEMP_LIMIT_SAVE) {
            if (_rxLen >= 7 && _rxBuf[4] == 0xFF && _rxBuf[5] == 0xFF && _rxBuf[6] == 0xFF) {
                _tempLimitC = ((uint16_t)_rxBuf[2] << 8) | _rxBuf[3];
                _flagTempLimitSave = true;
                _rxLen = 0;
                return;
            }
            if (_rxLen >= 16) {
                _rxLen = 0;
            }
            return;
        }

        if (_rxLen >= 5 && _rxBuf[2] == 0xFF && _rxBuf[3] == 0xFF && _rxBuf[4] == 0xFF) {
            switch (_rxBuf[1]) {
                case NX_CMD_START:            _flagStart = true; break;
                case NX_CMD_STOP:             _flagStop = true; break;
                case NX_CMD_RESET:            _flagReset = true; break;
                case NX_CMD_PAGE_LEAVE:       _flagPageLeave = true; break;
                case NX_CMD_PAGE_ENTER:       _flagPageEnter = true; break;
                case NX_CMD_WARMUP_LEAVE:     _flagWarmupLeave = true; break;
                case NX_CMD_WARMUP_ENTER:     _flagWarmupEnter = true; break;
                case NX_CMD_LIMIT_PAGE_ENTER: _flagLimitPageEnter = true; break;
                case NX_CMD_SETTINGS_ENTER:   _flagSettingsPageEnter = true; break;
            }
            _rxLen = 0;
            return;
        }
        if (_rxLen >= 16) {
            _rxLen = 0;
        }
        return;
    }

    // Standard touch: 65 page id event FF FF FF
    if (_rxBuf[0] == 0x65 && _rxLen >= 7) {
        if (_rxBuf[4] == 0xFF && _rxBuf[5] == 0xFF && _rxBuf[6] == 0xFF) {
            handleStandardTouch(_rxBuf[1], _rxBuf[2], _rxBuf[3]);
            _rxLen = 0;
            return;
        }
    }

    if (_rxLen >= sizeof(_rxBuf)) {
        _rxLen = 0;
    }
}

void NextionRoast::poll() {
    while (_nx->available()) {
        handleByte((uint8_t)_nx->read());
    }
}
