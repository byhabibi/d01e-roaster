# HEATER LEVEL — b4 UP / b5 DOWN

## Aturan
- **b4** (UP): `heatPct` +10, maks **100**
- **b5** (DOWN): `heatPct` -10, minimum **0**
- Tampilan **t6**: `0%`, `10%`, … `100%`

## Copy-paste di Nextion Editor

| File | Tempel di |
|------|-----------|
| `01_Program.s_tambahan.txt` | Tab **Program** → tambah `int heatPct=0` |
| `02_page2_Preinitialize.txt` | page2 → **Preinitialize** (sudah ada `heatPct=0` + `t6.txt="0%"`) |
| `09_b4_HEATER_UP_TouchPress.txt` | **b4** → **Touch Press** |
| `10_b5_HEATER_DOWN_TouchPress.txt` | **b5** → **Touch Press** |

## Penting: jangan pakai t6 untuk timer

File `03_tm0_Timer_Event.txt` pakai `covx` ke **t1/t2** sebagai scratch timer (bukan t5/t6), karena panel **NX8048P070** tidak mendukung `sprintf`.

## Uji simulator
1. F5 → page roast (page2)
2. Klik **b4** berkali-kali → `10%` … `100%`, tidak lewat 100%
3. Klik **b5** → turun per 10%, berhenti di `0%`

## ESP32 (nanti)
Kirim `heatPct` ke MCU lewat `printh` saat tombol ditekan — lihat `D01E_Heater_Control_v3.6.md` §4.3.

Tambahkan di akhir event **b4** dan **b5** (setelah update t6):

```
printh A5
printh 04
printh heatPct
printh FF
printh FF
printh FF
```
