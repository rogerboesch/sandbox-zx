#!/usr/bin/env python3
"""
Convert tilemap.png to tiles.h header file.
Usage: python3 tiles_to_header.py [input.png] [output.h]
"""

from PIL import Image
import sys
import os

# ZX Spectrum classic palette (RGB to palette index)
ZX_PALETTE = {
    (0, 0, 0): 0,         # Black
    (0, 0, 192): 1,       # Blue
    (192, 0, 0): 2,       # Red
    (192, 0, 192): 3,     # Magenta
    (0, 192, 0): 4,       # Green
    (0, 192, 192): 5,     # Cyan
    (192, 192, 0): 6,     # Yellow
    (192, 192, 192): 7,   # White
    # Bright versions (8-15)
    (0, 0, 255): 9,       # Bright Blue
    (255, 0, 0): 10,      # Bright Red
    (255, 0, 255): 11,    # Bright Magenta
    (0, 255, 0): 12,      # Bright Green
    (0, 255, 255): 13,    # Bright Cyan
    (255, 255, 0): 14,    # Bright Yellow
    (255, 255, 255): 15,  # Bright White
}


def convert_tilemap(input_path, output_path, tile_size=8):
    img = Image.open(input_path)
    img = img.convert('RGB')

    tiles_x = img.width // tile_size
    tiles_y = img.height // tile_size

    output = '''#ifndef TILES_H
#define TILES_H

#include <stdint.h>

// Tile definitions extracted from tilemap.png
// 4-bit tiles: 8x8 pixels, 32 bytes each (2 pixels per byte)
// Palette: ZX Spectrum colors 0-15

#define TILE_SIZE 32
#define TILEMAP_WIDTH {tiles_x}
#define TILEMAP_HEIGHT {tiles_y}

// Tile indices for road (update these to match tilemap positions)
// Format: row * 16 + col (e.g., tile at 3,2 = 2*16+3 = 35)
#define TILE_TRANS       0x00  // (0,0) - transparent/empty
#define TILE_ROAD        0x01  // (1,0) - black road solid
#define TILE_DASH        0x02  // (2,0) - road with center dash
#define TILE_BORDER_L    0x03  // (3,0) - left cyan border
#define TILE_BORDER_R    0x04  // (4,0) - right cyan border
#define TILE_BORDER_L_D  0x05  // (5,0) - left border with dash
#define TILE_BORDER_R_D  0x06  // (6,0) - right border with dash

'''.format(tiles_x=tiles_x, tiles_y=tiles_y)

    # Generate all tiles
    tile_arrays = []
    for ty in range(tiles_y):
        for tx in range(tiles_x):
            tile_data = []

            for py in range(tile_size):
                row_bytes = []
                for px in range(0, tile_size, 2):  # 2 pixels per byte
                    rgb1 = img.getpixel((tx * tile_size + px, ty * tile_size + py))
                    rgb2 = img.getpixel((tx * tile_size + px + 1, ty * tile_size + py))
                    pal1 = ZX_PALETTE.get(rgb1, 0)
                    pal2 = ZX_PALETTE.get(rgb2, 0)
                    byte_val = (pal1 << 4) | pal2
                    row_bytes.append(f'0x{byte_val:02X}')
                tile_data.append(','.join(row_bytes))

            # Format tile name as hex position
            name = f'tile_{tx:X}{ty:X}'
            output += f'// Tile at ({tx:X},{ty:X})\n'
            output += f'static const uint8_t {name}[32] = {{\n'
            for i, row in enumerate(tile_data):
                comma = ',' if i < 7 else ''
                output += f'    {row}{comma}\n'
            output += '};\n\n'
            tile_arrays.append(name)

    # Generate the tile lookup array
    output += '// Array of all tiles for indexed access\n'
    output += f'static const uint8_t * const tilemap_tiles[{len(tile_arrays)}] = {{\n'
    for i, name in enumerate(tile_arrays):
        comma = ',' if i < len(tile_arrays) - 1 else ''
        if i % 4 == 0:
            output += '    '
        output += f'{name}{comma} '
        if (i + 1) % 4 == 0:
            output += '\n'
    output += '\n};\n\n'

    # Helper macro
    output += '''// Get tile by grid position
#define TILE_AT(col, row) tilemap_tiles[(row) * TILEMAP_WIDTH + (col)]

#endif
'''

    with open(output_path, 'w') as f:
        f.write(output)

    print(f'Generated {output_path}')
    print(f'  Tiles: {tiles_x}x{tiles_y} = {len(tile_arrays)}')
    print(f'  Size: {len(tile_arrays) * 32} bytes tile data')


if __name__ == '__main__':
    script_dir = os.path.dirname(os.path.abspath(__file__))
    src_dir = os.path.join(os.path.dirname(script_dir), 'src')

    input_path = sys.argv[1] if len(sys.argv) > 1 else os.path.join(script_dir, 'tilemap.png')
    output_path = sys.argv[2] if len(sys.argv) > 2 else os.path.join(src_dir, 'tiles.h')

    convert_tilemap(input_path, output_path)
