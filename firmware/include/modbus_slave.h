#pragma once
#include <Arduino.h>

typedef uint16_t (*ModbusRegReadFn)(uint16_t reg);
typedef void (*ModbusRegWriteFn)(uint16_t reg, uint16_t value);

void modbusSlaveBegin(HardwareSerial &serial, uint8_t slaveId);
void modbusSlavePoll();
void modbusSlaveSetCallbacks(ModbusRegReadFn readFn, ModbusRegWriteFn writeFn);
