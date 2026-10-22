# Langkah Lengkap — Angka Axis + Kurva BT / ET / RoR

Gambar grid: **22.jpg** (637×346), pic ID **22**  
Asumsi Picture grid di **x=69 y=95 w=637 h=346** — sesuaikan jika beda.

---

## A. Komponen baru di page2

### 1. Waveform `s0` (kurva)

Taruh **di atas area grid** (hanya area plot, bukan label):

| Attribute | Nilai |
|-----------|--------|
| objname | **s0** |
| x | **127** (69+58) |
| y | **109** (95+14) |
| w | **573** |
| h | **282** |
| dis | **573** |
| ch | **3** |
| bco | **65535** |
| pco0 | **2016** (BT hijau) |
| pco1 | **31** (ET biru) |
| pco2 | **64512** (RoR oranye) |

### 2. Label Y tetap — `ty0` … `ty5`

Text kecil, **xcen=1**, **isbr=0**, font 11:

| objname | x | y | Text |
|---------|---|---|------|
| ty0 | 42 | 385 | 0 |
| ty1 | 42 | 330 | 50 |
| ty2 | 42 | 275 | 100 |
| ty3 | 42 | 220 | 150 |
| ty4 | 42 | 165 | 200 |
| ty5 | 42 | 110 | 250 |

*(Tidak perlu di-update script — selalu 0–250)*

### 3. Label X dinamis — `tx0` … `tx10`

Text, **xcen=1**, font 11, **y=399**:

| objname | x |
|---------|---|
| tx0 | 127 |
| tx1 | 184 |
| tx2 | 241 |
| tx3 | 298 |
| tx4 | 355 |
| tx5 | 412 |
| tx6 | 469 |
| tx7 | 526 |
| tx8 | 583 |
| tx9 | 640 |
| tx10 | 697 |

### 4. Timer `tm0` (jika belum)

| Attribute | Nilai |
|-----------|--------|
| tim | 1000 |
| en | 0 |
| vis | 0 |

### 5. Program.s — pastikan ada:

```
int roastEn=0
```

(sys0, sys1, sys2 sudah ada)

---

## B. Paste event (copy dari folder `nextion_paste/`)

| File | Tempel di |
|------|-----------|
| `02_page2_Preinitialize.txt` | page2 → **Preinitialize** |
| `03_tm0_Timer_Event.txt` | **tm0** → Timer Event |
| `04_b1_START_TouchPress.txt` | **b1** → Touch Press |
| `05_b2_STOP_TouchPress.txt` | **b2** → Touch Press |
| `06_b3_RESET_TouchPress.txt` | **b3** → Touch Press |

**Touch Release** b1/b2/b3 → **kosongkan**.

---

## C. Urutan layer (z-order)

Dari belakang ke depan:
1. Picture grid (22.jpg)
2. **s0** waveform
3. ty0–ty5, tx0–tx10
4. Tombol & angka t0–t4

---

## D. Uji simulator

1. **F5** → Debug → page2
2. **START** → timer jalan, **tx0–tx10** update, **3 kurva** muncul
3. Tunggu 1 menit → titik kurva bergeser kanan
4. **STOP** → timer & kurva berhenti
5. **RESET** → grafik kosong, tx kembali 0

---

## E. Warna kurva saat START (simulator dummy)

| Channel | Data | Skala Y |
|---------|------|---------|
| 0 BT | 185°C | 0–250°C |
| 1 ET | 210°C | 0–250°C |
| 2 RoR | 8 | 0–30 °C/menit |

Hardware nanti: ESP32 kirim `add s0,...` dari sensor MAX31856.
