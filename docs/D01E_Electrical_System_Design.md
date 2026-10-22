# Panduan Desain Elektronik — Hartanzah Danish 01 Electric (D01E)

**Versi:** 3.7 — Fase 1 Module-on-PCB (2 board production)  
**Status:** ✅ **APPROVED — MAIN memakai kursi modul (MP1584 / DevKitC / MAX31856×2); IO-PWR industrial locked**  
**Audit modul:** `D01E_Module_Optimization_Fase1.md`  
baca berurutan, jangan skip Bagian 2

---

## DAFTAR ISI

1. [Ringkasan mesin](#1-ringkasan-mesin)
2. [Aturan safety](#2-aturan-safety)
3. [Diagram aliran daya](#3-diagram-aliran-daya)
4. [Dua PCB & designator](#4-dua-pcb--designator)
5. [Tabel I/O](#5-tabel-io)
6. [Pin ESP32 & PCF8574](#6-pin-esp32--pcf8574)
7. [Thermocouple](#7-thermocouple)
8. [Harness kabel](#8-harness-kabel)
9. [Logika tombol & HMI](#9-logika-tombol--hmi)
10. [Hitungan daya](#10-hitungan-daya)
11. [Checklist schematic](#11-checklist-schematic)
12. [Keputusan terkunci](#12-keputusan-terkunci)

---

## 1. Ringkasan mesin

| Parameter | Nilai |
|-----------|-------|
| Listrik | 220V AC 1-fasa 50Hz (L + N + PE) |
| Heater | 3600W → **16,4A AC** |
| Motor drum | 12V DC, 25–60W |
| Kipas cooling | 12V DC, 2,16W × 2 (parallel) |
| Kipas exhaust | 12V DC, 2,16W × 1 (PWM speed) |
| HMI | Nextion **NX8048K070-011C** |
| Box kontrol | Maks 40°C → **NTC GPIO35** |
| Sertifikasi | CE |

| PCB | File KiCad | Isi |
|-----|------------|-----|
| PCB-01 | `D01E-MAIN-CTRL` | Module seats: DevKitC, MP1584 mod, MAX31856×2 + RS485/opto/DFPlayer |
| PCB-02 | `D01E-IO-PWR` | PSU, contactor, SSR, MOSFET beban (tidak diubah Fase 1) |

---

## 2. Aturan safety

### 2.1 Prinsip utama

> E-Stop memutus daya berbahaya di **hardware**, bukan hanya software.

### 2.2 Heater wajib: Contactor + SSR + Fuse

| Komponen | Fungsi |
|----------|--------|
| **K1** LC1D25JD | Memutus **L 220V** saat E-Stop |
| **SSR1** D2440 | Switch operasi normal |
| **F1** 25A | Fuse proteksi |

### 2.3 E-Stop XB4BS8444 (2NC) — BUKAN 8445

| NC | Fungsi |
|----|--------|
| **NC #1** | Memutus **L AC** masuk PSU 12V |
| **NC #2** | Memutus **+12V** ke jalur coil K1 saja (bukan bus 12V drum/fan) |

### 2.4 Main switch: ABB OT25FT2 (2P 25A)

Bukan switch Harmony 22mm (max 10A). Tanpa aux contact = normal.

### 2.5 Coil contactor pakai Q_K1, BUKAN PC817

Coil K1 ≈ **0,4A**. PC817 max **50mA** → **tidak bisa**.

**Solusi:** GPIO13 → IDC pin 5 → **Q_K1 IRLZ44N** + **D_K1 1N4007**

### 2.6 Urutan proteksi

```
L ──► [SW5 2P 25A] ──► [F4 MCB 25A] ──┬──► [F1 25A] ──► [K1] ──► [SSR1] ──► Heater ──► N
                                      └──► [EMI 6A] ──► [F2 6A] ──► [E-Stop NC#1] ──► PSU
```

---

## 3. Diagram aliran daya

### 3.1 Cabang HEATER (tanpa EMI)

```
L ──► SW5 ──► F4 ──► F1 ──► K1 (pole L1→T1) ──► SSR1 ──► Heater L
N ────────────────────────────────────────────────────────► Heater N

Driver coil + SSR (PCB IO):
  +12V (dari F3, cabang terpisah) ──► [E-Stop NC#2] ──► titik X
  titik X ──► K1 coil A1
  K1 coil A2 ──► Q_K1 drain ──► GND
  titik X ──► SSR1 control (+)
  SSR1 control (−) ──► Q_K1 drain
  Q_K1 gate ◄── 1kΩ ◄── IDC pin 5 ◄── GPIO13
  D_K1 (1N4007) paralel coil: katoda ke titik X, anoda ke A2
```

### 3.2 Cabang KONTROL

```
L ──► (dari F4, cabang paralel heater) ──► EMI 6A ──► F2 6A ──► E-Stop NC#1 ──► PSU AC in
N ──► SW5 ──► PSU N
PE ──► PSU FG + J_PE chassis (1 titik)
```

### 3.3 Distribusi 12V

```
PSU +12V ──► F3 10A ──┬──► F_DRUM 7.5A ──► Q2 ──► Drum
                       ├──► Q3 ◄── Q7 ◄── IDC pin 7 (P3 cooling)
                       ├──► Q4 ◄── IDC pin 8 (exhaust PWM)
                       ├──► IDC pin 1 (+12V ke MAIN buck)
                       └──► cabang NC#2 ──► Q_K1 path (3.1)

Catatan: NC#2 HANYA di jalur heater, BUKAN di bus drum/fan.
```

---

## 4. Dua PCB & designator

### PCB-01 MAIN — `D01E-MAIN-CTRL` (Fase 1: module seats)

Arsitektur: PCB fabrikasi massal dengan **kursi header 2.54 mm** — modul pasar pluggable untuk junior fab/maintenance. Detail pin map & QC: `D01E_Module_Optimization_Fase1.md`.

| RefDes / Silk | Komponen | Fungsi |
|---------------|----------|--------|
| U1 / `U1_DEVKIT` | ESP32-DevKitC-32E | MCU pada dual header 1×19 |
| U2 | MCP1700-3.3 | LDO +3V3_ANA → VIN seat U4/U5 |
| U3 / `U3_MOD` `BUCK 5V FIXED` | Modul buck **FIXED 5.0 V** ≥3A (MP1584-class) | **LARANG** adjustable/trimpot; QC ukur 4.90–5.10 V |
| U4 / `U4_MOD` `BEAN TC MODULE` | MAX31856 breakout | Thermocouple Bean (pluggable) |
| U5 / `U5_MOD` `ENV TC MODULE` | MAX31856 breakout | Thermocouple Env (pluggable) |
| U6 | MAX3485 | RS485 |
| U7–U9, U12 | PC817 ×4 | Opto tombol + E-Stop sense |
| U10 | PCF8574AP | Cooling + RS485 DE + DFPlayer BUSY |
| U11 | CP2102N | **DNP Fase 1** — flash via USB DevKit |
| U13 | DFPlayer Mini | Audio |
| Q6 | BC847C | Inverter RS485 DE (P2) |
| J1, J2 | PCC-SMP-K | TC panel → terminal modul U4/U5 |
| J3 | Phoenix 3p | RS485 |
| J_H1 | Header 20p | Harness panel H1 |
| J_USB | USB-C | Optional/DNP spare (bukan path flash utama) |
| J9 | IDC 2×10 | Ke PCB IO — pin1 +12V ke **U3_MOD VIN** |

### PCB-02 IO — `D01E-IO-PWR`

| RefDes | Komponen | Fungsi |
|--------|----------|--------|
| PSU1 | LRS-150-12 | PSU 12V |
| K1 | LC1D25JD | Contactor 25A |
| SSR1 + HS1 | D2440 + heatsink | SSR heater |
| Q_K1 + D_K1 | IRLZ44N + 1N4007 | Driver coil + SSR ctrl |
| R_G1, R_G2 | 1k + 10k | Gate Q_K1 |
| Q2 + U20 + D2 | IRLB8743 + TC4427 + SS34 | Drum |
| Q3 + Q7 + D3 | SI2302 + BC847C + 1N5819 | Cooling ×2 (P3 via Q7) |
| Q4 + D4 | SI2302 + 1N5819 | Exhaust (PWM gate) |
| F1–F3, F_DRUM | Fuse | Proteksi |
| J4–J8 | Terminal | Field |
| J9 | IDC 2×10 | Ke MAIN |
| J_PE | Stud M4 | Earth |

### J9 IDC Pinout FINAL

| Pin | Sinyal | Arah | Warna |
|-----|--------|------|-------|
| 1 | +12V | IO → MAIN **U3_MOD VIN** (MP1584 module) | MERAH |
| 2 | GND_PWR | Bersama | HITAM |
| 3 | NC | — | — |
| 4 | GND_SIG | Bersama | PUTIH/HITAM |
| 5 | HEATER_EN | MAIN GPIO13 → IO Q_K1 gate | KUNING |
| 6 | DRUM_EN | MAIN GPIO12 → IO U20 in | HIJAU |
| 7 | COOL_EN | MAIN PCF8574 P3 → IO Q7 → Q3 gate | BIRU |
| 8 | EXHAUST_PWM | MAIN GPIO2 LEDC → IO Q4 gate | UNGU |
| 9 | +5V sense | MAIN → IO (opsional) | ORANYE |
| 10–20 | SPARE | — | — |

> Tombol panel → **J_H1** langsung ke MAIN (bukan lewat IDC).  
> TC, RS485, Nextion, USB → MAIN langsung.

---

## 5. Tabel I/O

### INPUT

| # | Nama | Hardware | Tujuan |
|---|------|----------|--------|
| 1 | TC Bean | Probe K 2m | U4 |
| 2 | TC Environment | Probe K 2m exhaust | U5 |
| 3 | NTC Box | NTC 10k | GPIO35 |
| 4 | Tombol Drum | NO 22mm | GPIO25 via U7 |
| 5 | Tombol Heater | NO 22mm | GPIO26 via U8 |
| 6 | Tombol Air | NO 22mm | GPIO27 via U9 |
| 7 | Pot Air | 10kΩ | GPIO34 ADC |
| 8 | E-Stop | XB4BS8444 | NC→daya; GPIO14 via U12 |
| 9 | Comm Port | RS485 | U6 Modbus 9600 |
| 10 | Virtual Cooling | Nextion | PCF8574 P3 |
| 11 | Color sensor | DNP v1 | Header J_COLOR |

### OUTPUT

| # | Nama | Driver | Beban |
|---|------|--------|-------|
| 1 | Drum | Q2 + U20 | Motor 12V |
| 2 | Heater | K1 + SSR1 + Q_K1 | 3600W AC |
| 3 | Cooling | Q3 via P3 | 2× kipas 12V |
| 4 | Exhaust | Q4 via GPIO2 PWM | 1× kipas 12V |
| 5 | Speaker | U13 DFPlayer | 3W 8Ω |

---

## 6. Pin ESP32 & PCF8574

### 6.1 GPIO ESP32

| GPIO | Fungsi | Catatan |
|------|--------|---------|
| 1, 3 | UART0 → CP2102N | USB flash only |
| 2 | **LEDC PWM exhaust** → IDC pin 8 | Strapping — duty=0 saat boot |
| 4 | DFPlayer TX (SoftSerial) | One-wire command |
| 5, 15 | SPI CS TC Bean / Env | Pull-up 10k |
| 12 | Drum enable → IDC pin 6 | Pull-down 10k |
| 13 | Heater enable → IDC pin 5 | Ke Q_K1 |
| 14 | E-Stop sense | Via U12 |
| 16, 17 | UART1 RS485 RX/TX | — |
| 18, 19, 23 | SPI MAX31856 | — |
| 21, 22 | I2C SDA/SCL | PCF8574 @ 0x20 |
| 25, 26, 27 | Tombol Drum/Heater/Air | Via opto |
| 32, 33 | UART2 Nextion RX/TX | 115200 |
| 34 | ADC Pot Air | — |
| 35 | ADC NTC box | — |

### 6.2 PCF8574 (U10) — alamat 0x20

| Pin | Fungsi | Catatan |
|-----|--------|---------|
| P0 | SPARE | — |
| P1 | DFPlayer BUSY | Input |
| P2 | RS485 DE/RE | Via **Q6 inverter** → default RX mode |
| P3 | Cooling fan ON/OFF | → IDC pin 7 → **Q7** → Q3 (P3 LOW = ON, PCF8574 sink) |

### 6.3 UART

| Port | Baud | Device | Pin |
|------|------|--------|-----|
| UART0 | 115200 | CP2102N USB | GPIO1 TX, GPIO3 RX |
| UART1 | 9600 | RS485 | GPIO17 TX, GPIO16 RX |
| UART2 | 115200 | Nextion | GPIO33 TX, GPIO32 RX |
| SoftSerial | 9600 | DFPlayer TX | GPIO4 |

### 6.4 Logika PWM exhaust (pot air)

```
Tombol AIR ON + pot ADC (GPIO34) → firmware hitung duty 0–100%
→ LEDC GPIO2 → IDC pin 8 → Q4 gate → kecepatan exhaust
```

---

## 7. Thermocouple

1. Probe **ungrounded**, kabel **shielded** 2m
2. Shield → GND **1 titik** di J1/J2
3. Jarak **≥50mm** dari kabel drum & heater
4. Ferrite + 100pF C0G + LDO U2 terpisah

| Channel | Posisi |
|---------|--------|
| Bean U4 | Zona biji drum |
| Env U5 | Udara exhaust/chamber |
| Box 40°C | **NTC1 GPIO35** (bukan TC) |

---

## 8. Harness kabel

### H1 — Panel → MAIN (J_H1)

| Label | Warna | Ke |
|-------|-------|-----|
| H1-01 | MERAH | +12V lampu (dari IO J4) |
| H1-02 | HITAM | GND |
| H1-03 | PUTIH/MERAH | SW2 Drum → U7 |
| H1-04 | PUTIH/BIRU | SW3 Heater → U8 |
| H1-05 | PUTIH/HIJAU | SW4 Air → U9 |
| H1-06 | PUTIH/KUNING | E-Stop aux → U12 |
| H1-07A | MERAH MUDA | Pot +3.3V |
| H1-07B | HITAM | Pot GND |
| H1-07C | COKLAT | Pot wiper → GPIO34 |
| H1-08 | ORANYE | Nextion 5V |
| H1-09 | HITAM | Nextion GND |
| H1-10 | HIJAU | Nextion TX → GPIO32 |
| H1-11 | BIRU | Nextion RX → GPIO33 |
| H1-12 | KUNING | RS485 A |
| H1-13 | HIJAU MUDA | RS485 B |
| H1-14 | HITAM | RS485 GND |
| H1-15 | MERAH MUDA | Speaker + |
| H1-16 | PUTIH | Speaker − |

> **Label harness ≠ nomor pin header J_H1.**  
> `H1-12..14` = label kabel RS485 → landas di terminal **J3** (A/B/GND).  
> Pada footprint **J_H1**: pin **12/13** = UART2 Nextion; pin **15/16** = RS485_A/B.  
> Speaker `H1-15/16` → **J10**, bukan pin J_H1 sembarangan.

### H2A — MAIN → Probe TC (bukan lewat IO)

| Label | Warna | Ukuran | Ke |
|-------|-------|--------|-----|
| H2A-01..03 | COKLAT/PUTIH/DRAIN | TC ext | **MAIN J1** → seat **U4_MOD** Bean |
| H2A-04..06 | COKLAT/PUTIH/DRAIN | TC ext | **MAIN J2** → seat **U5_MOD** Env |

### H2B — IO → Mesin (daya & beban)

| Label | Warna | Ukuran | Ke |
|-------|-------|--------|-----|
| H2B-07..08 | MERAH/HITAM | 1,5mm² | J6 Drum |
| H2B-09..10 | MERAH/HITAM | 0,5mm² | J7 Cooling |
| H2B-11..12 | MERAH/HITAM | 0,5mm² | J8 Exhaust |
| H2B-13 | BIRU | 2,5mm² | Heater N (titik N panel) |
| H2B-14 | COKLAT | 2,5mm² | Heater L (J5 SSR OUT) |
| H2B-15 | KUN-HIJ | 2,5mm² | PE chassis |

### H3 — Power Safety (panel → IO box) — WAJIB

| Label | Warna | Ukuran | Fungsi |
|-------|-------|--------|--------|
| H3-01 | COKLAT TEBAL | 2,5mm² | L mains → SW5 → F4 (ke IO) |
| H3-02 | BIRU TEBAL | 2,5mm² | N mains → SW5 → PSU N |
| H3-03 | KUN-HIJ | 2,5mm² | PE panel → J_PE |
| H3-04 | COKLAT | 1,5mm² | F4 cabang → E-Stop **NC#1** → PSU L |
| H3-05 | MERAH | 1,0mm² | +12V setelah F3 → E-Stop **NC#2** → K1 coil path |

### Routing TC

Pisahkan kabel TC min **50mm** dari drum 1,5mm² dan heater 2,5mm².

---

## 9. Logika tombol & HMI

| Aksi | Hasil |
|------|-------|
| Tombol **DRUM** latched ON | GPIO12 → drum motor ON |
| Tombol **HEATER** latched ON | Heater boleh jalan (jika HMI/profil minta + TC OK) |
| Tombol **AIR** latched ON | Exhaust PWM dari pot / auto boleh jalan |
| Tanpa tombol fisik | Output terkait **selalu OFF** (manual & auto) |
| DRUM ON | Q2 ON |
| HEATER ON | Q_K1 ON → K1+SSR ON (TC OK) |
| AIR ON + pot | GPIO2 PWM exhaust (duty = pot) |
| Nextion Cooling | PCF8574 P3 → Q3 ON/OFF |
| E-Stop | NC#1+NC#2 buka → semua mati |

**Firmware wajib:** heater OFF jika TC fault **atau** tombol HEATER OFF; air PWM=0 jika tombol AIR OFF; drum OFF jika tombol DRUM OFF; LEDC duty=0 saat boot; watchdog aktif.

---

## 10. Hitungan daya

| Beban | Arus | Proteksi |
|-------|------|----------|
| Heater | 16,4A (17,4A @ 207V) | F1=25A |
| Drum | 5,0A DC | F_DRUM=7,5A |
| Fans+elektronik | ~1,8A DC | F3=10A |
| **Total 12V** | **~6,8A** | PSU 12,5A ✅ |
| Coil K1 | ~0,4A | Q_K1 IRLZ44N ✅ |

---

## 11. Checklist schematic

- [x] E-Stop **XB4BS8444**
- [x] Main switch **OT25FT2 25A 2P**
- [x] Contactor **LC1D25JD** + **Q_K1** + **D_K1**
- [x] EMI hanya cabang PSU
- [x] DFPlayer **GPIO4**, USB **UART0**
- [x] Exhaust **GPIO2 LEDC PWM**, Cooling **PCF8574 P3 + Q7**
- [x] RS485 DE via **P2 + Q6 inverter**
- [x] Panel harness ke **J_H1**, bukan IDC
- [x] TC shield 1 titik GND
- [ ] PCB layout + DRC (fase berikutnya)
- [ ] Netlist ERC clean di KiCad (verifikasi saat buka proyek)

---

## 12. Keputusan terkunci

| Item | Keputusan |
|------|-----------|
| TC Environment | Udara exhaust/chamber |
| Box suhu | NTC GPIO35 |
| Color sensor | DNP v1 |
| Pot Air | PWM exhaust via GPIO2 LEDC |
| Cooling HMI | Digital ON/OFF via PCF8574 P3 |
| Fase 1 module seats | MP1584 **FIXED 5V** + DevKitC + MAX31856×2 — **header pluggable** |
| Flash firmware Fase 1 | USB pada DevKitC; U11/J_USB path DNP |
| Buck 5V | **FIXED 5.0 V only** — dilarang modul adjustable/trimpot; QC 4.90–5.10 V |
| IO-PWR heater path | Tidak diubah Fase 1 |

---

## FILE PROYEK

| File | Isi |
|------|-----|
| `KiCad/D01E-MAIN-CTRL/` | Schematic KiCad PCB-01 (module seats v3.7) |
| `KiCad/D01E-IO-PWR/` | Schematic KiCad PCB-02 (hierarchical, locked Fase 1) |
| `D01E_Module_Optimization_Fase1.md` | Audit + QC junior module seats |
| `D01E_Harness_Wiring_Diagram.pdf` | Harness H1/H2A/H2B/H3 + J9 |
| `D01E_CE_Technical_File_Template.md` | CE technical file (lengkap) |
| `D01E_CE_Technical_File_Template.pdf` | CE summary PDF |
| `D01E_BOM_Production.csv` | BOM v3.7 (module seats) |
| `D01E_IO.txt` | Ringkasan I/O + module seats |
| `D01E_Paranoid_Audit_Report.md` | Audit final #4 |
| `scripts/gen_all.py` | Regenerate semua deliverable |

---

### Residual risk (dokumentasi CE, bukan blocker schematic)

| Risiko | Mitigasi |
|--------|----------|
| Contactor K1 welded | Inspeksi berkala, E-Stop NC#2 |
| Drum coast <500ms saat E-Stop | NC#1 matikan PSU, firmware freeze |
| Color sensor DNP v1 | Locked — header saja |
| Heater burst + K1 shared GPIO13 | Revisi **v3.6** — `D01E_Heater_Control_v3.6.md` |
| Modul longgar / vendor pinout | Standoff + first-article pin map — `D01E_Module_Optimization_Fase1.md` |

---

*D01E v3.7 — Fase 1 Module-on-PCB locked*
