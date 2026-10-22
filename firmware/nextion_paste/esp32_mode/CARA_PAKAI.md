# Copy-paste ESP32 mode — panel fisik + firmware

Folder: `nextion_paste/esp32_mode/`

**ESP32** mengurus: timer `t18`, suhu `t19`–`t22`, sumbu Y `t0`–`t5`, kurva `add s0.id,...`, RESET `cle s0.id,255`.

Nextion hanya: **tombol** → `printh A5` + visual lokal.

---

## Sebelum paste

1. **tm0/tm1** — tetap ada untuk animasi tab proses; `tm0.en=0` di Preinitialize
2. **s0** waveform — nama `s0` di **setiap** roast page (page2 + page16..27)
3. **Program.s** — `int roastEn=0` + `int sys0=0,sys1=0,sys2=0,sys3=0,sys4=0` **sebelum** `baud=`

## Aturan roast pages (WAJIB)

Berlaku **page2** dan **page16 sampai page27** (layout identik):

| UART | Arti |
|------|------|
| `A5 08` | Masuk **page2** manual |
| `A5 0F [page]` | Masuk **page16–27** auto (byte2 = 16…27) |
| `A5 07` | Keluar roast page → stop total, unload profil (lihat `D01E_HMI_DEVNOTE.md` §4B.7) |
| `A5 01` | START roast |
| `A5 02` | STOP |
| `A5 03` | RESET timer + kurva |
| `A5 04 [pct]` | Heater % |
| `A5 05 [val]` | Cooling fan (`bt8`) |
| `A5 06 [val]` | Color sensor (`bt7`) |

- **Keluar** (b0 back / b6 menu) → kirim `A5 07` **sebelum** `page page1` / `page page3`
- Keluar → ESP32 matikan: heater, exhaust, cooling, color sensor + STOP roast

---

## Komponen roast (semua page 2 + 16–27)

| Fungsi | Komponen |
|--------|----------|
| Timer | `t18` |
| Bean temp | `t19` |
| Env temp | `t20` |
| RoR bean | `t21` |
| RoR env | `t22` |
| Sumbu Y | `t0`–`t5` |
| Sumbu X | `t6`–`t17` |
| Kurva | `s0` |
| START/STOP | `bt0` (dual-state) |
| Color sensor | `bt7` |
| Cooling fan | `bt8` |
| Status COOLING-DOWN | `bt2` |
| Status WARMING-UP | `bt3` |
| Status INPUT BEAN | `bt4` |
| Status ROASTING | `bt5` |
| Status DONE | `bt6` |
| Banner status | `p3` (pic 165/210/214/213/212/211) |
| Back | `b0` |
| Menu | `b6` |
| Heater UP/DOWN | `b4` / `b5` (jika ada) |

---

## Tabel paste utama

| File | Tempel di |
|------|-----------|
| `30_roast_pages_Preinitialize.txt` | **DEPRECATED** — pakai `40_pageNN_*` per halaman |
| `40_page16_Preinitialize.txt` … `40_page27_Preinitialize.txt` | **page16–27** → Preinitialize (`A5 0F` + nomor page) |
| `02_page2_Preinitialize.txt` | **page2 manual** → Preinitialize (`A5 08`) |
| `04_bt0_START_TouchRelease.txt` | **bt0** → **Touch Release** (simulator) |
| `04_bt0_START_TouchRelease_HW.txt` | **bt0** → Touch Release (panel + ESP32) |
| `04_bt0_START_TouchPress.txt` | **bt0** → Touch Press → kosong |
| `03_tm0_Timer_Event_SIMULATOR.txt` | **tm0** → Timer Event (hanya uji F5) |
| `32_bt7_COLOR_SENSOR_TouchPress.txt` | **bt7** → Touch Press |
| `33_bt8_COOLING_FAN_TouchPress.txt` | **bt8** → Touch Press |
| `37_bt2_bt6_status_CLEAR_TouchPress.txt` | **bt2–bt6** → Touch Press → **kosong** |
| `03_tm0_Blink_CHARGE_Timer_Event.txt` | **tm0** → Timer Event (panel: kedip INPUT BEAN) |
| `03_tm1_Blink_Timer_Event.txt` | **tm1** → Timer Event (panel: kedip ROASTING DONE) |
| `34_b0_BACK_LEAVE_TouchPress.txt` | **b0** (back) → Touch Press |
| `35_b6_MENU_LEAVE_TouchPress.txt` | **b6** (menu) → Touch Press |
| `09_b4_HEATER_UP_TouchPress.txt` | **b4** → Touch Press (jika ada) |
| `10_b5_HEATER_DOWN_TouchPress.txt` | **b5** → Touch Press (jika ada) |
| `36_b3_RESET_TouchPress.txt` | **b3** RESET → Touch Press (opsional) |
| `15_page4_Preinitialize.txt` | page4 Preinitialize |
| `24_page6_Preinitialize.txt` | page6 Preinitialize |
| `25_c0_CELSIUS_TouchPress.txt` | page6 c0 |
| `26_c1_FAHRENHEIT_TouchPress.txt` | page6 c1 |

File lama `02_page2_Preinitialize.txt` = isi sama dengan `30_*` (untuk page2 saja).

---

## Page16–27 — profil auto roast

**Bukan manual seperti page2.** ESP32 jalankan jadwal heater + airflow (§4B di `D01E_HMI_DEVNOTE.md`).

| Page | Gram | Mode | Level |
|------|------|------|-------|
| 16 | 500 g | FAST | Light |
| 17 | 1000 g | FAST | Light |
| 18 | 500 g | FAST | Medium |
| 19 | 1000 g | FAST | Medium |
| 20 | 500 g | FAST | Dark |
| 21 | 1000 g | FAST | Dark |
| 22 | 500 g | SLOW | Light |
| 23 | 1000 g | SLOW | Light |
| 24 | 500 g | SLOW | Medium |
| 25 | 1000 g | SLOW | Medium |
| 26 | 1000 g | SLOW | Dark |
| 27 | 500 g | SLOW | Dark |

**Paste yang sama** di setiap page16–27:

1. Preinitialize → `40_pageNN_Preinitialize.txt` (termasuk `bt2`–`bt6.val=0`, `p3.pic=165`)
2. bt0, bt7, bt8, b0, b6 → file 04, 32, 33, 34, 35
3. bt2–bt6 Touch Press → **kosong** (`37_bt2_bt6_status_CLEAR_TouchPress.txt`)
4. tm0 Timer Event → `03_tm0_Blink_CHARGE_Timer_Event.txt` (panel fisik)
5. tm1 Timer Event → `03_tm1_Blink_Timer_Event.txt`
6. Hapus logic simulator lama di bt0 (toggle `p3`/`p4` tanpa `printh`)

**UI** sama page2 (timer, suhu, RoR, kurva, batch, °F). **Mesin:** heater/air **otomatis** per profil — lihat `D01E_HMI_DEVNOTE.md` §4B.

---

## Page6 — settings (global)

| UART | Arti |
|------|------|
| `A5 0E` | Masuk page6 → ESP32 kirim batch `t0` + sync `c0`/`c1` |
| `A5 0D 00` | Celsius |
| `A5 0D 01` | Fahrenheit |

- **Fahrenheit** ubah tampilan di **semua roast page** (t19–t22, sumbu Y t0–t5, skala kurva s0)
- **Batch** `t0` di page6: +1 roast ≥ **5 menit** di page2 **atau** page16–27
- Page4/page5 tetap **°C**

---

## bt0 START/STOP

**Penting:** UART `A5 01` / `A5 02` harus di **Touch Release** bt0 (bukan Press) — lihat `04_bt0_START_TouchRelease.txt`.

| File | Pakai saat |
|------|------------|
| `04_bt0_START_TouchRelease.txt` | **Simulator Editor (F5)** — + `roastEn` + `tm0.en` |
| `04_bt0_START_TouchRelease_HW.txt` | **Panel fisik + ESP32** — hanya `printh` |
| `04_bt0_START_TouchPress.txt` | Kosongkan logic lama di Press |

---

## Simulator Editor (F5) — timer/suhu/kurva tidak jalan?

**Penyebab:** Simulator **tidak** terhubung ke ESP32. Folder `esp32_mode` sengaja kosongkan `tm0` — tanpa script lokal, `t18`–`t22` dan `s0` tetap 0.

**Perbaikan (uji UI di PC):**

1. **tm0** → Timer Event → paste `03_tm0_Timer_Event_SIMULATOR.txt`
2. **bt0** → **Touch Release** → paste `04_bt0_START_TouchRelease.txt` (bukan Press saja)
3. **bt0** → Touch Press → kosong / hapus toggle `p3`/`p4` lama
4. **Program.s** — pastikan ada:
```
int roastEn=0
int sys0=0,sys1=0,sys2=0
int sys3=0,sys4=0
```
5. Preinit page16 → `40_page16_Preinitialize.txt` (`A5 0F 16`, bukan `A5 08`)
6. Compile **F5** → START → timer & kurva dummy harus jalan

**Error compile `sprintf` / `sys0%60`:** Nextion tidak mendukung — pakai isi terbaru `03_tm0_Timer_Event_SIMULATOR.txt` (`covx` + `if(sys2==0)`).

**Panel fisik + ESP32:**

1. Preinit → `40_pageNN_*` (auto) atau `02_page2_*` (manual `A5 08`)
2. bt0 → **Touch Release** → `04_bt0_START_TouchRelease_HW.txt`
3. tm0 Timer Event → `03_tm0_Blink_CHARGE_Timer_Event.txt` (bukan kosong)
4. tm1 Timer Event → `03_tm1_Blink_Timer_Event.txt`
5. bt2–bt6 Touch Press → kosong
6. Flash ESP32, UART GPIO32↔TX panel, GPIO33↔RX
7. START → serial monitor: `[AUTO] START` atau `[ROAST] MANUAL START`

**Cek log Simulator:** setelah START harus muncul `A5 01 FF FF FF`. Jika `A5 02` → event masih di Press atau `bt0.val` terbalik.

---

## Setelah paste

1. **Ctrl+S** → Compile **F5**
2. Upload HMI ke panel
3. Flash ESP32: `pio run -t upload`
4. UART: GPIO33→RX panel, GPIO32→TX, GND bersama

---

## Uji (page2 atau page16–27)

| Aksi | Harapan |
|------|---------|
| Masuk page | Serial `[ROAST] enter` |
| bt0 START | Timer `t18` jalan, `t19`/`t20` update, kurva `s0` |
| Page6 → Fahrenheit | Sumbu Y `t0`–`t5` jadi °F; suhu & kurva ikut °F |
| Roast ≥5 min | Batch page6 `t0` +1 |
| b0/b6 keluar | Serial `[ROAST] leave`, output mati |

---

## Simulator tanpa ESP32

Pakai `nextion_paste/` biasa (bukan `esp32_mode/`).
