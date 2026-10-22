#!/usr/bin/env python3
"""D01E HMI patcher — uses Nextion2Text parser for structure."""
from __future__ import annotations

import shutil
import struct
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
sys.path.insert(0, str(SCRIPT_DIR.parent.parent.parent / "_tools" / "Nextion2Text"))

# fallback temp clone
import os

TMP = Path(os.environ.get("TEMP", "/tmp")) / "Nextion2Text"
if TMP.exists():
    sys.path.insert(0, str(TMP))

from Nextion2Text import HMI, PageHeader  # noqa: E402

HMI_DEFAULT = Path(
    r"d:\Hartanzah Manufaktur Indonesia\Master Design 2026\Danish 01 Electric\HMI_Nextion_7inch_D01_Standar.HMI"
)
HMI_CONTENT_HDR = 32


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


def replace_span(buf: bytearray, start: int, end: int, new: bytes) -> int:
    delta = len(new) - (end - start)
    buf[:] = buf[:start] + new + buf[end:]
    return delta


def get_hmi_items(h: HMI) -> list[dict]:
    return [
        {"name": o.name, "start": o.start, "size": o.size, "hdr_off": o._headerStart}
        for o in h.header.content
        if o
    ]


def shift_starts(items: list[dict], after: int, delta: int) -> None:
    for it in items:
        if it["start"] > after:
            it["start"] += delta


def write_items(raw: bytearray, items: list[dict]) -> None:
    for it in items:
        struct.pack_into("<I", raw, it["hdr_off"] + 16, it["start"])
        struct.pack_into("<I", raw, it["hdr_off"] + 20, it["size"])


def patch_program_s(raw: bytearray, items: list[dict]) -> None:
    prog = next(x for x in items if x["name"] == "Program.s")
    text = raw[prog["start"] : prog["start"] + prog["size"]].decode("ansi")
    if "int roastEn=0" in text:
        return
    needle = "int blinking=0\r\n" if "int blinking=0\r\n" in text else "int blinking=0\n"
    ins = "int roastEn=0\r\n".encode("ansi")
    pos = prog["start"] + text.index(needle) + len(needle)
    raw[pos:pos] = ins
    prog["size"] += len(ins)
    shift_starts(items, prog["start"], len(ins))


def parse_page_local(ph_raw: bytes) -> dict:
    _, size, _, count, *_ = struct.unpack_from("<IIIII?bbb16s16b", ph_raw, 0)
    hdr_size = struct.calcsize("<IIIII?bbb16s16b")
    comps = []
    for i in range(count):
        off, csz = struct.unpack_from("<II", ph_raw, hdr_size + i * 8)
        comps.append({"start_offset": off, "size": csz})
    return {"size": size, "count": count, "hdr_size": hdr_size, "comps": comps}


def find_comp(ph_raw: bytes, name: str) -> int:
    ph = parse_page_local(ph_raw)
    for i, c in enumerate(ph["comps"]):
        s = ph["hdr_size"] + c["start_offset"]
        e = s + c["size"]
        if bytes(name, "ascii") in ph_raw[s:e] and b"objname" in ph_raw[s:e]:
            return i
    raise RuntimeError(f"{name} not found")


def patch_codes_in_page(ph_raw: bytearray, comp_i: int, kind: str, lines: list[str]) -> None:
    ph = parse_page_local(ph_raw)
    c = ph["comps"][comp_i]
    s = ph["hdr_size"] + c["start_offset"]
    tag = b"codesload-" if kind == "codesload" else b"codesup-"
    j = ph_raw[s : s + c["size"]].find(tag)
    if j < 4:
        raise RuntimeError(f"{tag} missing comp {comp_i}")
    bs = s + j - 4
    _, _, end = read_codes_block(ph_raw, bs)
    delta = replace_span(ph_raw, bs, end, encode_codes_block(kind, lines))
    if not delta:
        return
    c["size"] += delta
    off0 = c["start_offset"]
    for other in ph["comps"]:
        if other["start_offset"] > off0:
            other["start_offset"] += delta
    ph["size"] += delta
    struct.pack_into("<I", ph_raw, 4, ph["size"])
    for i, comp in enumerate(ph["comps"]):
        struct.pack_into("<II", ph_raw, ph["hdr_size"] + i * 8, comp["start_offset"], comp["size"])


def build_tm0(template: bytes) -> bytes:
    blob = bytearray(template)
    for i in range(len(blob) - 3):
        if blob[i : i + 4] == struct.pack("<I", 10):
            blob[i : i + 4] = struct.pack("<I", 28)
    timer = encode_codes_block(
        "codestimer",
        [
            "if(roastEn==1)",
            "{",
            "sys0++",
            'sprintf t0.txt,"%02d:%02d",sys0/60,sys0%60',
            't1.txt="185.2"',
            't2.txt="8.5"',
            't3.txt="210.0"',
            't4.txt="6.2"',
            "}",
        ],
    )
    ci = blob.find(b"codestimer-") - 4
    _, _, end = read_codes_block(blob, ci)
    replace_span(blob, ci, end, timer)
    ti = blob.find(b"tim\x00")
    if ti >= 0:
        k = ti + 3
        while k < ti + 20 and blob[k] == 0:
            k += 1
        blob[k : k + 2] = b"\xe8\x03"
    enm = blob.find(b"\x0b\x00\x00\x00en\x00")
    if enm >= 0:
        vp = enm + 6
        while vp < enm + 24 and blob[vp] == 0:
            vp += 1
        struct.pack_into("<I", blob, vp, 0)
    return bytes(blob)


def get_tm0_template(raw: bytes, h: HMI) -> bytes:
    p20 = next(o for o in h.header.content if o.name == "20.pa")
    ph = PageHeader(raw, p20.start)
    for comp in ph.components:
        s = p20.start + ph.headerSize + comp.startOffset
        chunk = raw[s : s + comp.size]
        if b"tm0" in chunk and b"codestimer" in chunk and len(chunk) > 500:
            return bytes(chunk)
    raise RuntimeError("tm0 template not found")


def patch_page2(raw: bytearray, items: list[dict]) -> None:
    p2 = next(x for x in items if x["name"] == "2.pa")
    page = bytearray(raw[p2["start"] : p2["start"] + p2["size"]])
    old_len = len(page)

    patch_codes_in_page(
        page,
        0,
        "codesload",
        [
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
        ],
    )
    patch_codes_in_page(
        page,
        find_comp(page, "b1"),
        "codesup",
        ["roastEn=1", "tm0.en=1", "printh A5", "printh 01", "printh FF", "printh FF", "printh FF"],
    )
    patch_codes_in_page(
        page,
        find_comp(page, "b2"),
        "codesup",
        ["roastEn=0", "tm0.en=0", "printh A5", "printh 02", "printh FF", "printh FF", "printh FF"],
    )
    patch_codes_in_page(
        page,
        find_comp(page, "b3"),
        "codesup",
        [
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
        ],
    )

    try:
        find_comp(page, "tm0")
    except RuntimeError:
        ph = parse_page_local(page)
        p20 = next(x for x in items if x["name"] == "20.pa")
        ph20 = PageHeader(raw, p20["start"])
        tmpl = None
        for comp in ph20.components:
            s = p20["start"] + ph20.headerSize + comp.startOffset
            chunk = raw[s : s + comp.size]
            if b"tm0" in chunk and b"codestimer" in chunk and len(chunk) > 500:
                tmpl = bytes(chunk)
                break
        if tmpl is None:
            raise RuntimeError("tm0 template not found")
        tm0_blob = build_tm0(tmpl)
        new_off = ph["size"] - ph["hdr_size"]
        page.extend(tm0_blob)
        ph = parse_page_local(page)
        new_count = ph["count"] + 1
        struct.pack_into("<I", page, 12, new_count)
        struct.pack_into("<I", page, 4, ph["size"] + len(tm0_blob))
        struct.pack_into("<II", page, ph["hdr_size"] + ph["count"] * 8, new_off, len(tm0_blob))

    new_page = bytes(page)
    delta = len(new_page) - old_len
    raw[p2["start"] : p2["start"] + old_len] = new_page
    for it in items:
        if it["name"] == "2.pa":
            it["size"] = len(new_page)
    shift_starts(items, p2["start"], delta)


def patch_hmi(path: Path) -> None:
    backup = path.with_suffix(path.suffix + ".bak")
    shutil.copy2(path, backup)
    h = HMI(str(path))
    raw = bytearray(h.raw)
    items = get_hmi_items(h)
    patch_program_s(raw, items)
    write_items(raw, items)
    patch_page2(raw, items)
    write_items(raw, items)
    path.write_bytes(raw)


def main() -> None:
    target = Path(sys.argv[1]) if len(sys.argv) > 1 else HMI_DEFAULT
    patch_hmi(target)
    print("Patched:", target)


if __name__ == "__main__":
    main()
