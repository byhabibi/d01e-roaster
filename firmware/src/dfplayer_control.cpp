#include "dfplayer_control.h"
#include "config.h"
#include <SoftwareSerial.h>

static SoftwareSerial s_dfSerial;

static void dfSendCommand(uint8_t cmd, uint16_t param) {
    uint8_t buf[10];
    buf[0] = 0x7E;
    buf[1] = 0xFF;
    buf[2] = 0x06;
    buf[3] = cmd;
    buf[4] = 0x00;
    buf[5] = (uint8_t)((param >> 8) & 0xFF);
    buf[6] = (uint8_t)(param & 0xFF);
    uint16_t sum = 0;
    for (int i = 1; i <= 6; i++) {
        sum += buf[i];
    }
    buf[7] = (uint8_t)(0x100 - (sum & 0xFF));
    buf[8] = 0xEF;
    s_dfSerial.write(buf, 9);
}

void dfplayerBegin() {
    s_dfSerial.begin(DF_BAUD, SWSERIAL_8N1, -1, DF_TX_PIN, false, 256);
    delay(500);
    dfplayerSetVolume(DF_VOLUME);
    Serial.println(F("[DFP] ready GPIO4 (SoftSerial)"));
}

void dfplayerSetVolume(uint8_t vol) {
    if (vol > 30) {
        vol = 30;
    }
    dfSendCommand(0x06, vol);
}

void dfplayerPlayTrack(uint16_t track) {
    if (track == 0) {
        return;
    }
    dfSendCommand(0x03, track);
    Serial.printf("[DFP] play track %u\n", (unsigned)track);
}

void dfplayerStop() {
    dfSendCommand(0x16, 0);
}
