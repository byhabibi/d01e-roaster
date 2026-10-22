#!/usr/bin/env python3
import re
from pathlib import Path

path = Path(r"d:\Hartanzah Manufaktur Indonesia\Master Design 2026\Danish 01 Electric\HMI_Nextion_7inch_D01_Standar.HMI")
data = path.read_bytes()
print("size", len(data))

start = data.find(b"int toggle=0")
end = data.find(b"page 0", start)
prog = data[start:end]
print("\n=== Program.s ===")
for line in re.split(rb"[\r\n]+", prog):
    if line.strip():
        print(line.decode("ascii", "replace"))

# last page2 roastEn=1 block
idx = data.rfind(b"roastEn=1")
print("\n=== b1 event (last roastEn=1 context) ===")
print(repr(data[idx - 120 : idx + 150]))

idx2 = data.rfind(b"if(roastEn==1)")
print("\n=== tm0 timer event ===")
print(repr(data[idx2 : idx2 + 250]))

idx3 = data.rfind(b"vis b2,0")
print("\n=== last vis b2,0 context ===")
print(repr(data[idx3 : idx3 + 200]))
