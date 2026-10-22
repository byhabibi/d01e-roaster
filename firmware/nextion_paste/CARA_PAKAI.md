# Paste cepat — Nextion Page 2 (simulator)

File `.HMI` **tidak bisa diedit dari Cursor** (format biner + dikunci saat Nextion Editor terbuka).

Yang perlu Anda lakukan: **1 komponen baru** + **6x copy-paste** (±3 menit).

## Satu-satunya langkah drag (tidak bisa otomatis)

1. Tab **Display** → **page2**
2. Toolbox → **Timer** → taruh di canvas
3. Attribute: **objname = tm0**, **tim = 1000**, **en = 0**, **vis = 0**

## Copy-paste (buka file .txt, Ctrl+A, Ctrl+C, tempel di event)

| File | Tempel di |
|------|-----------|
| `01_Program.s_tambahan.txt` | Tab **Program** → Program.s |
| `02_page2_Preinitialize.txt` | page2 (klik canvas kosong) → **Preinitialize Event** |
| `03_tm0_Timer_Event.txt` | pilih **tm0** → **Timer Event** |
| `04_b1_START_TouchPress.txt` | **b1** → **Touch Press** (ganti semua isi); **Touch Release** kosongkan |
| `05_b2_STOP_TouchPress.txt` | **b2** → **Touch Press**; **Touch Release** kosongkan |
| `06_b3_RESET_TouchPress.txt` | **b3** → **Touch Press**; **Touch Release** kosongkan |
| `09_b4_HEATER_UP_TouchPress.txt` | **b4** → **Touch Press** |
| `10_b5_HEATER_DOWN_TouchPress.txt` | **b5** → **Touch Press** |

Detail heater: lihat `12_HEATER_LEVEL.md`.

## Compile

F5 → Simulator → START → timer jalan, Return = `A5 01 FF FF FF`
