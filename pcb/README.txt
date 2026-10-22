D01E — Paket PCB MANUFACTURER
Tanggal audit: 2026-07-15 | Electrical v3.7 · Fase 1 module seats

******************************************************************************
  STATUS FABRIKASI — WAJIB BACA
******************************************************************************
  PCB-01 MAIN-CTRL Gerber di folder ini = LAYOU LAMA (2026-07-10)
  positions.csv masih: ESP32-WROOM-32E · MP1584EN SOT-23-6 · L4 4.7uH · MAX31856 SOIC
  Truth Fase 1 (BOM + schematics): DevKitC seat · buck FIXED 5V modul · MAX31856 breakout ×2
  → JANGAN order / kirim Gerber MAIN sampai layout seats di-regen + DRC 0 + export baru.

  PCB-02 IO-PWR: part lock OK (K1=LC1D25JD, SSR=D2440) tetapi QC DRC masih ada violation
  → JANGAN order sampai engineer konfirmasi DRC = 0.

  Acuan desain (bukan Gerber lama): schematics/INDEX.html · BOM · docs Module Fase1
******************************************************************************

MULAI DI SINI
-------------
1. Baca README_Fase1.txt + file ini
2. Buka schematics/INDEX.html (L0→L3) — aturan seats / track / BOM lock
3. BOM/D01E_BOM_Production.csv — order part sesuai BOM (bukan sesuai positions.csv lama)
4. QC/QC_Module_Fase1.txt + *-drc.rpt / *-erc.rpt

ISI PAKET
---------
README.txt / README_Fase1.txt
BOM/D01E_BOM_Production.csv
docs/
  D01E_Electrical_System_Design.md     v3.7
  D01E_Module_Optimization_Fase1.md    DevKit / FIXED 5V / U11 DNP
  D01E_Modbus_Artisan.md
schematics/                            SVG L0–L3 + INDEX (sama truth tech)
QC/                                    ERC/DRC + QC_Module_Fase1.txt
PCB-01_MAIN-CTRL/Gerbers/              *** STALE — jangan fab ***
PCB-02_IO-PWR/Gerbers/                 part lock OK; tunggu DRC 0

BOM LOCK FASE 1 (harus match order)
- U1 = ESP32-DevKitC-32E (bukan WROOM soldered)
- U3 = MP1584-FIXED-5V modul ≥3A (LARANG trimpot/adjustable; QC 4.90–5.10 V)
- U4/U5 = MAX31856 breakout pluggable
- U11 CP2102 + jalur J_USB = DNP (flash via USB DevKit)
- L4 discrete inductor = TIDAK ada di BOM Fase 1
- K1 = LC1D25JD coil 12VDC (BUKAN LC1D25BD 24V)
- SSR1 = Crydom D2440

SPESIFIKASI PCB (setelah Gerber baru valid)
- 2 layer FR4, 1.6 mm · Cu 1 oz · mask hijau · silk putih
- Finish: HASL atau ENIG (konfirmasi Hartanzah)
- Impedansi: tidak dikontrol

FILE GERBER (setelah regen)
- F/B Cu Mask Paste Silk · Edge_Cuts · drill · job.gbrjob · positions.csv
- positions.csv HARUS memuat seat DevKit / BUCK FIXED / TC MODULE — bukan WROOM/L4

KONTAK ENGINEER sebelum PO fabrikasi.
