#!/usr/bin/env python3
"""Patch D01E Nextion HMI page2 roast events + tm0 timer (binary HMI).

WARNING (2026-07-11): Full binary patch CORRUPTS the .HMI file — Nextion Editor
reports "Wrong Hmifile or Hmifile has been damaged." Do NOT run this script.
Use manual paste via Nextion Editor (see nextion_paste/ and D01E_HMI_DEVNOTE.md).
"""
from __future__ import annotations

import sys

if __name__ == "__main__":
    print("ERROR: patch_hmi_page2.py is DISABLED — it corrupts the HMI file.")
    print("Restore from .bak if needed, then edit events manually in Nextion Editor.")
    print("See: D01E_HMI_DEVNOTE.md section 6 (event code) and firmware/D01E_Roaster/nextion_paste/")
    sys.exit(1)

# --- Original implementation kept below for reference only (do not execute) ---

import shutil
import struct
import sys
from pathlib import Path

HMI_DEFAULT = Path(
    r"d:\Hartanzah Manufaktur Indonesia\Master Design 2026\Danish 01 Electric\HMI_Nextion_7inch_D01_Standar.HMI"
)

PAGE2_START = 26_586_169
PAGE2_PREINIT = 26_586_788
B2_CODESUP = 26_593_514
B3_CODESUP = 26_595_017
TM0_INSERT = 26_599_386
TM0_TEMPLATE = (25_878_733, 25_880_111)
PROGRAM_INSERT_AFTER = b"int blinking=0\r\n"
PROGRAM_INSERT = b"int roastEn=0\r\n"


def read_codes_block(data: bytes | bytearray, start: int) -> tuple[str, list[bytes], int]:
    i = start
    hdr_len = struct.unpack_from("<I", data, i)[0]
    i += 4
    hdr = data[i : i + hdr_len].decode("ascii")
    i += hdr_len
    count = int(hdr.split("-", 1)[1])
    lines: list[bytes] = []
    for _ in range(count):
        ln = struct.unpack_from("<I", data, i)[0]
        i += 4
        lines.append(data[i : i + ln])
        i += ln
    return hdr, lines, i


def encode_codes_block(kind: str, lines: list[str]) -> bytes:
    header = f"{kind}-{len(lines)}".encode("ascii")
    out = bytearray()
    out += struct.pack("<I", len(header))
    out += header
    for line in lines:
        b = line.encode("ascii")
        out += struct.pack("<I", len(b))
        out += b
    return bytes(out)


def replace_span(data: bytearray, start: int, end: int, new: bytes) -> int:
    delta = len(new) - (end - start)
    data[:] = data[:start] + new + data[end:]
    return delta


def page2_has_tm0(data: bytes) -> bool:
    region = data[PAGE2_START : PAGE2_START + 160_000]
    return b"objname\x00\x00\x00\x00\x00\x00\x00\x00\x00tm0" in region


def patch_program_s(data: bytearray) -> int:
    pos = data.find(PROGRAM_INSERT_AFTER)
    if pos < 0:
        raise RuntimeError("Program.s anchor not found")
    insert_at = pos + len(PROGRAM_INSERT_AFTER)
    if data.find(b"int roastEn=0", pos, pos + 200) >= 0:
        return 0
    data[insert_at:insert_at] = PROGRAM_INSERT
    return len(PROGRAM_INSERT)


def patch_page2_preinit(data: bytearray, offset: int) -> int:
    _, _, end = read_codes_block(data, offset)
    lines = [
        "vis b2,0",
        "vis p7,0",
        "vis b7,0",
        "vis b8,0",
        "roastEn=0",
        "sys0=0",
        "tm0.en=0",
        't0.txt="00:00"',
        't1.txt="0.0"',
        't2.txt="0.0"',
        't3.txt="0.0"',
        't4.txt="0.0"',
    ]
    return replace_span(data, offset, end, encode_codes_block("codesload", lines))


def patch_b2_stop(data: bytearray, offset: int) -> int:
    _, _, end = read_codes_block(data, offset)
    lines = [
        "roastEn=0",
        "tm0.en=0",
        "printh A5",
        "printh 02",
        "printh FF",
        "printh FF",
        "printh FF",
    ]
    return replace_span(data, offset, end, encode_codes_block("codesup", lines))


def patch_b3_reset(data: bytearray, offset: int) -> int:
    _, _, end = read_codes_block(data, offset)
    lines = [
        "roastEn=0",
        "tm0.en=0",
        "sys0=0",
        't0.txt="00:00"',
        't1.txt="0.0"',
        't2.txt="0.0"',
        't3.txt="0.0"',
        't4.txt="0.0"',
        "printh A5",
        "printh 03",
        "printh FF",
        "printh FF",
        "printh FF",
    ]
    return replace_span(data, offset, end, encode_codes_block("codesup", lines))


def build_tm0_component(template: bytes) -> bytes:
    blob = bytearray(template)
    old_id = struct.pack("<I", 10)
    new_id = struct.pack("<I", 27)
    replaced = 0
    for i in range(len(blob) - 3):
        if blob[i : i + 4] == old_id:
            blob[i : i + 4] = new_id
            replaced += 1
    if replaced == 0:
        raise RuntimeError("tm0 template ID not patched")

    timer_lines = [
        "if(roastEn==1)",
        "{",
        "sys0++",
        'sprintf t0.txt,"%02d:%02d",sys0/60,sys0%60',
        't1.txt="185.2"',
        't2.txt="8.5"',
        't3.txt="210.0"',
        't4.txt="6.2"',
        "}",
    ]
    new_timer = encode_codes_block("codestimer", timer_lines)
    start = blob.find(b"codestimer-")
    if start < 4:
        raise RuntimeError("codestimer block not found in tm0 template")
    start -= 4
    _, _, end = read_codes_block(blob, start)
    replace_span(blob, start, end, new_timer)

    # Period 400 ms -> 1000 ms (uint16 LE after tim attribute)
    tim_pos = blob.find(b"tim\x00")
    if tim_pos >= 0:
        k = tim_pos + 3
        while k < tim_pos + 20 and blob[k] == 0:
            k += 1
        if k + 2 <= len(blob) and blob[k : k + 2] == b"\x90\x01":
            blob[k : k + 2] = b"\xe8\x03"

    # Disabled on load
    en_marker = b"\x0b\x00\x00\x00en\x00"
    pos = blob.find(en_marker)
    if pos >= 0:
        val_pos = pos + len(en_marker)
        while val_pos < pos + 24 and blob[val_pos] == 0:
            val_pos += 1
        if val_pos + 4 <= len(blob):
            struct.pack_into("<I", blob, val_pos, 0)

    return bytes(blob)


def insert_tm0(data: bytearray, offset: int, template_source: bytes) -> int:
    if page2_has_tm0(data):
        return 0
    t0, t1 = TM0_TEMPLATE
    component = build_tm0_component(template_source[t0:t1])
    data[offset:offset] = component
    return len(component)


def patch_hmi(path: Path) -> None:
    backup = path.with_suffix(path.suffix + ".bak")
    if not backup.exists():
        shutil.copy2(path, backup)

    template_source = backup.read_bytes()
    data = bytearray(backup.read_bytes())

    # Apply in ascending file offset order; shift later offsets after each grow.
    delta = 0
    delta += patch_program_s(data)

    off_pre = PAGE2_PREINIT + delta
    delta += patch_page2_preinit(data, off_pre)

    off_b2 = B2_CODESUP + delta
    delta += patch_b2_stop(data, off_b2)

    off_b3 = B3_CODESUP + delta
    delta += patch_b3_reset(data, off_b3)

    off_tm0 = TM0_INSERT + delta
    delta += insert_tm0(data, off_tm0, template_source)

    path.write_bytes(data)


def main() -> None:
    target = Path(sys.argv[1]) if len(sys.argv) > 1 else HMI_DEFAULT
    if not target.exists():
        raise SystemExit(f"HMI not found: {target}")
    patch_hmi(target)
    print(f"Patched: {target}")
    print(f"Backup:  {target.with_suffix(target.suffix + '.bak')}")


if __name__ == "__main__":
    main()
