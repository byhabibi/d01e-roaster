#!/usr/bin/env python3
"""Audit HMI_Nextion_7inch_D01_Standar.HMI against D01E firmware requirements."""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]
CANDIDATES = [
    ROOT / "HMI_Nextion_7inch_D01_Standar.HMI",
    ROOT / "HMI_Nextion_7inch_D01_Standar.HMI.GOOD_COPY",
]


def audit(data: bytes, label: str) -> list[str]:
    issues: list[str] = []
    ok: list[str] = []

    def need(name: str, cond: bool, hint: str = "") -> None:
        if cond:
            ok.append(name)
        else:
            issues.append(f"{name}: MISSING — {hint}")

    need("Program.s roastEn", b"int roastEn=0" in data or b"int roastEn = 0" in data,
         "tambah int roastEn=0 di Program.s")
    need("Program.s sys0-4", b"sys0=0" in data and b"sys4=0" in data,
         "tambah int sys0=0..sys4=0")
    need("heatPct global", b"heatPct" in data, "01_Program.s_heatPct.txt")
    need("tempLim global", b"tempLim" in data, "settings page5")

    need("obj t18 timer", b"t18" in data)
    need("obj t19 BT", b"t19" in data)
    need("obj t20 ET", b"t20" in data)
    need("obj t21-t22 RoR", b"t21" in data and b"t22" in data)
    need("obj bt0 START", b"bt0" in data)
    need("obj bt7 color", b"bt7" in data)
    need("obj bt8 cooling", b"bt8" in data)
    need("status bt2-bt6", all(f"bt{i}".encode() in data for i in range(2, 7)))
    need("banner p3", b"p3" in data)

    need("UART A5 01 START", b"\xa5\x01" in data)
    need("UART A5 02 STOP", b"\xa5\x02" in data)
    need("UART A5 03 RESET", b"\xa5\x03" in data)
    need("UART A5 07 leave", b"\xa5\x07" in data)
    need("UART A5 08 page2", b"\xa5\x08" in data)
    need("UART A5 0F auto", b"\xa5\x0f" in data.lower())
    need("UART A5 05 cool", b"\xa5\x05" in data)
    need("UART A5 06 color", b"\xa5\x06" in data)
    need("UART A5 09 warmup", b"\xa5\x09" in data)
    need("UART A5 0E settings", b"\xa5\x0e" in data)

    for pg in range(16, 28):
        pat = bytes([0xA5, 0x0F, pg])
        if data.count(pat) < 1:
            issues.append(f"page{pg} Preinit: A5 0F {pg} tidak ditemukan")

  # page2 manual enter
    if data.count(b"\xa5\x08") < 1:
        issues.append("page2 Preinit: A5 08 tidak ditemukan")

    if b"sprintf" in data:
        issues.append("RISIKO: sprintf masih ada — compile error di beberapa Nextion")
    if b"n0.val" in data or b"n0.txt" in data:
        issues.append("RISIKO: objek lama n0 masih ada (harus t18-t22)")
    if b"vis b1,0" in data and b"bt0" in data:
        issues.append("RISIKO: event lama b1 START mungkin masih ada")

    tm0_blink = b"tm0.en=1" in data or b"tm0.en=1" in data
    if b"tm0.en=0" not in data:
        issues.append("tm0.en=0 di preinit mungkin belum ada")

    print(f"\n{'='*60}\nFILE: {label}\nSIZE: {len(data):,} bytes\n{'='*60}")
    print(f"OK ({len(ok)}):", ", ".join(ok[:8]), "..." if len(ok) > 8 else "")
    if issues:
        print(f"\nISSUES ({len(issues)}):")
        for i, x in enumerate(issues, 1):
            print(f"  {i}. {x}")
    else:
        print("\nTidak ada issue struktural dari scan biner.")

    # Program.s excerpt
    start = data.find(b"int toggle=0")
    if start < 0:
        start = data.find(b"int roastEn")
    end = data.find(b"page 0", start) if start >= 0 else -1
    if start >= 0 and end > start:
        print("\nProgram.s (cuplikan):")
        for line in data[start:end].decode("ascii", "replace").splitlines()[:15]:
            if line.strip():
                print(" ", line.rstrip())

    return issues


def main() -> int:
    audited = False
    for path in CANDIDATES:
        if not path.is_file():
            continue
        try:
            data = path.read_bytes()
        except PermissionError:
            print(f"SKIP (file terkunci / dibuka Editor): {path.name}")
            continue
        audit(data, path.name)
        audited = True

    if not audited:
        print("Tidak bisa baca file HMI — tutup Nextion Editor lalu jalankan ulang.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
