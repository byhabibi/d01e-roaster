# Heater Firmware Flow — v3.5 vs v3.6

Lihat diagram lengkap & wiring: [`../../../docs/D01E_Heater_Control_v3.6.md`](../../../docs/D01E_Heater_Control_v3.6.md)

## Konstanta (rencana `config.h` v3.6)

```c
#define PIN_SSR_BURST       13    // IDC J9 pin 5 → Q_SSR gate
#define PCF8574_ADDR        0x20
#define PCF8574_PIN_K1_HOLD 0     // P0 → IDC J9 pin 10 → Q_K1A

#define HEAT_CYCLE_MS       10000UL
#define HEAT_MIN_ON_MS      1000UL   // jangan ON < 1s per siklus

#define NX_CMD_HEAT_SET     0x04
```

## Pseudocode burst scheduler (v3.6)

```c
void heaterTick(uint32_t nowMs) {
    if (!safetyOk() || heatPct == 0) {
        k1HoldOff();
        ssrOff();
        return;
    }
    k1HoldOn();

    if (heatPct >= 100) {
        ssrOn();
        return;
    }

    uint32_t onMs = (uint32_t)heatPct * HEAT_CYCLE_MS / 100;
    if (onMs < HEAT_MIN_ON_MS && heatPct > 0)
        onMs = HEAT_MIN_ON_MS;

    uint32_t pos = nowMs % HEAT_CYCLE_MS;
    if (pos < onMs)
        ssrOn();
    else
        ssrOff();
}
```

## v3.5 interim (tanpa ubah PCB)

```c
void heaterTickV35() {
    if (!safetyOk() || heatPct < 100) {
        digitalWrite(PIN_SSR_BURST, LOW);  // matikan K1+SSR
        return;
    }
    digitalWrite(PIN_SSR_BURST, HIGH);       // hanya 100%
}
```

## Nextion → ESP32

Setelah b4/b5 update `heatPct`:

```
printh A5
printh 04
printh heatPct
printh FF
printh FF
printh FF
```

Parser di `nextion_roast.cpp`:

```c
case NX_CMD_HEAT_SET:
    heatPct = _rxBuf[2];
    if (heatPct > 100) heatPct = 100;
    break;
```

## Panggilan di `loop()`

```c
nextion.poll();          // terima heatPct
heaterTick(millis());    // v3.5 atau v3.6
// ... roast tick 1 Hz ...
```
