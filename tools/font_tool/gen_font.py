#!/usr/bin/env python3
"""Generate moui_font_t C source from BDF or TTF font files.

Usage:
    python gen_font.py --font WenQuanYiMicroHei.ttf --size 16 \
        --charset charset.txt --output font_cn_wqy16.c --name moui_font_cn_wqy16

    python gen_font.py --font unifont.bdf --size 16 \
        --charset charset.txt --output font_cn_unifont16.c --name moui_font_cn_unifont16

charset.txt: one character per line, or a continuous string of characters.
"""

import argparse
import os
import struct
import sys
from pathlib import Path


def load_charset(path):
    text = Path(path).read_text(encoding="utf-8")
    chars = set()
    for ch in text:
        if ch == '\n' or ch == '\r':
            continue
        chars.add(ch)
    return sorted(chars, key=lambda c: ord(c))


def render_ttf(font_path, size, chars, aa=False):
    try:
        from PIL import Image, ImageDraw, ImageFont
    except ImportError:
        print("pip install Pillow", file=sys.stderr)
        sys.exit(1)

    font = ImageFont.truetype(str(font_path), size)
    glyphs = []

    for ch in chars:
        bbox = font.getbbox(ch)
        if bbox is None:
            continue
        x0, y0, x1, y1 = bbox
        w = x1 - x0
        h = y1 - y0
        advance = font.getlength(ch)
        if advance <= 0:
            # Zero-ink whitespace (e.g. U+3000 full-width space) reports a
            # zero advance from PIL. Fall back to a sensible width.
            if ch.isspace():
                advance = size if (ord(ch) >= 0x3000 or ord(ch) == 0xA0) else size / 2
            else:
                continue

        # Space and other zero-ink glyphs (e.g. NBSP): no bitmap, keep advance
        if w <= 0 or h <= 0:
            glyphs.append({
                "codepoint": ord(ch),
                "width": 0,
                "height": 0,
                "x_offset": 0,
                "y_offset": 0,
                "x_advance": int(advance),
                "bitmap": b"",
                "aa": aa,
            })
            continue

        if aa:
            img = Image.new("L", (w, h), 0)
            draw = ImageDraw.Draw(img)
            draw.text((-x0, -y0), ch, fill=255, font=font)

            bytes_per_row = (w + 3) // 4  # 2 bits per pixel
            bitmap = bytearray()
            for row in range(h):
                for byte_idx in range(bytes_per_row):
                    val = 0
                    for px in range(4):
                        col = byte_idx * 4 + px
                        if col < w:
                            gray = img.getpixel((col, row))
                            if gray >= 192:
                                level = 3  # solid
                            elif gray >= 96:
                                level = 2  # 75%
                            elif gray >= 32:
                                level = 1  # 25%
                            else:
                                level = 0  # transparent
                            val |= (level << (6 - px * 2))
                    bitmap.append(val)
        else:
            img = Image.new("1", (w, h), 0)
            draw = ImageDraw.Draw(img)
            draw.text((-x0, -y0), ch, fill=1, font=font)

            bytes_per_row = (w + 7) // 8
            bitmap = bytearray()
            for row in range(h):
                for byte_idx in range(bytes_per_row):
                    val = 0
                    for bit in range(8):
                        col = byte_idx * 8 + bit
                        if col < w and img.getpixel((col, row)):
                            val |= 0x80 >> bit
                    bitmap.append(val)

        # Advance must cover the glyph's right edge (x_offset + width),
        # otherwise the next glyph overlaps by (x_offset + w - advance) px.
        x_advance = max(int(advance), x0 + w)
        glyphs.append({
            "codepoint": ord(ch),
            "width": w,
            "height": h,
            "x_offset": x0,
            "y_offset": y0,
            "x_advance": x_advance,
            "bitmap": bytes(bitmap),
            "aa": aa,
        })

    return glyphs


def render_bdf(font_path, chars):
    codepoint_set = {ord(c) for c in chars}
    glyphs = []

    with open(font_path, "r", encoding="utf-8", errors="replace") as f:
        lines = f.readlines()

    i = 0
    while i < len(lines):
        line = lines[i].strip()
        if line.startswith("STARTCHAR"):
            encoding = -1
            bbx = (0, 0, 0, 0)
            bitmap_data = []
            i += 1
            while i < len(lines):
                l = lines[i].strip()
                if l.startswith("ENCODING"):
                    encoding = int(l.split()[1])
                elif l.startswith("BBX"):
                    parts = l.split()
                    bbx = (int(parts[1]), int(parts[2]), int(parts[3]), int(parts[4]))
                elif l == "BITMAP":
                    i += 1
                    while i < len(lines) and lines[i].strip() != "ENDCHAR":
                        bitmap_data.append(lines[i].strip())
                        i += 1
                    break
                i += 1

            if encoding in codepoint_set and bitmap_data:
                w, h, xoff, yoff = bbx
                # BDF rows may be padded wider than ceil(w/8). Use the actual
                # on-disk row width as the stride so no pixels are dropped,
                # and widen the glyph accordingly so the reader's
                # (width+7)/8 stride matches.
                stride = 0
                rows = []
                for hex_row in bitmap_data:
                    rb = bytes.fromhex(hex_row)
                    rows.append(rb)
                    if len(rb) > stride:
                        stride = len(rb)
                if stride < (w + 7) // 8:
                    stride = (w + 7) // 8
                render_w = stride * 8

                bitmap = bytearray()
                for rb in rows:
                    for b in range(stride):
                        if b < len(rb):
                            bitmap.append(rb[b])
                        else:
                            bitmap.append(0)

                glyphs.append({
                    "codepoint": encoding,
                    "width": render_w,
                    "height": h,
                    "x_offset": xoff,
                    "y_offset": yoff,
                    "bitmap": bytes(bitmap),
                })
        i += 1

    return sorted(glyphs, key=lambda g: g["codepoint"])


def scan_source_charset(src_dirs):
    """Scan .c/.h files for non-ASCII characters to build a minimal charset."""
    chars = set()
    for src_dir in src_dirs:
        for root, dirs, files in os.walk(src_dir):
            for fname in files:
                if not fname.endswith(('.c', '.h')):
                    continue
                fpath = os.path.join(root, fname)
                try:
                    text = open(fpath, encoding='utf-8').read()
                except (UnicodeDecodeError, OSError):
                    continue
                for ch in text:
                    if ord(ch) > 0x7F:
                        chars.add(ch)
    return sorted(chars, key=lambda c: ord(c))


def print_stats(glyphs, all_bitmaps_size, name):
    """Print font generation statistics."""
    glyph_array_size = len(glyphs) * 12  # sizeof(moui_glyph_t) with uint32_t bitmap_offset
    codepoint_array_size = len(glyphs) * 4  # sizeof(uint32_t)
    total = all_bitmaps_size + glyph_array_size + codepoint_array_size
    print(f"\n--- Font Statistics: {name} ---")
    print(f"  Glyphs:     {len(glyphs)}")
    print(f"  Bitmaps:    {all_bitmaps_size:,} bytes")
    print(f"  Glyph tbl:  {glyph_array_size:,} bytes")
    print(f"  Codepoints: {codepoint_array_size:,} bytes")
    print(f"  Total Flash: {total:,} bytes ({total/1024:.1f} KB)")


def generate_c(glyphs, name, output_path, px_size, aa=False, header_comment=None):
    all_bitmaps = bytearray()
    glyph_entries = []

    for g in glyphs:
        offset = len(all_bitmaps)
        all_bitmaps.extend(g["bitmap"])
        glyph_entries.append({
            **g,
            "bitmap_offset": offset,
        })

    with open(output_path, "w", encoding="utf-8") as f:
        if header_comment:
            for line in header_comment.splitlines():
                f.write(f"/* {line} */\n")
            f.write("\n")
        f.write('#include "font/moui_font.h"\n\n')

        f.write(f"static const uint8_t {name}_bitmaps[] = {{\n")
        for i, b in enumerate(all_bitmaps):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{b:02X},")
            if i % 16 == 15:
                f.write("\n")
        f.write("\n};\n\n")

        f.write(f"static const uint32_t {name}_codepoints[] = {{\n")
        for i, g in enumerate(glyph_entries):
            if i % 8 == 0:
                f.write("    ")
            f.write(f"0x{g['codepoint']:04X},")
            if i % 8 == 7:
                f.write("\n")
        f.write("\n};\n\n")

        f.write(f"static const moui_glyph_t {name}_glyphs[] = {{\n")
        for g in glyph_entries:
            adv = g.get('x_advance', g['width'])
            f.write(f"    {{ {g['width']}, {g['height']}, "
                    f"{g['x_offset']}, {g['y_offset']}, {adv}, {g['bitmap_offset']} }},\n")
        f.write("};\n\n")

        line_h = px_size + 2
        f.write(f"const moui_font_t {name} = {{\n")
        f.write(f"    .px_size       = {px_size},\n")
        f.write(f"    .line_height   = {line_h},\n")
        f.write(f"    .ascii_width   = 0,\n")
        f.write(f"    .bpp           = {2 if aa else 1},\n")
        f.write(f"    .ascii_glyphs  = NULL,\n")
        f.write(f"    .cn_count      = {len(glyph_entries)},\n")
        f.write(f"    .cn_codepoints = {name}_codepoints,\n")
        f.write(f"    .cn_glyphs     = {name}_glyphs,\n")
        f.write(f"    .bitmap_data   = {name}_bitmaps,\n")
        f.write("};\n")

    print(f"Generated {output_path}: {len(glyph_entries)} glyphs, "
          f"{len(all_bitmaps)} bytes bitmap data")
    print_stats(glyph_entries, len(all_bitmaps), name)


def main():
    parser = argparse.ArgumentParser(description="Generate moui_font_t C source")
    parser.add_argument("--font", required=True, help="TTF or BDF font file")
    parser.add_argument("--size", type=int, default=16, help="Font size (TTF only)")
    parser.add_argument("--charset", help="File with characters to include")
    parser.add_argument("--scan-src", nargs='+', help="Scan source directories for CJK characters")
    parser.add_argument("--output", required=True, help="Output .c file")
    parser.add_argument("--name", required=True, help="C symbol name for the font")
    parser.add_argument("--aa", action='store_true', help="2-bit anti-aliased rendering")
    parser.add_argument("--comment", help="Header comment lines (font source/license)")
    args = parser.parse_args()

    if args.scan_src:
        chars = scan_source_charset(args.scan_src)
        print(f"Scanned {len(chars)} unique CJK characters from source")
        if args.charset:
            extra = load_charset(args.charset)
            chars = sorted(set(chars) | set(extra), key=lambda c: ord(c))
            print(f"  + {len(extra)} from charset file = {len(chars)} total")
    elif args.charset:
        chars = load_charset(args.charset)
        print(f"Loaded {len(chars)} characters from charset")
    else:
        print("Error: provide --charset or --scan-src", file=sys.stderr)
        sys.exit(1)

    font_path = Path(args.font)
    if font_path.suffix.lower() == ".bdf":
        glyphs = render_bdf(font_path, chars)
    else:
        glyphs = render_ttf(font_path, args.size, chars, aa=args.aa)

    if not glyphs:
        print("No glyphs rendered!", file=sys.stderr)
        sys.exit(1)

    generate_c(glyphs, args.name, args.output, args.size, aa=args.aa,
               header_comment=args.comment)


if __name__ == "__main__":
    main()
