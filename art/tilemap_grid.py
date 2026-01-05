#!/usr/bin/env python3
"""
Generate a tilemap grid image with 1px red separators and hex labels.
Usage: python3 tilemap_grid.py [input.png] [output.png]
"""

from PIL import Image, ImageDraw, ImageFont
import sys
import os

def create_tilemap_grid(input_path, output_path, tile_size=8):
    # Load the tilemap
    src = Image.open(input_path)
    src = src.convert('RGBA')

    src_width, src_height = src.size
    tiles_x = src_width // tile_size
    tiles_y = src_height // tile_size

    # Layout settings
    separator = 1
    label_margin = 10  # Space for labels

    # Calculate dimensions
    grid_width = tiles_x * tile_size + (tiles_x + 1) * separator
    grid_height = tiles_y * tile_size + (tiles_y + 1) * separator
    new_width = label_margin + grid_width
    new_height = label_margin + grid_height

    # Create new image with black background
    dst = Image.new('RGBA', (new_width, new_height), (0, 0, 0, 255))
    draw = ImageDraw.Draw(dst)

    # Use default font
    try:
        font = ImageFont.truetype("/System/Library/Fonts/Monaco.dfont", 8)
    except:
        font = ImageFont.load_default()

    # Labels for columns/rows (hex-style: 0-9, A-F, then G, H, etc.)
    def get_label(i):
        if i < 16:
            return format(i, 'X')
        return chr(ord('G') + i - 16)

    col_labels = [get_label(i) for i in range(tiles_x)]
    row_labels = [get_label(i) for i in range(tiles_y)]

    # Draw column labels (top) - white text
    for tx in range(tiles_x):
        x = label_margin + separator + tx * (tile_size + separator) + tile_size // 2
        draw.text((x, 1), col_labels[tx], fill=(255, 255, 255, 255), font=font, anchor="mt")

    # Draw row labels (left) - white text
    for ty in range(tiles_y):
        y = label_margin + separator + ty * (tile_size + separator) + tile_size // 2
        draw.text((4, y), row_labels[ty], fill=(255, 255, 255, 255), font=font, anchor="mm")

    # Fill grid area with red (separator color)
    for x in range(label_margin, new_width):
        for y in range(label_margin, new_height):
            dst.putpixel((x, y), (255, 0, 0, 255))

    # Copy each tile to its new position
    for ty in range(tiles_y):
        for tx in range(tiles_x):
            # Source position
            src_x = tx * tile_size
            src_y = ty * tile_size

            # Destination position (with margin and separators)
            dst_x = label_margin + separator + tx * (tile_size + separator)
            dst_y = label_margin + separator + ty * (tile_size + separator)

            # Extract and paste tile
            tile = src.crop((src_x, src_y, src_x + tile_size, src_y + tile_size))
            dst.paste(tile, (dst_x, dst_y))

    # Save the result
    dst.save(output_path)
    print(f'Saved to: {output_path}')
    print(f'Original: {src_width}x{src_height} ({tiles_x}x{tiles_y} tiles)')
    print(f'New size: {new_width}x{new_height}')


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))

    input_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(script_dir, 'tilemap.png')
    output_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(script_dir, 'tilemap_grid.png')

    create_tilemap_grid(input_path, output_path)
