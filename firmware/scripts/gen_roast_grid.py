#!/usr/bin/env python3
"""Roast grid — full width, Y labels only."""
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

ASSETS = Path(__file__).resolve().parents[2] / "nextion_assets"
PX_PER_MIN = 8
MARGIN_L, MARGIN_R, MARGIN_T, MARGIN_B = 40, 2, 12, 26
H = 330
TEMP_MAX = 250

# ~76 menit @ 8px + margin — muat area putih page2 (x69 .. x~720)
WIN_SPAN_MIN = 76
W = MARGIN_L + WIN_SPAN_MIN * PX_PER_MIN + MARGIN_R  # 654

C_BG = (255, 255, 255)
C_BORDER = (100, 100, 100)
C_GRID_MINOR = (228, 228, 228)
C_GRID_MAJOR = (185, 185, 185)
C_LABEL = (30, 30, 30)


def y_font():
    for path, size in [
        ("C:/Windows/Fonts/consolab.ttf", 11),
        ("C:/Windows/Fonts/consola.ttf", 11),
    ]:
        if Path(path).exists():
            return ImageFont.truetype(path, size)
    return ImageFont.load_default()


def main() -> None:
    img = Image.new("RGB", (W, H), C_BG)
    draw = ImageDraw.Draw(img)
    font = y_font()

    x0, y0 = MARGIN_L, MARGIN_T
    x1, y1 = W - MARGIN_R, H - MARGIN_B
    ph = y1 - y0

    for t in range(0, TEMP_MAX + 1, 10):
        y = y1 - round(t / TEMP_MAX * ph)
        major = t % 50 == 0
        draw.line([(x0, y), (x1, y)], fill=C_GRID_MAJOR if major else C_GRID_MINOR, width=1)
        if major:
            bbox = draw.textbbox((0, 0), str(t), font=font)
            tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
            draw.text((x0 - tw - 5, y - th // 2), str(t), fill=C_LABEL, font=font)

    for m in range(0, WIN_SPAN_MIN + 1):
        x = x0 + m * PX_PER_MIN
        major = m % 5 == 0
        draw.line([(x, y0), (x, y1)], fill=C_GRID_MAJOR if major else C_GRID_MINOR, width=1)

    draw.rectangle([(x0, y0), (x1, y1)], outline=C_BORDER, width=1)

    ASSETS.mkdir(parents=True, exist_ok=True)
    for name in ("roast_grid_654x330.png", "roast_grid_660x330.png", "roast_grid_573x330.png"):
        out = ASSETS / name
        img.save(out, format="PNG", compress_level=0, optimize=False)
        print("saved", out, f"{W}x{H}")


if __name__ == "__main__":
    main()
