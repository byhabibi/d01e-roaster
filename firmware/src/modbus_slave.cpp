#include "modbus_slave.h"
#include "pcf8574_io.h"
#include "config.h"

static HardwareSerial *s_serial = nullptr;
static uint8_t s_slaveId = 1;
static ModbusRegReadFn s_readFn = nullptr;
static ModbusRegWriteFn s_writeFn = nullptr;

static uint8_t s_rxBuf[256];
static size_t s_rxLen = 0;
static uint32_t s_lastRxMs = 0;

static uint16_t crc16(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

static void txBytes(const uint8_t *data, size_t len) {
    if (!s_serial) {
        return;
    }
    pcf8574SetRs485TxMode();
    delayMicroseconds(50);
    s_serial->write(data, len);
    s_serial->flush();
    delayMicroseconds(500);
    pcf8574SetRs485RxMode();
}

static void sendException(uint8_t slave, uint8_t func, uint8_t exCode) {
    uint8_t out[5];
    out[0] = slave;
    out[1] = (uint8_t)(func | 0x80);
    out[2] = exCode;
    uint16_t crc = crc16(out, 3);
    out[3] = (uint8_t)(crc & 0xFF);
    out[4] = (uint8_t)(crc >> 8);
    txBytes(out, 5);
}

static void handleReadHolding(uint8_t slave, uint8_t func) {
    if (s_rxLen < 8 || !s_readFn) {
        sendException(slave, func, 0x03);
        return;
    }
    uint16_t start = (uint16_t)((s_rxBuf[2] << 8) | s_rxBuf[3]);
    uint16_t qty = (uint16_t)((s_rxBuf[4] << 8) | s_rxBuf[5]);
    if (qty == 0 || qty > 64) {
        sendException(slave, func, 0x03);
        return;
    }

    uint8_t out[256];
    out[0] = slave;
    out[1] = func;
    out[2] = (uint8_t)(qty * 2);
    for (uint16_t i = 0; i < qty; i++) {
        uint16_t val = s_readFn((uint16_t)(start + i));
        out[3 + i * 2] = (uint8_t)(val >> 8);
        out[4 + i * 2] = (uint8_t)(val & 0xFF);
    }
    size_t bodyLen = 3 + qty * 2;
    uint16_t crc = crc16(out, bodyLen);
    out[bodyLen] = (uint8_t)(crc & 0xFF);
    out[bodyLen + 1] = (uint8_t)(crc >> 8);
    txBytes(out, bodyLen + 2);
}

static void handleWriteSingle(uint8_t slave, uint8_t func) {
    if (s_rxLen < 8) {
        sendException(slave, func, 0x03);
        return;
    }
    uint16_t reg = (uint16_t)((s_rxBuf[2] << 8) | s_rxBuf[3]);
    uint16_t val = (uint16_t)((s_rxBuf[4] << 8) | s_rxBuf[5]);
    if (s_writeFn) {
        s_writeFn(reg, val);
    }
    uint8_t out[8];
    memcpy(out, s_rxBuf, 6);
    uint16_t crc = crc16(out, 6);
    out[6] = (uint8_t)(crc & 0xFF);
    out[7] = (uint8_t)(crc >> 8);
    txBytes(out, 8);
}

static void handleWriteMultiple(uint8_t slave, uint8_t func) {
    if (s_rxLen < 9 || !s_writeFn) {
        sendException(slave, func, 0x03);
        return;
    }
    uint16_t start = (uint16_t)((s_rxBuf[2] << 8) | s_rxBuf[3]);
    uint16_t qty = (uint16_t)((s_rxBuf[4] << 8) | s_rxBuf[5]);
    uint8_t byteCount = s_rxBuf[6];
    if (qty == 0 || qty > 64 || byteCount != qty * 2 || s_rxLen < (size_t)(9 + byteCount)) {
        sendException(slave, func, 0x03);
        return;
    }
    for (uint16_t i = 0; i < qty; i++) {
        uint16_t val = (uint16_t)((s_rxBuf[7 + i * 2] << 8) | s_rxBuf[8 + i * 2]);
        s_writeFn((uint16_t)(start + i), val);
    }
    uint8_t out[8];
    out[0] = slave;
    out[1] = func;
    out[2] = s_rxBuf[2];
    out[3] = s_rxBuf[3];
    out[4] = s_rxBuf[4];
    out[5] = s_rxBuf[5];
    uint16_t crc = crc16(out, 6);
    out[6] = (uint8_t)(crc & 0xFF);
    out[7] = (uint8_t)(crc >> 8);
    txBytes(out, 8);
}

static void processFrame() {
    if (s_rxLen < 4) {
        return;
    }
    uint16_t recvCrc = (uint16_t)(s_rxBuf[s_rxLen - 2] | (s_rxBuf[s_rxLen - 1] << 8));
    uint16_t calcCrc = crc16(s_rxBuf, s_rxLen - 2);
    if (recvCrc != calcCrc) {
        return;
    }

    uint8_t slave = s_rxBuf[0];
    if (slave != s_slaveId && slave != 0) {
        return;
    }
    uint8_t func = s_rxBuf[1];
    switch (func) {
    case 0x03:
    case 0x04:
        handleReadHolding(slave, func);
        break;
    case 0x06:
        handleWriteSingle(slave, func);
        break;
    case 0x10:
        handleWriteMultiple(slave, func);
        break;
    default:
        sendException(slave, func, 0x01);
        break;
    }
}

void modbusSlaveBegin(HardwareSerial &serial, uint8_t slaveId) {
    s_serial = &serial;
    s_slaveId = slaveId;
    s_rxLen = 0;
    s_lastRxMs = 0;
    pcf8574SetRs485RxMode();
}

void modbusSlaveSetCallbacks(ModbusRegReadFn readFn, ModbusRegWriteFn writeFn) {
    s_readFn = readFn;
    s_writeFn = writeFn;
}

void modbusSlavePoll() {
    if (!s_serial) {
        return;
    }

    while (s_serial->available()) {
        if (s_rxLen < sizeof(s_rxBuf)) {
            s_rxBuf[s_rxLen++] = (uint8_t)s_serial->read();
        } else {
            s_serial->read();
        }
        s_lastRxMs = millis();
    }

    if (s_rxLen > 0 && (millis() - s_lastRxMs) >= MODBUS_FRAME_GAP_MS) {
        processFrame();
        s_rxLen = 0;
    }
}
