# D01E HMI DevNote — Nextion Roast Screens (Page 2 + Page 16–27)

**Dokumen handoff untuk AI / developer.** Baca ini **sebelum** menganalisis ulang file `.HMI`.  
**Terakhir diperbarui:** 2026-07-12  
**Proyek:** Danish 01 Electric (D01E)  
**File HMI:** `HMI_Nextion_7inch_D01_Standar.HMI` (~26 MB, format biner proprietary)

---

## 1. Ringkasan Eksekutif

| Item | Nilai |
|------|-------|
| Layar roast **manual** | **page2** — heater & air dari user (pot + b4/b5) |
| Layar roast **otomatis** | **page16–page27** — heater & air dari **jadwal profil** ESP32 |
| Layar warm-up otomatis | **page4** — heater 100%, air 60%, target BT 180°C |
| Layout roast (page2 + 16–27) | `t18` timer, `t19`–`t22` suhu/RoR, `t0`–`t5` sumbu Y, `s0` kurva |
| Tombol roast | `bt0`=START/STOP, `bt7`=color, `bt8`=cooling, `b0` back, `b6` menu |
| Protokol ke ESP32 | `printh A5 01–0E FF FF FF` (5 baris terpisah) |
| Firmware | `firmware/D01E_Roaster/` — UART @ 115200 |
| Patch otomatis | ~~`patch_hmi_page2.py`~~ **NONAKTIF** — merusak file (lihat §2.5) |
| Backup HMI asli | `HMI_Nextion_7inch_D01_Standar.HMI.bak` |

**Status patch 2026-07-11:** Binary auto-patch **GAGAL** — merusak file di Editor. File dikembalikan dari `.bak`. Event page2 harus diisi **manual** di Nextion Editor (§6 + `nextion_paste/`).

### 2.5 Binary patch = RUSAK file (KRITIS)

Percobaan `patch_hmi_page2.py` (2026-07-11) menghasilkan:
> **"Wrong Hmifile or Hmifile has been damaged."**

Penyebab: insert komponen `tm0` + ubah ukuran event **tanpa** update header internal (CRC, `numberobj`, pointer `.pa`). File `.HMI` punya struktur terproteksi — **hanya Nextion Editor** yang boleh menulis.

**Jika file rusak:** restore dari backup:
```powershell
Copy-Item "HMI_Nextion_7inch_D01_Standar.HMI.bak" "HMI_Nextion_7inch_D01_Standar.HMI" -Force
```

**Cara aman:** edit manual di Editor (§6) atau paste dari `firmware/D01E_Roaster/nextion_paste/`.

---

## 2. Aturan Emas — JANGAN Ulangi Kesalahan Umum

### 2.1 File `.HMI` tidak bisa diedit seperti source code biasa

- Format **biner proprietary** Nextion Editor.
- **Terkunci** saat Nextion Editor membuka file → Cursor/script tidak bisa baca/tulis.
- Tool **Nextion2Text** hanya **baca** (export teks), **tidak bisa** menulis balik ke `.HMI`.
- Satu-satunya edit otomatis yang sudah diverifikasi: ~~**`patch_hmi_page2.py`**~~ **DINONAKTIFKAN — merusak file** (§2.5).

### 2.2 Simulator ≠ Hardware

| Mode | Timer & angka | `printh` |
|------|---------------|----------|
| **Simulator Editor** | Di-update script lokal `tm0` + `roastEn` | Hex muncul di "Simulator Return" |
| **Panel + ESP32** | Di-update firmware (`t0.txt`–`t4.txt`) | UART ke GPIO32/33 |

Tanpa `tm0` + Preinitialize, START di simulator **hanya** mengirim hex — tampilan tetap `00`.

### 2.3 Error `88 FF FF FF` = sintaks event salah

Penyebab umum:
- `printh A5 01 FF FF FF` **satu baris** → invalid
- `t0.txt=00:00` tanpa tanda kutip
- Referensi `tm0.en` padahal komponen `tm0` belum ada

**Benar:** satu perintah per baris (lihat §6).

### 2.4 File HMI berisi DUPLIKAT page2

File ini menyimpan **beberapa salinan** struktur page2 (riwayat internal Editor).  
**Hanya salinan TERAKHIR** yang aktif saat compile — offset sekitar **26 586 169+**.

Jangan patch offset dari salinan lama. Script patch sudah menarget salinan aktif (offset dari sesi 2026-07-11). Jika user save ulang struktur besar di Editor, **re-validasi offset** (§8).

---

## 3. Peta File Terkait HMI

```
Danish 01 Electric/
├── HMI_Nextion_7inch_D01_Standar.HMI      ← sumber utama (biner)
├── HMI_Nextion_7inch_D01_Standar.HMI.bak  ← backup sebelum patch otomatis
├── D01E_HMI_DEVNOTE.md                    ← dokumen ini
├── D01E_Electrical_System_Design.md       ← GPIO map v3.3 + Electrical v3.7 module seats
├── D01E_Module_Optimization_Fase1.md      ← DevKitC / FIXED 5V / flash USB DevKit
└── firmware/D01E_Roaster/
    ├── include/config.h                   ← nama objek Nextion (SINKRON dengan HMI)
    ├── src/main.cpp                       ← loop roast 1 Hz
    ├── src/nextion_roast.cpp              ← parser byte A5 01/02/03
    ├── scripts/
    │   ├── patch_hmi_page2.py             ← PATCH OTOMATIS (utama)
    │   └── hmi_parse_events.py            ← ekstrak event ke teks (read-only)
    ├── hmi_extracted/                     ← hasil Nextion2Text (bisa basi, regenerasi)
    ├── nextion_paste/                     ← blok copy-paste manual (fallback)
    ├── NEXTION_PAGE2_EVENTS.txt           ← referensi event singkat
    └── NEXTION_SIMULATOR_GUIDE.md         ← troubleshooting simulator
```

**Catatan:** `firmware/D01E_Roaster/README.md` masih menyebut `b0/b1/b2` dan `n0`–`n4` — **sudah usang**. Gunakan dokumen ini + `config.h`.

---

## 4. Registry Komponen Page 2 (FINAL)

### 4.1 Display roast (Text — wajib `.txt=`)

| Objek | ID | Label UI | Format | Di-update oleh |
|-------|-----|----------|--------|----------------|
| `t0` | 20 | Timer bawah RESET | `MM:SS` | tm0 (sim) / ESP32 (hardware) |
| `t1` | 21 | BEAN TEMP | `185.2` | tm0 dummy / MAX31856 BT |
| `t2` | 22 | Δ BEAN ROR | `8.5` | tm0 dummy / firmware RoR |
| `t3` | 23 | ET TEMP | `210.0` | tm0 dummy / MAX31856 ET |
| `t4` | 24 | Δ ET ROR | `6.2` | tm0 dummy / firmware RoR |
| `t5` | 25 | AIR LEVEL | `00` | **belum firmware v1** |
| `t6` | 26 | HEATER LEVEL | `00` | **belum firmware v1** |

### 4.2 Tombol kontrol roast

| Objek | ID | Fungsi | Touch Press | Touch Release |
|-------|-----|--------|-------------|---------------|
| `b1` | 8 | START | `vis b1,0` / `vis b2,1` / `vis b3,0` | roast + printh (§6.3) |
| `b2` | 9 | STOP | `vis b1,1` / `vis b2,0` / `vis b3,1` | stop + printh (§6.4) |
| `b3` | 11 | RESET | — | reset + printh (§6.5) |

### 4.3 Timer tersembunyi (simulator)

| Objek | Period | en default | vis | Fungsi |
|-------|--------|------------|-----|--------|
| `tm0` | 1000 ms | 0 | 0 | Tick dummy saat `roastEn==1` |

### 4.4 Komponen lain page2 (jangan hapus)

| Objek | Fungsi |
|-------|--------|
| `b0` | Back → tampilkan dialog `p7`, `b7`, `b8` |
| `b6` | Navigasi → `page page3` |
| `b7`, `b8` | Dialog konfirmasi back → `page1` / cancel |
| `bt0`, `bt1` | COOLING FAN, COLOR SENSOR (dual-state, belum firmware) |
| `b4`, `b5` | Panah AIR/HEATER (belum firmware) |
| `p0`–`p7` | Gambar / overlay UI |

---

## 4B. Auto Roast — Page 16–27 (WAJIB DIBACA AI)

### 4B.1 Perbedaan page2 vs page16–27

| | **Page 2** | **Page 16–27** |
|---|------------|----------------|
| Mode | **Manual roast** | **Automatic roast** |
| Heater | User (`b4`/`b5` → `A5 04`) | **ESP32 jadwal waktu** — pot/b4/b5 **diabaikan** |
| Airflow | Pot GPIO34 (PWM exhaust) | **ESP32 jadwal waktu** — pot **diabaikan** |
| UI | Timer, BT, ET, RoR, kurva `s0` | **Layout identik** (nama komponen sama) |
| Fahrenheit (page6) | Ya — t19–t22, sumbu Y, kurva | Ya — sama |
| Batch (page6, ≥5 menit) | Ya | Ya |
| START | `bt0` → `A5 01` | `bt0` → `A5 01` + jalankan **profil halaman itu** |

**Jangan** implementasikan page16–27 sebagai salinan manual page2. Firmware harus punya **12 profil** terpisah.

### 4B.2 Mapping page → profil (FINAL — konfirmasi user 2026-07-12)

| Page | Gram | Kecepatan | Level roast |
|------|------|-----------|-------------|
| **16** | 500 g | FAST | Light |
| **17** | 1000 g | FAST | Light |
| **18** | 500 g | FAST | Medium |
| **19** | 1000 g | FAST | Medium |
| **20** | 500 g | FAST | Dark |
| **21** | 1000 g | FAST | Dark |
| **22** | 500 g | SLOW | Light |
| **23** | 1000 g | SLOW | Light |
| **24** | 500 g | SLOW | Medium |
| **25** | 1000 g | SLOW | Medium |
| **26** | 1000 g | SLOW | Dark |
| **27** | 500 g | SLOW | Dark |

Pola: **page genap−1 / ganjil** = pasangan 500 g / 1000 g untuk kombinasi speed+level yang sama (kecuali urutan dark slow: 26=1000 g, 27=500 g).

### 4B.3 Navigasi HMI (referensi)

```
page7 → page8 (pilih level) → page13/14/15 (DARK/MEDIUM/LIGHT) → page20–21 / 18–19 / 16–17
page7 → page9  (pilih mode)  → page10/11/12                    → page22–23 / 24–25 / 26–27
```

Paste ESP32: `40_page16_Preinitialize.txt` … `40_page27_Preinitialize.txt` (page2 manual: `02_page2_Preinitialize.txt` → `A5 08`).

### 4B.4 Jadwal profil — heater % & airflow %

Waktu roast = **sejak Charge BT tercapai** (atau sejak START jika tanpa Charge BT). Lihat §4B.6.

#### FAST ROAST — 1000 g

**Light** — target ±9 menit — Charge BT: **210°C**

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–8 min | 100% | 50% |
| 8–9 min | 100% | 100% |

**Medium** — target ±10:30

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–6 min | 100% | 50% |
| 6–9 min | 90% | 60% |
| 9–10:30 | 80% | 100% |

**Dark** — target ±12 menit

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 100% | 50% |
| 5–9 min | 85% | 60% |
| 9–11 min | 75% | 80% |
| 11–12 min | 65% | 100% |

→ Page **17** Light, **19** Medium, **21** Dark

#### FAST ROAST — 500 g

Bean separuh → heater **tidak** 100% sepanjang roast.

**Light** — target ±8 menit — Charge BT: **205°C**

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 90% | 50% |
| 5–7 min | 80% | 60% |
| 7–8 min | 70% | 100% |

**Medium** — target ±9:30 — Charge BT: **205°C**

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 90% | 50% |
| 5–8 min | 75% | 60% |
| 8–9:30 | 65% | 100% |

**Dark** — target ±11 menit

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–4 min | 90% | 50% |
| 4–8 min | 70% | 60% |
| 8–10 min | 60% | 80% |
| 10–11 min | 55% | 100% |

→ Page **16** Light, **18** Medium, **20** Dark

#### SLOW ROAST — 1000 g

Development lebih panjang.

**Light** — target ±11 menit — Charge BT: **200°C**

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–6 min | 90% | 40% |
| 6–9 min | 80% | 50% |
| 9–11 min | 70% | 100% |

**Medium** — target ±12:30

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–6 min | 90% | 40% |
| 6–10 min | 75% | 50% |
| 10–12:30 | 65% | 100% |

**Dark** — target ±14 menit

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 90% | 40% |
| 5–9 min | 75% | 50% |
| 9–12 min | 65% | 70% |
| 12–14 min | 55% | 100% |

→ Page **23** Light, **25** Medium, **26** Dark

#### SLOW ROAST — 500 g

**Light** — target ±10 menit — Charge BT: **195°C**

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 80% | 40% |
| 5–8 min | 70% | 50% |
| 8–10 min | 60% | 100% |

**Medium** — target ±11:30

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 80% | 40% |
| 5–9 min | 70% | 50% |
| 9–11:30 | 60% | 100% |

**Dark** — target ±13 menit

| Waktu | Heater | Airflow |
|-------|--------|---------|
| 0–5 min | 80% | 40% |
| 5–9 min | 65% | 50% |
| 9–11 min | 55% | 70% |
| 11–13 min | 50% | 100% |

→ Page **22** Light, **24** Medium, **27** Dark

### 4B.5 Ringkasan page → profil firmware

| `config` / enum | Page | Profil |
|-----------------|------|--------|
| `AUTO_FAST_500_LIGHT` | 16 | §4B.4 FAST 500 Light |
| `AUTO_FAST_1000_LIGHT` | 17 | §4B.4 FAST 1000 Light |
| `AUTO_FAST_500_MED` | 18 | §4B.4 FAST 500 Medium |
| `AUTO_FAST_1000_MED` | 19 | §4B.4 FAST 1000 Medium |
| `AUTO_FAST_500_DARK` | 20 | §4B.4 FAST 500 Dark |
| `AUTO_FAST_1000_DARK` | 21 | §4B.4 FAST 1000 Dark |
| `AUTO_SLOW_500_LIGHT` | 22 | §4B.4 SLOW 500 Light |
| `AUTO_SLOW_1000_LIGHT` | 23 | §4B.4 SLOW 1000 Light |
| `AUTO_SLOW_500_MED` | 24 | §4B.4 SLOW 500 Medium |
| `AUTO_SLOW_1000_MED` | 25 | §4B.4 SLOW 1000 Medium |
| `AUTO_SLOW_1000_DARK` | 26 | §4B.4 SLOW 1000 Dark |
| `AUTO_SLOW_500_DARK` | 27 | §4B.4 SLOW 500 Dark |

### 4B.6 Perilaku Charge BT & akhir profil (FINAL — user 2026-07-12)

**Charge BT** = suhu saat user **memasukkan biji** ke mesin.

| Kondisi | Aksi |
|---------|------|
| START diklik, BT **&lt; Charge BT** | Panaskan (segmen pertama profil) hingga BT ≥ Charge BT |
| START diklik, BT **&gt; Charge BT** | Heater **OFF**, airflow **100%** hingga BT ≤ Charge BT |
| BT dalam **[Charge−5°C, Charge)** | DFPlayer track **2** — *"Prepare for charge"* setiap **3 detik** |
| BT **≥ Charge BT** | Timer roast **mulai dari 0**, jalankan jadwal profil |
| Profil tanpa Charge BT (0) | Timer langsung jalan dari START |

**Akhir profil** (waktu target **tepat**, mis. 10:30 = 630 s):

| Waktu | Aksi |
|-------|------|
| **30 detik sebelum selesai** | Track **3** — *"Prepare for remove coffee bean"* setiap **3 detik**; **cooling fan ON**; airflow **tetap** |
| **Saat target tercapai** | **Auto STOP** — heater **OFF**; airflow tetap; cooling fan **ON**; **tetap di page yang sama** |
| STOP (`A5 02`) / keluar (`A5 07`) | Lihat **§4B.7** — profil batal, semua output **OFF** |

**Cooling fan `bt8` (OR logic):** `fan ON = auto ON ∨ manual ON`. Manual bisa nyala lebih awal; **30 detik sebelum selesai** auto paksa ON (tombol HMI ikut `bt8.val=1`). Page2 manual: hanya `bt8`.

**Status bar (page16–27, ESP32):** Hanya **satu** aktif — `bt2` COOLING-DOWN, `bt3` WARMING-UP, `bt4` INPUT BEAN (+ track 2), `bt5` ROASTING IN PROCESS, `bt6` ROASTING DONE. Banner bawah: `p3.pic` → 210 / 214 / 213 / 212 / 211 (idle 165). Touch Press bt2–bt6 **kosong**. Kedip: `tm0` (INPUT BEAN), `tm1` (DONE).

**Audio:** `0002.mp3`, `0003.mp3` — `scripts/gen_auto_roast_audio.py`  
**UART:** `A5 0F [page]` — `40_page16_Preinitialize.txt` … `40_page27_Preinitialize.txt`

### 4B.7 Keluar halaman roast — Back / Menu (`A5 07`)

Berlaku **page2 (manual)** dan **page16–27 (auto)**. Tombol **Back** (`b0`) atau **Menu** (`b6`) harus kirim `A5 07` **sebelum** `page page1` / `page page3` (paste: `34_b0_BACK_LEAVE_TouchPress.txt`, `35_b6_MENU_LEAVE_TouchPress.txt`).

**Tidak ada** mode “roast lanjut di background” — keluar halaman = **hentikan sesi** sepenuhnya.

#### Alur firmware (`main.cpp` → `onRoastPageLeave()`)

| Langkah | Efek |
|---------|------|
| `roastPageActive = false` | ESP32 berhenti update `t18`–`t22`, kurva `s0`, status bar |
| `onStop()` + `autoRoastOnStop()` | `roastRunning = false`; fase auto → **IDLE**; audio DFPlayer stop |
| `autoRoastUnload()` | Profil page16–27 **dibuang** (`s_prof = null`); state internal **reset** |
| `stopAllRoastOutputs()` | Heater **OFF**, exhaust **OFF**, cooling **OFF**, color sensor **OFF** |

Serial monitor: `[ROAST] leave — all outputs OFF` dan `[AUTO] STOP`.

#### Per fase auto (saat user keluar)

| Fase saat keluar | Hasil |
|------------------|-------|
| PREHEAT_COOL / PREHEAT_HEAT | Preheat **batal** |
| INPUT BEAN (audio charge) | Audio stop; preheat **batal** |
| ROASTING (timer + jadwal) | Roast **batal**; timer profil tidak disimpan |
| FINISHED (cooling pasca-roast) | Cooling fan ikut **mati** — tidak lanjut di belakang layar |

#### Masuk lagi ke page roast

1. Preinitialize kirim `A5 08` (page2) atau `A5 0F [page]` (page16–27)  
2. Profil auto **dimuat ulang dari awal** (bukan lanjut dari waktu sebelumnya)  
3. User harus tekan **START** (`bt0`) lagi

#### Perbedaan dengan STOP saja (`A5 02`)

| | **STOP** (`bt0` / `A5 02`) | **Keluar** (`b0`/`b6` / `A5 07`) |
|---|---------------------------|-----------------------------------|
| Tetap di page roast | Ya | Tidak — HMI pindah page1/page3 |
| Profil auto di memori | Masih ter-load (bisa START lagi) | **Unload** — harus masuk page lagi |
| Output mesin | Auto OFF; **manual `bt8`** bisa tetap ON | **Semua OFF** (termasuk manual cooling) |
| Update UI ESP32 | Masih jalan (suhu/timer) | Berhenti |

> **Desain safety:** keluar halaman dianggap operator tidak lagi mengawasi roast — mesin tidak boleh heater/fan aktif tanpa UI roast. Perilaku **background roast** atau **cooling tetap ON setelah keluar** belum diimplementasikan (butuh spec produk jika diinginkan).

---

## 5. Variabel Global (Program.s)

Sudah ada di HMI (jangan duplikat):

```c
int toggle=0
int show_text=0
int blink_state=0
int blinking=0
int roastEn=0          // ← ditambahkan patch 2026-07-11
int sys0=0,sys1=0,sys2=0
baud=115200
dim=100
recmod=0
page 0                 // boot page
```

| Variabel | Pemakaian page2 |
|----------|-----------------|
| `roastEn` | 1 = roast jalan (simulator); START set 1, STOP/RESET set 0 |
| `sys0` | Counter detik untuk `sprintf t0.txt` di tm0 |

---

## 6. Event Code Lengkap (copy exact)

### 6.1 Page 2 → Preinitialize Event

```
vis b2,0
vis p7,0
vis b7,0
vis b8,0
roastEn=0
sys0=0
tm0.en=0
t0.txt="00:00"
t1.txt="0.0"
t2.txt="0.0"
t3.txt="0.0"
t4.txt="0.0"
```

**Cara buka di Editor:** tab Display → pilih `page2` di daftar → klik **canvas kosong** (bukan komponen) → tab **Preinitialize Event**. Bukan Program.s, bukan event tombol.

### 6.2 tm0 → Timer Event

```
if(roastEn==1)
{
  sys0++
  sprintf t0.txt,"%02d:%02d",sys0/60,sys0%60
  t1.txt="185.2"
  t2.txt="8.5"
  t3.txt="210.0"
  t4.txt="6.2"
}
```

### 6.3 b1 → Touch Release (START)

```
roastEn=1
tm0.en=1
printh A5
printh 01
printh FF
printh FF
printh FF
```

### 6.4 b2 → Touch Release (STOP)

```
roastEn=0
tm0.en=0
printh A5
printh 02
printh FF
printh FF
printh FF
```

### 6.5 b3 → Touch Release (RESET)

```
roastEn=0
tm0.en=0
sys0=0
t0.txt="00:00"
t1.txt="0.0"
t2.txt="0.0"
t3.txt="0.0"
t4.txt="0.0"
printh A5
printh 03
printh FF
printh FF
printh FF
```

---

## 7. Protokol UART HMI ↔ ESP32

### 7.1 HMI → ESP32 (Touch Press / Preinitialize)

| Perintah | Bytes serial | Pemicu |
|----------|----------------|--------|
| START | `A5 01 FF FF FF` | roast `bt0` Touch Release |
| STOP | `A5 02 FF FF FF` | roast `bt0` Touch Release (toggle OFF) |
| RESET | `A5 03 FF FF FF` | roast `b3` (opsional) |
| Heater % | `A5 04 [pct] FF FF FF` | page2 `b4`/`b5` |
| Cooling | `A5 05 [val] FF FF FF` | roast `bt8` |
| Color sensor | `A5 06 [val] FF FF FF` | roast `bt7` |
| Keluar roast page | `A5 07 FF FF FF` | roast `b0` back / `b6` menu — lihat **§4B.7** |
| Masuk page2 manual | `A5 08 FF FF FF` | page2 Preinitialize |
| Masuk page4 warm-up | `A5 09 FF FF FF` | page4 Preinitialize |
| Keluar page4 | `A5 0A FF FF FF` | page4 leave |
| SAVE temp limit | `A5 0B [hi][lo] FF FF FF` | page5 b4 SAVE |
| Masuk page5 limit | `A5 0C FF FF FF` | page5 Preinitialize |
| Unit C / F | `A5 0D 00` / `A5 0D 01` + `FF FF FF` | page6 c0 / c1 |
| Masuk page6 settings | `A5 0E FF FF FF` | page6 Preinitialize |
| Masuk auto roast | `A5 0F [page] FF FF FF` | page16–27 Preinitialize (byte2 = 16…27) |

Parser: `firmware/D01E_Roaster/src/nextion_roast.cpp`  
Konstanta: `firmware/D01E_Roaster/include/config.h`

### 7.2 ESP32 → HMI (setiap 1 detik saat roast aktif)

Perintah Nextion (contoh):

```
t0.txt="05:23"
t1.txt="185.2"
t2.txt="8.5"
t3.txt="210.0"
t4.txt="6.2"
```

Objek harus **Text** — perintah `.txt=` dengan string ber-quote.

### 7.3 Hardware pin (GPIO map v3.3 · PCB Electrical v3.7 Fase 1)

| Fungsi | Pin |
|--------|-----|
| Nextion TX → ESP32 RX | GPIO32 |
| Nextion RX ← ESP32 TX | GPIO33 |
| Baud | 115200 |
| MAX31856 Bean CS | GPIO5 |
| MAX31856 Env CS | GPIO15 |
| SPI SCK/MISO/MOSI | 18 / 19 / 23 |
| DFPlayer TX (perintah) | GPIO4 @ 9600 |

### 7.4 Audio DFPlayer — standar suara (WAJIB konsisten)

Semua clip suara pengumuman mesin D01E **harus memakai suara yang sama** agar pengalaman operator konsisten.

| Item | Nilai standar |
|------|----------------|
| **Voice TTS** | **`en-US-JennyNeural`** (wanita, ramah, bahasa Inggris) |
| **Engine** | Microsoft Edge TTS via Python **`edge-tts`** |
| **Script generate** | `firmware/D01E_Roaster/scripts/gen_warmup_audio.py` |
| **Template clip baru** | Salin script di atas → ganti `TEXT` + nama file `000N.mp3` |
| **Hardware** | DFPlayer Mini U13, TX ESP32 **GPIO4**, 9600 baud |
| **microSD** | FAT32, **`0001.mp3`**, **`0002.mp3`**, … di **root** kartu |
| **Volume firmware** | `DF_VOLUME` di `config.h` (default 25, skala 0–30) |

**Aturan untuk developer / AI berikutnya:**

1. **Jangan ganti voice** (pria, aksen lain, bahasa lain) tanpa persetujuan desain produk.
2. Setiap clip baru: generate dengan **`en-US-JennyNeural`**, export MP3, nomor track berurutan.
3. Catat track di tabel registry di bawah + tambah `#define DF_TRACK_...` di `config.h`.
4. Jika voice diganti suatu saat — **regenerate semua clip**; jangan campur Jenny dengan voice lain.

**Generate (PowerShell):**

```powershell
cd "d:\Hartanzah Manufaktur Indonesia\Master Design 2026\Danish 01 Electric\firmware\D01E_Roaster"
python -m pip install edge-tts   # sekali saja
python scripts\gen_warmup_audio.py
```

**Template Python untuk clip baru:**

```python
VOICE = "en-US-JennyNeural"   # JANGAN UBAH — standar produk D01E
TEXT = "Your new phrase here"
OUT = Path("audio_assets/0002.mp3")
```

#### Registry audio (perbarui tiap tambah clip)

| Track | File SD | Voice | Teks | Dipakai saat |
|-------|---------|-------|------|--------------|
| 1 | `0001.mp3` | Jenny Neural | *"The warm-up is almost complete"* | page4 warm-up, BT 175–179°C (ulang tiap 3 s) |
| 2 | — | Jenny Neural | *(belum ada)* | — |

Detail: `firmware/D01E_Roaster/audio_assets/README.md`  
Paste page4: `nextion_paste/esp32_mode/15–18_*.txt`

---

## 8. Edit HMI — Cara Aman (Manual di Editor)

> **JANGAN jalankan script patch apapun** (`patch_hmi_page2.py`, `patch_hmi_v2.py`, atau patch Program.s via Python) — semua merusak file saat dibuka di Editor.

### 8.1 Restore jika rusak

```powershell
cd "d:\Hartanzah Manufaktur Indonesia\Master Design 2026\Danish 01 Electric"
Copy-Item "HMI_Nextion_7inch_D01_Standar.HMI.bak" "HMI_Nextion_7inch_D01_Standar.HMI" -Force
```

### 8.2 Langkah manual (~5 menit)

1. Buka `HMI_Nextion_7inch_D01_Standar.HMI` di Nextion Editor
2. **Program.s** → tambah: `int roastEn=0`
3. Tambah **Timer** `tm0` di page2 (`tim=1000`, `en=0`, `vis=0`)
4. Copy-paste dari `firmware/D01E_Roaster/nextion_paste/`:

| File | Tempel ke |
|------|-----------|
| `02_page2_Preinitialize.txt` | page2 → Preinitialize Event |
| `03_tm0_Timer_Event.txt` | tm0 → Timer Event |
| `04_b1_START_TouchRelease.txt` | b1 → Touch Release |
| `05_b2_STOP_TouchRelease.txt` | b2 → Touch Release |
| `06_b3_RESET_TouchRelease.txt` | b3 → Touch Release |

5. Compile (F5) → uji simulator

Panduan singkat: `firmware/D01E_Roaster/nextion_paste/CARA_PAKAI.md`

### 8.3 Offset biner (referensi saja — jangan patch manual)

Offset di bawah hanya untuk **debug/read** dengan script parse, bukan untuk write:

| Target | Offset byte (.bak 2026-07-11) |
|--------|-------------------------------|
| page2 salinan aktif | ~26 586 169 |
| Preinitialize | ~26 586 788 |
| b2 Touch Release | ~26 593 514 |
| b3 Touch Release | ~26 595 017 |

File HMI berisi **duplikat** page2 — hanya salinan terakhir yang aktif.

---

## 9. Dual Mode: Simulator vs Mesin Nyata

| Aspek | Simulator (Editor) | Hardware |
|-------|-------------------|----------|
| Timer `t0` | `tm0` + `sys0` + `sprintf` | ESP32 `main.cpp` 1 Hz |
| Suhu `t1`–`t4` | Konstanta dummy di tm0 | MAX31856 + RoR 15 s |
| `tm0.en` | START=1, STOP/RESET=0 | Preinit=0; ESP32 tidak pakai tm0 |
| `printh` | Hex di Simulator Return | UART ke ESP32 |

**Konflik:** Jika `tm0.en=1` bersamaan ESP32 menulis `t0.txt`, tampilan bisa “berkedip”. Di production: pastikan Preinitialize `tm0.en=0` dan andalkan ESP32 saja (bisa hapus/disable tm0 Timer Event di build final).

---

## 10. Checklist Verifikasi

### Setelah patch / edit HMI

- [ ] Nextion Editor: Compile **0 Errors**
- [ ] Simulator page2: START → timer `00:01`, `00:02`, …
- [ ] Simulator: angka dummy muncul di t1–t4
- [ ] Simulator Return: `A5 01 FF FF FF` (bukan `88`)
- [ ] STOP menghentikan timer; RESET ke `00:00` / `0.0`
- [ ] Upload `.tft` ke panel 7"

### Dengan ESP32

- [ ] Flash `firmware/D01E_Roaster`
- [ ] UART 115200 terhubung
- [ ] START → t0 naik, suhu real (atau `---` jika TC lepas)
- [ ] `config.h` nama objek = `t0`–`t4`, `b1`–`b3`

---

## 11. Workflow untuk AI Agent

```
1. Baca dokumen ini (D01E_HMI_DEVNOTE.md) — JANGAN analisa HMI dari nol
2. JANGAN jalankan patch_hmi_page2.py (merusak file)
3. Untuk perubahan event page2 roast:
   a. Edit manual di Nextion Editor (§6 + nextion_paste/)
   b. Atau minta user paste dari nextion_paste/*.txt
4. Jika file rusak: restore dari .HMI.bak (§8.1)
5. Ubah firmware: config.h + main.cpp — sinkron nama objek
6. Verifikasi §10
```

**Jangan:**
- Asumsikan `n0`–`n4` (sudah diganti `t0`–`t4`)
- Asumsikan `b0/b1/b2` untuk START/STOP/RESET (sekarang `b1/b2/b3`)
- Tulis ulang `.HMI` dari teks tanpa script patch
- Patch offset tanpa cek duplikat page2

---

## 12. Pekerjaan Belum (v2+)

| Fitur | Komponen / halaman | Catatan |
|-------|-------------------|---------|
| AIR level manual | page2 only | Pot GPIO34 — page16–27 pakai override jadwal |
| HEATER manual | page2 `b4`/`b5` | page16–27 nonaktif saat auto |
| COLOR SENSOR | `bt7` | dual-state — semua roast pages |
| Grafik roast | `s0` waveform | `add s0.id,ch,val` — layout 169.jpg |
| Artisan / Modbus | RS485 | **Selesai** — `D01E_Modbus_Artisan.md` |
| Background roast saat keluar page | — | **Belum** — keluar = stop total (§4B.7) |

**Selesai (2026-07-12):** Auto roast 12 profil page16–27, Charge BT §4B.6, `A5 0F`, DF track 2/3, cooling `bt8` post-roast, Modbus/Artisan, panel interlock

---

## 13. Riwayat Perubahan

| Tanggal | Perubahan |
|---------|-----------|
| 2026-07-11 | `n0`–`n4` → `t0`–`t4` (Text) |
| 2026-07-11 | START/STOP/RESET = `b1`/`b2`/`b3` |
| 2026-07-11 | Fix printh multi-baris (error 88) |
| 2026-07-11 | Tambah `roastEn`, `tm0`, Preinitialize lengkap |
| 2026-07-11 | `patch_hmi_page2.py` — **GAGAL**, merusak file; restore .bak; edit manual saja |
| 2026-07-11 | Firmware `D01E_Roaster` v1 page2 |
| 2026-07-12 | page4 warm-up otomatis (`A5 09`/`A5 0A`) |
| 2026-07-12 | Standar audio: **Jenny Neural** (`en-US-JennyNeural`) — §7.4 |
| 2026-07-12 | page6 settings: unit C/F (page2 only) + batch NVS (`A5 0D`/`A5 0E`) |
| 2026-07-12 | Layout roast baru: `t18`–`t22`, sumbu Y `t0`–`t5`, `bt0`/`bt7`/`bt8` |
| 2026-07-12 | **§4B** Mapping page16–27 → 12 profil auto roast (FAST/SLOW × 500/1000 g × L/M/D) |
| 2026-07-12 | **§4B.6** Charge BT + akhir profil — firmware `auto_roast*.cpp`, UART `A5 0F`, audio 0002/0003 |
| 2026-07-12 | **§4B.7** Keluar roast page (`A5 07`) — stop total, unload profil, tidak background roast |
| 2026-07-12 | §7.1 UART — sinkron `bt0`/`bt7`/`bt8`, `A5 0F` page16–27 |

---

## 14. Referensi Silang

- Hardware sistem: `D01E_Electrical_System_Design.md` v3.7 (GPIO map locked v3.3) · `D01E_Module_Optimization_Fase1.md`
- PCB handoff: `D01E_DEVNOTE.md`
- Firmware GPIO + objek: `firmware/D01E_Roaster/include/config.h`
- Event singkat: `firmware/D01E_Roaster/NEXTION_PAGE2_EVENTS.txt`
- Troubleshooting sim: `firmware/D01E_Roaster/NEXTION_SIMULATOR_GUIDE.md`
