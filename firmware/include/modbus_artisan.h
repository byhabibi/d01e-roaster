#pragma once
#include <Arduino.h>

// Artisan / external SCADA — holding register map (FC3 read, FC6/16 write)
// See D01E_Modbus_Artisan.md

#define MB_REG_BT           26   // °C × 10, uint16
#define MB_REG_ET           27
#define MB_REG_AIR          28   // exhaust % 0–100 RW
#define MB_REG_SV           29   // PID setpoint °C × 10 (optional)
#define MB_REG_HEATER       32   // heater % 0–100 RW
#define MB_REG_DRUM         33   // drum request 0/1 RW (still needs panel DRUM ON)
#define MB_REG_BT_ROR       44   // °C/min × 10, int16 as uint16
#define MB_REG_TIME_SEC     45   // roast elapsed seconds
#define MB_REG_COOLING      46   // cooling fan 0/1 RW
#define MB_REG_CMD          47   // 1=START 2=STOP 3=RESET (pulse)
#define MB_REG_STATUS       48   // read-only bitfield
#define MB_REG_MODE         49   // 0=local HMI, 1=Artisan remote control

#define MB_CMD_START        1
#define MB_CMD_STOP         2
#define MB_CMD_RESET        3

#define MB_STATUS_ROASTING  (1U << 0)
#define MB_STATUS_ESTOP_OK  (1U << 1)
#define MB_STATUS_DRUM_BTN  (1U << 2)
#define MB_STATUS_HEAT_BTN  (1U << 3)
#define MB_STATUS_AIR_BTN   (1U << 4)
#define MB_STATUS_ROAST_PG  (1U << 5)
#define MB_STATUS_REMOTE    (1U << 6)

typedef struct {
    bool remoteActive;
    bool cmdStart;
    bool cmdStop;
    bool cmdReset;
    bool heaterUpdated;
    bool airUpdated;
    bool coolingUpdated;
    uint8_t heaterPct;
    uint8_t airPct;
    bool coolingOn;
} ModbusCmdState;

void modbusArtisanBegin();
void modbusArtisanPoll();

void modbusArtisanUpdateTelemetry(float btC, float etC, float btRorCMin,
                                float etRorCMin, uint32_t roastSec, uint16_t statusBits);

ModbusCmdState modbusArtisanTakeCommands();

bool modbusArtisanRemoteActive();
