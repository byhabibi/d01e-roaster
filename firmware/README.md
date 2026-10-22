# D01E Roaster Firmware — Page 2

Firmware ESP32 v1.01 — roast pages 2/16–27, warm-up, settings, Modbus Artisan.

> **HMI lengkap (komponen, event, patch biner, simulator):** baca [`../../docs/D01E_HMI_DEVNOTE.md`](../../docs/D01E_HMI_DEVNOTE.md)

## Changelog v1.01

- Dedup `setText`/`setVal` Nextion — kurangi flicker HMI saat logging RS485
- Fix auto-cool `bt8` yang spam UART setiap loop
- `Danish Series.aset` extra baud diselaraskan 9600

## Perilaku

| Aksi | Hasil |
|------|--------|
| **START** (b1) | Timer `t0` mulai hitung **MM:SS** (00:00, 00:01, …) |
| **STOP** (b2) | Timer berhenti; suhu & RoR tetap di-update |
| **RESET** (b3) | Timer ke 00:00; display suhu/RoR ke 0.0 |
| Setiap 1 detik | BT → `t1`, RoR BT → `t2`, ET → `t3`, RoR ET → `t4` |
| **HEATER b4/b5** | `A5 04 [pct] FF FF FF` → GPIO13 (v3.5: hanya **0%** / **100%**) |
| **Kurva BT/ET/RoR** | `add 48,0/1/2,Y` setiap 1 s saat START |
| **AIR LEVEL** | Pot GPIO34 → `t5` % + exhaust PWM GPIO2 |
| **COOLING FAN bt0** | Toggle → PCF8574 P3 → 2× kipas cooling |

Skala kurva: BT/ET 0–250°C; Bean RoR 0–30. Pot air: 0% = fan off, 100% = full PWM.

**HMI dengan ESP32:** matikan `tm0` + `add` di Nextion — lihat `nextion_paste/13_HMI_MODE_ESP32.md`.

RoR dihitung dari jendela **15 detik** (°C/menit).

### Heater (Fase 1 — PCB v3.5)

| heatPct HMI | GPIO13 |
|-------------|--------|
| 0% | OFF |
| 10–90% | OFF (tampilan saja; log serial) |
| 100% | ON jika TC Bean OK |

Setelah revisi PCB v3.6: set `-DHEATER_HW_V36=1` di `platformio.ini` untuk burst fire.

Lihat [`../../docs/D01E_Heater_Control_v3.6.md`](../../docs/D01E_Heater_Control_v3.6.md). Detail modul: [`../../docs/D01E_Module_Optimization_Fase1.md`](../../docs/D01E_Module_Optimization_Fase1.md).

## Hardware (GPIO map v3.3 · Electrical v3.7 Fase 1)

MCU = **ESP32-DevKitC-32E** plug-in (`U1_DEVKIT`). Flash via **USB DevKit** (U11 CP2102 / J_USB = **DNP** Fase 1).  
Buck = **MP1584 FIXED 5V** (`U3_MOD`). Detail: `D01E_Module_Optimization_Fase1.md`.

| Fungsi | Pin |
|--------|-----|
| Nextion TX/RX | GPIO33 / GPIO32 @ 115200 |
| MAX31856 Bean | CS GPIO5 |
| MAX31856 Env | CS GPIO15 |
| Heater enable (v3.5 PCB) | GPIO13 → K1 + SSR (Active HIGH = ON) |
| Pot Air ADC | GPIO34 |
| Exhaust fan PWM | GPIO2 LEDC → IDC pin 8 |
| Cooling fan ON/OFF | PCF8574 P3 @ I2C 0x20 → IDC pin 7 (**Active LOW = ON**) |
| Panel DRUM/HEATER/AIR | GPIO25/26/27 in (Active LOW) → GPIO12 drum out; gating heater & air |
| E-Stop sense | GPIO14 (HIGH = UNSAFE) |
| RS485 Modbus | UART1 GPIO16/17 @ 9600, slave ID 19 — `D01E_Modbus_Artisan.md` |

## Panel interlock (latching buttons)

| Tombol | GPIO in | Output / gate |
|--------|---------|----------------|
| DRUM | 25 | GPIO12 drum enable |
| HEATER | 26 | Wajib ON sebelum GPIO13 heater |
| AIR | 27 | Wajib ON sebelum GPIO2 exhaust PWM |
| E-Stop | 14 | Semua output mati jika tripped |

Modul: `panel_interlock.cpp` — dipanggil setiap `loop()`.

## Build & flash

```powershell
cd "firmware\D01E_Roaster"
# Colok USB pada modul DevKitC (bukan J_USB PCB bila U11 DNP)
pio run -t upload
pio device monitor
```

## Nextion HMI

Lihat **`D01E_HMI_DEVNOTE.md`** (root) dan `scripts/patch_hmi_page2.py`. Event Touch Release: `NEXTION_PAGE2_EVENTS.txt`.

## Simulasi tanpa TC

Jika MAX31856 tidak terpasang, display menampilkan `---` untuk suhu.
