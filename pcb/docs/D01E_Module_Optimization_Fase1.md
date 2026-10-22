# D01E Fase 1 — Module-on-PCB Optimization

**Versi:** 1.1 — MP1584 **FIXED 5V** locked (no trimpot)  
**Status:** LOCKED untuk produksi 2 PCB  
**Referensi:** `D01E_Electrical_System_Design.md` v3.7 · `D01E_BOM_Production.csv`  
**Scope:** Ganti 3 jenis komponen diskrit di MAIN dengan modul berheader — tanpa mengubah jalur safety heater di IO-PWR.

---

## 1. Ringkasan keputusan

| Item | Keputusan Fase 1 |
|------|------------------|
| Arsitektur PCB | Tetap **2 board fabrikasi**: `D01E-MAIN-CTRL` + `D01E-IO-PWR` |
| Mounting modul | **Header 2.54 mm male/female — pluggable** |
| Diganti di MAIN | MP1584 **modul FIXED 5V**, ESP32-**DevKitC-32E**, MAX31856 **breakout ×2** |
| Buck 5V | **FIXED step-down 5.0 V only** — **LARANG** tipe adjustable / trimpot |
| Tidak diganti | PSU / K1 / SSR / MOSFET / fuse / EMI di IO-PWR; opto; RS485; DFPlayer; PCF8574 |
| Tujuan | Junior-friendly: mudah dimengerti, difabrikasi, diganti saat maintenance |

```
IO-PWR (industrial, locked)
  PSU 12V ──J9 pin1──► MAIN U3_MOD (MP1584 FIXED 5V module) ──► +5V_SYS
                                              │
                                              ▼
                                    U1_DEVKIT (ESP32-DevKitC)
                                         │
                    ┌────────────────────┼────────────────────┐
                    ▼                    ▼                    ▼
              U4_MOD Bean TC       U5_MOD Env TC         Rest (opto/RS485/…)
```

---

## 2. Apa yang diganti / tidak diganti

### Diganti (MAIN only)

| Seat silk | RefDes | Was (diskrit) | Becomes (modul) |
|-----------|--------|---------------|-----------------|
| `BUCK 5V FIXED` / `U3_MOD` | U3 | MP1584EN SOT-23-6 + L4 | **Modul buck FIXED 5V** (MP1584-class, ≥3A, **no pot**) |
| `ESP32 DevKit` / `U1_DEVKIT` | U1 | ESP32-WROOM-32E | ESP32-DevKitC-32E on dual 1×19 header |
| `BEAN TC MODULE` / `U4_MOD` | U4 | MAX31856ATI+ SOIC | Breakout MAX31856 (Adafruit / pin-compatible) |
| `ENV TC MODULE` / `U5_MOD` | U5 | MAX31856ATI+ SOIC | Breakout MAX31856 ×2 |

### Tidak diganti (Fase 1)

- Stack heater IO-PWR: F4, F1, K1, SSR1, Q_K1, E-Stop NC  
- PSU / EMI / fuse DC / MOSFET drum-cool-exhaust  
- Opto, MAX3485, PCF8574, DFPlayer seat  
- Tidak merge 1 PCB; MOSFET/RS485 module = Fase 2

### Implikasi service

| Item | Rule |
|------|------|
| Flash | USB pada **DevKitC** |
| U11 + J_USB | **DNP** — konflik UART0 jika dipasang bersama DevKit |
| L4 | **Dihapus** — inductor di dalam modul |
| U2 MCP1700 | Tetap → +3V3_ANA ke VIN seat U4/U5 |

---

## 3. Aturan mounting (junior tech)

1. Female header 2.54 mm di silk zona:
   - `U3_MOD` — **1×3**: `VIN`, `GND`, `VOUT` (FIXED 5V — **tidak ada pad ADJ**)
   - `U1_DEVKIT` — 2×19 DevKitC-32E
   - `U4_MOD` / `U5_MOD` — 1×8 MAX31856 breakout
2. Modul **plug-in** only — tanpa reflow chip modul.
3. Retention: nylon/brass M2.5–M3 standoff **wajib** (getaran drum).
4. Incoming QC modul buck: **tolak** jika ada trimpot / label “adjustable”.

### Spec buck FIXED 5V (locked production)

| Parameter | Nilai |
|-----------|--------|
| Tipe | **Fixed output 5.0 V** step-down (MP1584-class / Mini360-5V fixed / setara) |
| **Dilarang** | Modul **adjustable** dengan potensiometer/trimpot |
| Input | +12 V (J9 pin1) |
| Rating | ≥ **3 A** continuous |
| QC measure | VOUT vs GND = **4.90 … 5.10 V** saat +12 V, **tanpa** Nextion dulu |
| Jika di luar rentang | **REJECT** modul — jangan “setel trimpot” (tidak ada) |

Junior **tidak boleh** membeli / memasang modul yang bisa diputar tegangannya.

---

## 4. Pin map DevKit → net existing (GPIO locked v3.3)

Firmware `config.h` **tidak diubah**.

| Net board | GPIO | DevKitC pad (USB di atas) |
|-----------|------|---------------------------|
| +5V_SYS | — | **L19 `5V`** |
| GND | — | L14, R1, R7 |
| POT_AIR | 34 | L5 |
| NTC_ADC | 35 | L6 |
| UART2_RX (Nextion) | 32 | L7 |
| UART2_TX (Nextion) | 33 | L8 |
| BTN_DRUM | 25 | L9 |
| BTN_HEATER | 26 | L10 |
| BTN_AIR | 27 | L11 |
| ESTOP_SENSE | 14 | L12 |
| DRUM_EN | 12 | L13 |
| HEATER_EN | 13 | L15 |
| SPI_MOSI | 23 | R2 |
| I2C_SCL | 22 | R3 |
| UART0_TX | 1 | R4 (DevKit USB only) |
| UART0_RX | 3 | R5 |
| I2C_SDA | 21 | R6 |
| SPI_MISO | 19 | R8 |
| SPI_SCK | 18 | R9 |
| SPI_CS_BEAN | 5 | R10 |
| UART1_TX (RS485) | 17 | R11 |
| UART1_RX (RS485) | 16 | R12 |
| DFPLAYER_TX | 4 | R13 |
| EXHAUST_PWM | 2 | R15 |
| SPI_CS_ENV | 15 | R16 |

**Jangan** kawat L1 `3V3` DevKit ke rail board.

### MAX31856 module seat (U4 / U5) — pin 1…8

| Pad | Nama modul | Net board |
|-----|------------|-----------|
| 1 | VIN | +3V3_ANA |
| 2 | 3Vo | NC |
| 3 | GND | GND_SIG |
| 4 | DO (MISO) | SPI_MISO |
| 5 | CS | SPI_CS_BEAN (U4) / SPI_CS_ENV (U5) |
| 6 | CLK | SPI_SCK |
| 7 | DIN (MOSI) | SPI_MOSI |
| 8 | DRDY | NC |

TC wires: H2A → **J1/J2** → T+/T− modul.

### MP1584 FIXED module (U3) — 3 pin

| Pad | Net |
|-----|-----|
| 1 VIN | +12V_IN (J9 pin1) |
| 2 GND | GND_PWR |
| 3 VOUT | +5V_SYS |

Bypass board: C3 dekat VIN, C4 dekat VOUT (tetap).

---

## 5. Checklist fabrikasi + QC junior

### Incoming parts
- [ ] Buck = **FIXED 5V**, label jelas, **tidak ada trimpot**
- [ ] DevKitC-32E; MAX31856 breakout ×2 pin-compatible

### Board + headers
- [ ] Silk: `BUCK 5V FIXED`, `ESP32 DevKit`, `BEAN TC MODULE`, `ENV TC MODULE`
- [ ] Continuity J9-1 → U3 VIN; U3 VOUT → +5V_SYS

### Power (buck saja)
- [ ] +12 V on → ukur VOUT **4.90…5.10 V**
- [ ] Di luar rentang → ganti modul (**jangan** coba adjust)

### DevKit + TC
- [ ] DevKit hidup dari +5V_SYS; flash USB DevKit (U11 DNP)
- [ ] 2× TC baca; unplug/ganti modul tanpa solder chip
- [ ] Standoff nylon terpasang

### Maintenance swap buck
1. Power OFF + E-Stop  
2. Cabut modul → pasang **FIXED 5V** cadangan (verify incoming QC)  
3. Ukur 4.90…5.10 V → baru beban penuh  

---

## 6. Residual risk

| Risiko | Mitigasi |
|--------|----------|
| Vendor kirim adjustable by mistake | Incoming reject: ada pot = **REJECT** |
| VOUT di luar 4.90–5.10 V | Reject modul; jangan continue |
| EMI modul murah | C3/C4; short wiring VIN/VOUT |
| Modul longgar getaran | Standoff wajib |
| DevKit + U11 dual UART | U11 **DNP** |

---

## 7. File terdampak

| File | Peran |
|------|--------|
| `D01E_Module_Optimization_Fase1.md` | Dokumen ini (v1.1 FIXED) |
| `D01E_BOM_Production.csv` | MPN FIXED 5V; larangan adjustable |
| `D01E_Electrical_System_Design.md` | §4 + keputusan locked |
| `D01E_IO.txt` / `D01E_DEVNOTE.md` | Fixed buck |
| `KiCad/D01E-MAIN-CTRL/` | Seat 3P FIXED + silk |
| `D01E_Delivery/...` | Sync BOM + QC |

## 8. Fase 2 (out of scope)

MOSFET/RS485/opto modules; merge PCB; weaken K1/SSR/E-Stop — **tidak**.

---

*D01E Fase 1 v1.1 — FIXED 5V buck locked for production*
