#!/usr/bin/env python3
"""Draw grid on 22.jpg — lines only, margins for dynamic axis numbers."""
from pathlib import Path

from PIL import Image, ImageDraw

PATH = Path(__file__).resolve().parents[2] / "nextion_assets" / "22.jpg"
PX_PER_MIN = 8
TEMP_MAX = 250

# Margins: left = room for Y numbers; bottom = room for X numbers above "ROASTING TIME"
MARGIN_L, MARGIN_R, MARGIN_T, MARGIN_B = 58, 6, 14, 50

C_GRID_MINOR = (220, 220, 220)
C_GRID_MAJOR = (175, 175, 175)
C_BORDER = (110, 110, 110)


def main() -> None:
    img = Image.open(PATH).convert("RGB")
    W, H = img.size
    draw = ImageDraw.Draw(img)

    x0 = MARGIN_L
    y0 = MARGIN_T
    x1 = W - MARGIN_R
    y1 = H - MARGIN_B
    pw, ph = x1 - x0, y1 - y0

    # Horizontal — every 10°C, major every 50°C (no labels)
    for t in range(0, TEMP_MAX + 1, 10):
        y = y1 - round(t / TEMP_MAX * ph)
        major = t % 50 == 0
        draw.line([(x0, y), (x1, y)], fill=C_GRID_MAJOR if major else C_GRID_MINOR, width=1)

    # Vertical — every 1 min, major every 5 min (no labels)
    mins = pw // PX_PER_MIN
    for m in range(0, mins + 1):
        x = x0 + m * PX_PER_MIN
        major = m % 5 == 0
        draw.line([(x, y0), (x, y1)], fill=C_GRID_MAJOR if major else C_GRID_MINOR, width=1)

    draw.rectangle([(x0, y0), (x1, y1)], outline=C_BORDER, width=1)

    img.save(PATH, format="JPEG", quality=95, subsampling=0)
    print(f"updated {PATH} ({W}x{H}) plot={pw}x{ph} ~{mins} min wide")


if __name__ == "__main__":
    main()
