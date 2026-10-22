# Grafik Responsif — BT / ET / RoR + X Axis Dinamis

## Ukuran grafik (page2)

| Komponen | w | h | dis (s0) |
|----------|---|---|----------|
| **p8** (grid PNG) | **650** | **330** | — |
| **s0** (waveform) | **650** | **330** | **650** |

File grid: `firmware\nextion_assets\roast_grid_654x330.png` (ukuran asli **650×330**)  
**p8** x≈69 y≈95 — **w=650 h=330** (jangan scale).

---

## Konsep X responsif

- Grid PNG = garis saja + label Y (0, 50 … 250)
- **tx0–tx10** = label menit, di-update script
- **visStart** geser saat roast > **76 menit** (lebar plot ÷ 8 px/menit)

---

## Label X — tx0 sampai tx10

Posisi x relatif page (p8 di x=69):

| objname | x | y |
|---------|---|---|
| tx0 | 111 | 418 |
| tx1 | 166 | 418 |
| tx2 | 221 | 418 |
| tx3 | 276 | 418 |
| tx4 | 331 | 418 |
| tx5 | 386 | 418 |
| tx6 | 441 | 418 |
| tx7 | 496 | 418 |
| tx8 | 551 | 418 |
| tx9 | 606 | 418 |
| tx10 | 649 | 418 |

font 12, xcen=1, isbr=0

---

## Paste event

| File | Event |
|------|--------|
| `02_page2_Preinitialize.txt` | Preinitialize |
| `03_tm0_Timer_Event.txt` | tm0 Timer |
| `04_b1_START_TouchPress.txt` | b1 Touch Press |
| `05_b2_STOP_TouchPress.txt` | b2 Touch Press |
| `06_b3_RESET_TouchPress.txt` | b3 Touch Press |
