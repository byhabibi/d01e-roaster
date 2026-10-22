#!/usr/bin/env python3
"""Draw roast grid on nextion_assets/169.jpg — proportional margins for axis numbers."""
from pathlib import Path

from PIL import Image, ImageDraw

PATH = Path(__file__).resolve().parents[2] / "nextion_assets" / "169.jpg"
PX_PER_MIN = 8
TEMP_MAX = 250

# White plot area — after vertical "ROASTING TEMPERATURE" (~x253-260)
PLOT_LEFT = 262
PLOT_RIGHT = 798

# Y numbers (t7–t12) sit here; grid starts immediately after
Y_NUM_ZONE = 28
X_NUM_ZONE = 26
MARGIN_T = 8
MARGIN_R = 5

C_WHITE = (255, 255, 255)
C_GRID_MINOR = (220, 220, 220)
C_GRID_MAJOR = (175, 175, 175)
C_BORDER = (110, 110, 110)


def clear_plot(img: Image.Image) -> None:
    """Erase previous grid — fill only the white plot rectangle."""
    draw = ImageDraw.Draw(img)
    w, h = img.size
    draw.rectangle([(PLOT_LEFT, 0), (w - 1, h - 1)], fill=C_WHITE)


def main() -> None:
    img = Image.open(PATH).convert("RGB")
    w, h = img.size
    clear_plot(img)
    draw = ImageDraw.Draw(img)

    x0 = PLOT_LEFT + Y_NUM_ZONE
    y0 = MARGIN_T
    x1 = PLOT_RIGHT - MARGIN_R
    y1 = h - X_NUM_ZONE
    pw, ph = x1 - x0, y1 - y0

    for t in range(0, TEMP_MAX + 1, 10):
        y = y1 - round(t / TEMP_MAX * ph)
        major = t % 50 == 0
        draw.line([(x0, y), (x1, y)], fill=C_GRID_MAJOR if major else C_GRID_MINOR, width=1)

    mins = pw // PX_PER_MIN
    for m in range(0, mins + 1):
        x = x0 + m * PX_PER_MIN
        major = m % 5 == 0
        draw.line([(x, y0), (x, y1)], fill=C_GRID_MAJOR if major else C_GRID_MINOR, width=1)

    draw.rectangle([(x0, y0), (x1, y1)], outline=C_BORDER, width=1)

    img.save(PATH, format="JPEG", quality=95, subsampling=0)
    print(f"updated {PATH} ({w}x{h})")
    print(f"  Y-num zone: x{PLOT_LEFT}..{x0 - 1} ({Y_NUM_ZONE}px)")
    print(f"  grid plot:  ({x0},{y0})-({x1},{y1}) = {pw}x{ph}px ~{mins} min")
    print(f"  X-num zone: y{y1 + 1}..{h - 1} ({X_NUM_ZONE}px)")


if __name__ == "__main__":
    main()
