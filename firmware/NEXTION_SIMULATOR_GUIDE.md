Nextion Page 2 — Kenapa Simulator Tidak Jalan & Cara Perbaiki
=============================================================

## Kenapa tekan START di Editor tidak mengubah t0–t4?

Simulator Nextion **HANYA** menjalankan script di dalam file .HMI.
Firmware ESP32 (baca thermocouple + update t1–t4) **TIDAK** berjalan di simulator.

Alur yang benar di mesin asli:
  [Tombol START] → printh A5 01 → [ESP32 terima] → ESP32 kirim t0.txt / t1.txt ...

Di simulator tanpa ESP32:
  [Tombol START] → printh A5 01 → keluar ke "Simulator Return" (hex)
  → t0/t1 **tidak berubah** kecuali Anda tambah script Nextion lokal.

---

## Error `88 FF FF FF` di Simulator Return

Artinya: **instruksi invalid** di Touch Release Event tombol START.

Penyebab umum:
1. `printh A5 01 FF FF FF` satu baris — beberapa versi Editor gagal parse
2. `t0.txt=00:00` tanpa tanda kutip — harus `t0.txt="00:00"`
3. Referensi komponen lama `n0` padahal sudah diganti `t0`

---

## PERBAIKAN event tombol (pakai baris terpisah)

### b1 START — Touch Release Event
```
printh A5
printh 01
printh FF
printh FF
printh FF
```

### b2 STOP
```
printh A5
printh 02
printh FF
printh FF
printh FF
```

### b3 RESET
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

Setelah edit: Compile → coba START lagi.
Simulator Return harus menampilkan: `A5 01 FF FF FF` (bukan 88).

---

## OPSIONAL — timer jalan di Simulator (tanpa ESP32)

Tambah komponen **Timer** `tm0` di page 2 (bisa kecil/disembunyikan):
  - Attribute `tim` = 1000  (1 detik)
  - `en` = 0

### b1 START — tambahkan di bawah printh:
```
roastEn=1
sys0=0
t0.txt="00:00"
tm0.en=1
```

### tm0 — Timer Event:
```
if(roastEn==1)
{
  sys0+=1
  sprintf t0.txt,"%02d:%02d",sys0/60,sys0%60
}
```

### b2 STOP — tambahkan:
```
roastEn=0
tm0.en=0
```

Catatan: `sprintf` tersedia di Nextion Enhanced (NX8048K070).
Di mesin asli dengan ESP32, timer tetap dikendalikan ESP32 — blok tm0 opsional untuk uji simulator saja.

---

## Uji suhu di Simulator (dummy)

Di kotak **Instruction input** simulator, ketik:
```
t1.txt="185.3"
```
Enter → BEAN TEMP harus berubah. Jika tidak, nama komponen salah.

---

## Uji lengkap (mesin asli)

1. Upload HMI ke panel Nextion
2. Flash firmware: firmware/D01E_Roaster/
3. Sambungkan UART: ESP32 GPIO33→RX panel, GPIO32→TX panel, GND
4. TC Bean/Env ke MAX31856
5. Tekan START di panel → t0 jalan, t1/t3 dari probe
