#pragma once
#include <Arduino.h>

class NextionRoast {
public:
    void begin(HardwareSerial &serial, uint32_t baud);

    void setText(const char *component, const char *text);
    void setVal(const char *component, uint32_t val);
    void setPic(const char *component, uint32_t picId);
    void setEn(const char *component, uint32_t en);
    void setTimerMmSs(const char *component, uint32_t elapsedSec);
    void setPage(const char *pageName);

    void addWaveform(uint8_t channel, uint8_t valueY);
    void clearWaveform(uint8_t channel);

    // Non-blocking parser — call every loop()
    void poll();

    bool startPressed()  { return _flagStart; }
    bool stopPressed()   { return _flagStop; }
    bool resetPressed()  { return _flagReset; }
    void clearFlags();

    bool heatPctUpdated() { return _flagHeatSet; }
    uint8_t takeHeatPct();

    bool coolStateUpdated() { return _flagCoolSet; }
    bool takeCoolOn();

    bool colorStateUpdated() { return _flagColorSet; }
    bool takeColorOn();

    bool pageLeavePressed() { return _flagPageLeave; }
    bool pageEnterPressed() { return _flagPageEnter; }
    bool warmupLeavePressed() { return _flagWarmupLeave; }
    bool warmupEnterPressed() { return _flagWarmupEnter; }
    bool limitPageEnterPressed() { return _flagLimitPageEnter; }
    bool settingsPageEnterPressed() { return _flagSettingsPageEnter; }
    bool autoPageEnterPressed() { return _flagAutoPageEnter; }
    uint8_t takeAutoPageNum();
    bool tempLimitSavePressed() { return _flagTempLimitSave; }
    uint16_t takeTempLimitC();
    bool unitSetUpdated() { return _flagUnitSet; }
    bool takeUnitFahrenheit();
    void clearPageFlags();

private:
    HardwareSerial *_nx = nullptr;
    uint8_t _rxBuf[16];
    uint8_t _rxLen = 0;

    bool _flagStart = false;
    bool _flagStop = false;
    bool _flagReset = false;
    bool _flagHeatSet = false;
    uint8_t _heatPct = 0;
    bool _flagCoolSet = false;
    bool _coolOn = false;
    bool _flagColorSet = false;
    bool _colorOn = false;
    bool _flagPageLeave = false;
    bool _flagPageEnter = false;
    bool _flagWarmupLeave = false;
    bool _flagWarmupEnter = false;
    bool _flagLimitPageEnter = false;
    bool _flagSettingsPageEnter = false;
    bool _flagAutoPageEnter = false;
    uint8_t _autoPageNum = 0;
    bool _flagTempLimitSave = false;
    bool _flagUnitSet = false;
    bool _unitFahrenheit = false;
    uint16_t _tempLimitC = TEMP_LIMIT_DEFAULT_C;

    void sendEnd();
    void handleByte(uint8_t b);
    void handleStandardTouch(uint8_t page, uint8_t compId, uint8_t event);
};
