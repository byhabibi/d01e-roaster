#!/usr/bin/env python3
"""Generate DFPlayer MP3 for auto roast prompts — voice: en-US-JennyNeural."""
import asyncio
import sys
from pathlib import Path

try:
    import edge_tts
except ImportError:
    print("Install: python -m pip install edge-tts", file=sys.stderr)
    sys.exit(1)

VOICE = "en-US-JennyNeural"
TRACKS = [
    ("0002.mp3", "Prepare for charge"),
    ("0003.mp3", "Prepare for remove coffee bean"),
]
OUT_DIR = Path(__file__).resolve().parent.parent / "audio_assets"


async def main() -> None:
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    for filename, text in TRACKS:
        out = OUT_DIR / filename
        communicate = edge_tts.Communicate(text, VOICE)
        await communicate.save(str(out))
        print(f"Wrote {out}")
    print("Copy 0002.mp3 and 0003.mp3 to microSD (DFPlayer track 2 & 3).")


if __name__ == "__main__":
    asyncio.run(main())
