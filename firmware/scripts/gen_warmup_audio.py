#!/usr/bin/env python3
"""Generate DFPlayer MP3 clips for D01E — voice standard: en-US-JennyNeural.

See D01E_HMI_DEVNOTE.md §7.4 — do NOT change VOICE without product approval.
Copy this script for new tracks (0002.mp3, 0003.mp3, …).
"""
import asyncio
import sys
from pathlib import Path

try:
    import edge_tts
except ImportError:
    print("Install: python -m pip install edge-tts", file=sys.stderr)
    sys.exit(1)

# --- D01E voice standard (JANGAN UBAH tanpa persetujuan desain) ---
VOICE = "en-US-JennyNeural"
TEXT = "The warm-up is almost complete"
OUT = Path(__file__).resolve().parent.parent / "audio_assets" / "0001.mp3"


async def main() -> None:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    communicate = edge_tts.Communicate(TEXT, VOICE)
    await communicate.save(str(OUT))
    print(f"Wrote {OUT}")
    print("Copy to microSD root as 0001.mp3 (or mp3/0001.mp3 per your DFPlayer folder layout).")


if __name__ == "__main__":
    asyncio.run(main())
