#!/usr/bin/env python3
"""
Convert PNG image to z88dk assembly file with banked data.

Usage:
    python3 png_to_asm.py <input.png> <start_page> [output.asm]

Example:
    python3 png_to_asm.py border.png 40 src/border_data.asm

Requirements:
- Image height must be exactly 192 pixels
- Image width can be 1-256 pixels
- Image is converted to RGB332 format
"""

import sys
import os

try:
    from PIL import Image
except ImportError:
    print("Error: PIL/Pillow is required. Install with: pip3 install Pillow")
    sys.exit(1)


def rgb_to_rgb332(r, g, b):
    """Convert 24-bit RGB to 8-bit RGB332."""
    r3 = (r >> 5) & 0x07
    g3 = (g >> 5) & 0x07
    b2 = (b >> 6) & 0x03
    return (r3 << 5) | (g3 << 2) | b2


def png_to_rgb332(png_path):
    """Load PNG and convert to RGB332 byte array."""
    img = Image.open(png_path).convert('RGB')
    width, height = img.size

    # Validate dimensions
    if height != 192:
        print(f"Error: Image height must be 192 pixels, got {height}")
        sys.exit(1)

    if width < 1 or width > 256:
        print(f"Error: Image width must be 1-256 pixels, got {width}")
        sys.exit(1)

    print(f"Image: {width}x{height} pixels")

    # Convert to RGB332
    pixels = []
    for y in range(height):
        for x in range(width):
            r, g, b = img.getpixel((x, y))
            pixels.append(rgb_to_rgb332(r, g, b))

    return pixels, width, height


def generate_asm(pixels, width, height, start_page, label_base):
    """Generate assembly file content with PAGE sections."""
    total_bytes = len(pixels)
    page_size = 8192
    num_pages = (total_bytes + page_size - 1) // page_size

    print(f"Total bytes: {total_bytes}")
    print(f"Pages needed: {num_pages} (pages {start_page}-{start_page + num_pages - 1})")

    lines = []
    lines.append(f"; Generated from PNG: {width}x{height} pixels, {total_bytes} bytes")
    lines.append(f"; Pages: {start_page}-{start_page + num_pages - 1}")
    lines.append("")

    offset = 0
    for page_idx in range(num_pages):
        page_num = start_page + page_idx
        page_label = f"_{label_base}_page{page_num}"

        # Calculate bytes for this page
        page_start = page_idx * page_size
        page_end = min(page_start + page_size, total_bytes)
        page_bytes = pixels[page_start:page_end]

        lines.append(f"SECTION PAGE_{page_num}")
        lines.append(f"PUBLIC {page_label}")
        lines.append(f"{page_label}:")

        # Write defb statements, 16 bytes per line
        for i in range(0, len(page_bytes), 16):
            chunk = page_bytes[i:i+16]
            hex_bytes = ', '.join(f'0x{b:02x}' for b in chunk)
            lines.append(f"    defb {hex_bytes}")

        lines.append("")

    return '\n'.join(lines), num_pages


def main():
    if len(sys.argv) < 3:
        print("Usage: python3 png_to_asm.py <input.png> <start_page> [output.asm]")
        print("")
        print("Example:")
        print("  python3 png_to_asm.py border.png 40 src/border_data.asm")
        sys.exit(1)

    png_path = sys.argv[1]
    start_page = int(sys.argv[2])

    # Derive output path and label from input filename
    base_name = os.path.splitext(os.path.basename(png_path))[0]

    if len(sys.argv) >= 4:
        output_path = sys.argv[3]
    else:
        output_path = f"src/{base_name}_data.asm"

    if not os.path.exists(png_path):
        print(f"Error: File not found: {png_path}")
        sys.exit(1)

    print(f"Converting: {png_path}")
    print(f"Start page: {start_page}")
    print(f"Output: {output_path}")
    print("")

    # Convert PNG to RGB332
    pixels, width, height = png_to_rgb332(png_path)

    # Generate assembly
    asm_content, num_pages = generate_asm(pixels, width, height, start_page, base_name)

    # Write output
    with open(output_path, 'w') as f:
        f.write(asm_content)

    print(f"\nGenerated: {output_path}")
    print(f"Add to Makefile: ASMS = {output_path}")
    print("")
    print("Add to C code:")
    for i in range(num_pages):
        page_num = start_page + i
        print(f"  extern uint8_t {base_name}_page{page_num};")


if __name__ == '__main__':
    main()
