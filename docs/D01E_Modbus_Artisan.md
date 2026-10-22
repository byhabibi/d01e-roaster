# D01E — Modbus RTU / Artisan

**Protokol:** Modbus RTU slave  
**Port:** RS485 → terminal **J3** (A/B/GND) → U6 MAX3485 → ESP32 UART1  
**Baud:** 9600 8N1  
**Slave ID:** 19  
**Artisan preset:** `Danish Series.aset`

---

## Hardware (PCB-01 MAIN — bukan IO-PWR)

RS485 ada di **PCB-01 MAIN-CTRL**, bukan PCB-02. PCB-02 hanya daya + beban drum/heater/fan.

| Ref | Part | Fungsi |
|-----|------|--------|
| U6 | MAX3485ESA+ | Transceiver RS485 |
| J3 | Phoenix 1757018 | Terminal A/B/GND (panel) |
| D1 | SMAJ6.5CA | TVS surge |
| R_BIAS1/2 | 560Ω | Bias A/B |
| R_TERM | 120Ω | Terminasi (pasang jika ujung bus) |
| Q6 | BC847C | Inverter DE ← PCF8574 P2 |
| U10 P2 | PCF8574 | DE/RE (HIGH = mode RX) |

Label harness panel: H1-12 KUNING = A, H1-13 HIJAU MUDA = B, H1-14 HITAM = GND → **J3**  
Pada header J_H1: RS485 = **pin 15/16** (pin 12/13 = Nextion — jangan tertukar)  
Kabel lapangan: Belden 9841 (shielded) — BOM `CBL_RS485`

---

## Register map (Holding, FC3 read / FC6 write)

| Reg | Nama | Akses | Satuan | Artisan |
|-----|------|-------|--------|---------|
| 26 | BT | R | °C × 10 | Input 1, divider 1/10 |
| 27 | ET | R | °C × 10 | Input 2, divider 1/10 |
| 28 | AIR | RW | 0–100 % | Input 3 + slider Fan |
| 29 | SV | RW | °C × 10 | PID SV (opsional) |
| 32 | HEATER | RW | 0–100 % | Input 5 + slider Burner |
| 33 | DRUM | R | 0/1 status | Input 6 (panel drum ON) |
| 44 | BT RoR | R | °C/min × 10 | Input 7, divider 1/10 |
| 45 | TIME | R | detik | Input 8 |
| 46 | COOLING | RW | 0/1 | Cooling fan virtual |
| 47 | CMD | W | 1=START 2=STOP 3=RESET | Pulse (auto-clear) |
| 48 | STATUS | R | bitfield | Lihat bawah |
| 49 | MODE | RW | 0=HMI 1=remote | Aktifkan kontrol Artisan |

### STATUS (reg 48)

| Bit | Makna |
|-----|-------|
| 0 | Roast running |
| 1 | E-Stop OK |
| 2 | Panel DRUM ON |
| 3 | Panel HEATER ON |
| 4 | Panel AIR ON |
| 5 | Halaman roast aktif |
| 6 | Remote mode aktif |

---

## Artisan setup

> **Jalur data:** Artisan / logger **RS485** ke terminal **J3** A/B/GND (label harness H1-12/13/14), **bukan** USB-C/J_USB dan **bukan** J_H1 pin 12/13 (Nextion). USB DevKit = flash/debug @ 115200.

1. **Config → Machine** → load `Danish Series.aset`
2. **Config → Ports → Modbus** — verifikasi:
   - COM port = USB-RS485 adapter
   - 9600 8N1
   - Input 1: reg **26**, slave **19**, FC3, div **1/10**, °C
   - Input 2: reg **27**, idem
   - Extra devices: reg **28**, **32**, **44** (sudah di aset)
3. Slider **Fan** → `writeSingle(19,28,{})`
4. Slider **Burner** → `writeSingle(19,32,{})`
5. Aktifkan **Control** di Device untuk PID (opsional)

Tulis reg **49 = 1** untuk mode remote, atau geser slider heater/air (otomatis set MODE).

---

## Safety

- E-Stop, tombol panel HEATER/AIR/DRUM tetap berlaku (interlock firmware).
- Heater % dari Artisan hanya keluar jika tombol **HEATER** panel ON.
- Exhaust % hanya jika tombol **AIR** panel ON.
- Drum motor hanya dari tombol **DRUM** panel (reg 33 = status baca saja).
- Mode **auto roast** page16–27: kontrol Artisan heater/air **nonaktif** (baca suhu saja). Gunakan **page2 manual** untuk logging + kontrol penuh.

---

## Firmware

| File | Peran |
|------|-------|
| `modbus_slave.cpp` | RTU slave FC3/4/6/16 |
| `modbus_artisan.cpp` | Register map + telemetri |
| `pcf8574_io.cpp` | P2 DE + P3 cooling |
| `dfplayer_control.cpp` | SoftSerial GPIO4 (UART1 = RS485) |

GPIO: RX=16, TX=17, DE=PCF8574 P2 via Q6.

---

## Uji cepat

1. USB-RS485 ke J3 panel, A↔A, B↔B, GND↔GND
2. Flash firmware, Serial Monitor: `[MB] RTU slave id=19`
3. Artisan connect → kurva BT/ET naik
4. Page2 manual, tombol panel ON, slider heater/air di Artisan

Modbus scanner (di Artisan Ports) bisa scan reg 26–27.
