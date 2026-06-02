#!/usr/bin/env python3
"""Convert images to 1-bit monochrome C header arrays for Moui.

Single file:
    python3 img_to_c.py input.png output.h [options]

Batch mode:
    python3 img_to_c.py --batch input_dir/ --outdir output_dir/ [options]
    Converts all PNG/JPG/BMP files, outputs .h files + assets_manifest.h

Options:
    --width W      Target width (default: 168)
    --height H     Target height (default: 384)
    --invert       Invert black/white
    --dither       FSD (Floyd-Steinberg), ATKINSON, or NONE (default: FSD)
    --compress     none or rle (default: none)
    --scale        FIT (letterbox) or CROP (center-crop)
    --preview      Output a _preview.png alongside each .h
    --name NAME    C variable name prefix (single-file mode only)
"""

import argparse
import os
import sys
from PIL import Image


def floyd_steinberg(img):
    pixels = img.load()
    w, h = img.size
    errors = [[0.0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            old = pixels[x, y] + errors[y][x]
            new = 255.0 if old >= 128.0 else 0.0
            pixels[x, y] = int(new)
            err = old - new
            if x + 1 < w:
                errors[y][x + 1] += err * 7.0 / 16.0
            if y + 1 < h:
                if x > 0:
                    errors[y + 1][x - 1] += err * 3.0 / 16.0
                errors[y + 1][x] += err * 5.0 / 16.0
                if x + 1 < w:
                    errors[y + 1][x + 1] += err * 1.0 / 16.0
    return img


def atkinson(img):
    pixels = img.load()
    w, h = img.size
    errors = [[0.0] * w for _ in range(h)]
    for y in range(h):
        for x in range(w):
            old = pixels[x, y] + errors[y][x]
            new = 255.0 if old >= 128.0 else 0.0
            pixels[x, y] = int(new)
            err = old - new
            for dx, dy in [(1, 0), (2, 0), (-1, 1), (0, 1), (1, 1), (0, 2)]:
                nx, ny = x + dx, y + dy
                if 0 <= nx < w and 0 <= ny < h:
                    errors[ny][nx] += err / 8.0
    return img


def threshold(img):
    pixels = img.load()
    w, h = img.size
    for y in range(h):
        for x in range(w):
            pixels[x, y] = 255 if pixels[x, y] >= 128 else 0
    return img


def pack_hmsb(img):
    w, h = img.size
    pixels = img.load()
    bytes_per_row = (w + 7) // 8
    data = bytearray()
    for y in range(h):
        for byte_idx in range(bytes_per_row):
            val = 0
            for bit in range(8):
                x = byte_idx * 8 + bit
                if x < w and pixels[x, y] == 0:
                    val |= (0x80 >> bit)
            data.append(val)
    return bytes(data)


def rle_encode(data):
    out = bytearray()
    i = 0
    n = len(data)
    while i < n:
        b = data[i]
        run = 1
        while i + run < n and data[i + run] == b and run < 255:
            run += 1
        if run >= 3 or b == 0x80:
            out.append(0x80)
            out.append(run)
            out.append(b)
            i += run
        else:
            out.append(b)
            i += 1
    return bytes(out)


def process_image(input_path, output_path, args, name=None):
    img = Image.open(input_path).convert('RGB')

    if args.scale == 'CROP':
        src_ratio = img.width / img.height
        dst_ratio = args.width / args.height
        if src_ratio > dst_ratio:
            new_h = img.height
            new_w = int(img.height * dst_ratio)
            left = (img.width - new_w) // 2
            img = img.crop((left, 0, left + new_w, new_h))
        else:
            new_w = img.width
            new_h = int(img.width / dst_ratio)
            top = (img.height - new_h) // 2
            img = img.crop((0, top, new_w, top + new_h))
    img = img.resize((args.width, args.height), Image.LANCZOS)
    img = img.convert('L')

    if args.dither == 'FSD':
        img = floyd_steinberg(img)
    elif args.dither == 'ATKINSON':
        img = atkinson(img)
    else:
        img = threshold(img)

    if args.invert:
        pixels = img.load()
        for y in range(img.height):
            for x in range(img.width):
                pixels[x, y] = 255 - pixels[x, y]

    if args.preview:
        preview_path = os.path.splitext(output_path)[0] + '_preview.png'
        img.save(preview_path)

    raw_data = pack_hmsb(img)
    compressed = args.compress == 'rle'
    data = rle_encode(raw_data) if compressed else raw_data

    if not name:
        name = os.path.splitext(os.path.basename(input_path))[0]
        name = ''.join(c if c.isalnum() else '_' for c in name)
        name = 'img_' + name

    guard = name.upper() + '_IMG_H'
    lines = [
        f'#ifndef {guard}',
        f'#define {guard}',
        '',
        '#include <stdint.h>',
        '',
        f'#define {name.upper()}_W {args.width}',
        f'#define {name.upper()}_H {args.height}',
        f'#define {name.upper()}_COMPRESSED {1 if compressed else 0}',
        f'#define {name.upper()}_RAW_SIZE {len(raw_data)}',
        f'#define {name.upper()}_DATA_SIZE {len(data)}',
        '',
        f'static const uint8_t {name}[] = ' + '{',
    ]
    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        line = '    ' + ', '.join(f'0x{b:02X}' for b in chunk) + ','
        lines.append(line)
    lines.append('};')
    lines.append('')
    lines.append('#endif')

    with open(output_path, 'w') as f:
        f.write('\n'.join(lines) + '\n')

    return {
        'name': name,
        'path': output_path,
        'width': args.width,
        'height': args.height,
        'raw_size': len(raw_data),
        'data_size': len(data),
        'compressed': compressed,
    }


def write_manifest(entries, out_path):
    lines = [
        '#ifndef ASSETS_MANIFEST_H',
        '#define ASSETS_MANIFEST_H',
        '',
        '/* Auto-generated by img_to_c.py --batch */',
        '',
    ]
    for e in entries:
        lines.append(f'#include "{os.path.basename(e["path"])}"')
    lines.append('')
    lines.append(f'#define MOUI_ASSET_COUNT {len(entries)}')
    lines.append('')
    for i, e in enumerate(entries):
        lines.append(f'#define MOUI_ASSET_{e["name"].upper()} {i}')
    lines.append('')
    lines.append('#endif')

    with open(out_path, 'w') as f:
        f.write('\n'.join(lines) + '\n')


def main():
    parser = argparse.ArgumentParser(description='Convert images to 1-bit C headers for Moui')
    parser.add_argument('input', nargs='?', help='Input image (single-file mode)')
    parser.add_argument('output', nargs='?', help='Output .h path')
    parser.add_argument('--batch', help='Batch mode: input directory')
    parser.add_argument('--outdir', help='Batch mode: output directory')
    parser.add_argument('--width', type=int, default=168, help='Target width (default: 168)')
    parser.add_argument('--height', type=int, default=384, help='Target height (default: 384)')
    parser.add_argument('--invert', action='store_true', help='Invert black and white')
    parser.add_argument('--dither', choices=['FSD', 'ATKINSON', 'NONE'], default='FSD',
                        help='Dithering algorithm (default: FSD)')
    parser.add_argument('--name', help='C variable name prefix (single-file only)')
    parser.add_argument('--scale', choices=['FIT', 'CROP'], default='FIT',
                        help='Resize mode: FIT or CROP')
    parser.add_argument('--compress', choices=['none', 'rle'], default='none',
                        help='Compression: none or rle')
    parser.add_argument('--preview', action='store_true',
                        help='Output PNG preview alongside each .h')
    args = parser.parse_args()

    if args.batch:
        if not os.path.isdir(args.batch):
            print(f"Error: {args.batch} is not a directory", file=sys.stderr)
            sys.exit(1)

        outdir = args.outdir or args.batch
        os.makedirs(outdir, exist_ok=True)

        exts = ('.png', '.jpg', '.jpeg', '.bmp', '.gif', '.tiff')
        files = sorted(f for f in os.listdir(args.batch)
                       if os.path.splitext(f)[1].lower() in exts)

        if not files:
            print(f"No image files found in {args.batch}")
            sys.exit(1)

        entries = []
        for fname in files:
            input_path = os.path.join(args.batch, fname)
            stem = os.path.splitext(fname)[0]
            name = 'img_' + ''.join(c if c.isalnum() else '_' for c in stem)
            out_path = os.path.join(outdir, f'{name}.h')

            info = process_image(input_path, out_path, args, name=name)
            entries.append(info)

            ratio_str = ''
            if info['compressed']:
                ratio = 100.0 * info['data_size'] / info['raw_size']
                ratio_str = f' (RLE {ratio:.0f}%)'
            print(f"  {fname} → {name}.h  {info['data_size']}B{ratio_str}")

        manifest_path = os.path.join(outdir, 'assets_manifest.h')
        write_manifest(entries, manifest_path)
        print(f"\nManifest: {manifest_path} ({len(entries)} assets)")

    elif args.input:
        if not os.path.exists(args.input):
            print(f"Error: {args.input} not found", file=sys.stderr)
            sys.exit(1)

        name = args.name
        if not name:
            name = os.path.splitext(os.path.basename(args.input))[0]
            name = ''.join(c if c.isalnum() else '_' for c in name)
            name = 'img_' + name

        out_path = args.output
        if not out_path:
            out_dir = os.path.dirname(args.input) or '.'
            out_path = os.path.join(out_dir, f'{name}.h')

        info = process_image(args.input, out_path, args, name=name)

        print(f"Output: {out_path}")
        print(f"  Dimensions: {info['width']} x {info['height']}")
        print(f"  Dither: {args.dither}")
        print(f"  Raw data: {info['raw_size']} bytes")
        if info['compressed']:
            ratio = 100.0 * info['data_size'] / info['raw_size']
            print(f"  RLE data: {info['data_size']} bytes ({ratio:.1f}%)")
        print(f"  C identifier: {name}")
    else:
        parser.print_help()
        sys.exit(1)


if __name__ == '__main__':
    main()
