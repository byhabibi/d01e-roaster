# Mode ESP32 — panduan singkat

**Semua file copy-paste ada di folder:**

```
firmware/D01E_Roaster/nextion_paste/esp32_mode/
```

Buka **`CARA_PAKAI.md`** di folder itu untuk tabel lengkap.

---

## Quick reference

| Komponen | File |
|----------|------|
| Preinitialize page2 | `02_page2_Preinitialize.txt` |
| tm0 Timer | `03_tm0_Timer_Event.txt` |
| b1 START Press | `04_b1_START_TouchPress.txt` |
| b2 STOP Press | `05_b2_STOP_TouchPress.txt` |
| b3 RESET Press | `06_b3_RESET_TouchPress.txt` |
| b4 HEATER UP | `09_b4_HEATER_UP_TouchPress.txt` |
| b5 HEATER DOWN | `10_b5_HEATER_DOWN_TouchPress.txt` |

**Touch Release** b1/b2/b3 → kosongkan (file `*_TouchRelease.txt`).

---

## Waveform ID

Cek **s0** di Editor → default firmware `NX_WAVE_ID = 48`.
