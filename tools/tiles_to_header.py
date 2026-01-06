#!/usr/bin/env python3
"""
Convert tileset PNG to C header file.
Usage: python3 tiles_to_header.py <input.png> <tile|sprite> <rows>

  sprite - Creates spriteset.h, 16x16, 8-bit per pixel (256 bytes each)
  tile   - Creates tileset.h, 8x8, 4-bit packed (32 bytes each)
  rows   - Number of rows to process
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

# Transparent color for sprites (0xE3 is standard ZX Next transparent)
SPRITE_TRANSPARENT = 0xE3


def get_palette_index(rgb, is_sprite=False):
    """Get palette index for RGB color, handling transparency for sprites"""
    # Check for transparent (alpha or black for sprites)
    idx = ZX_PALETTE.get(rgb, 0)
    return idx


def convert_sprites(input_path, output_path, num_rows):
    """Convert to 16x16 sprites, 8-bit per pixel (256 bytes each)"""
    img = Image.open(input_path)
    img = img.convert('RGBA')  # Keep alpha for transparency detection

    tile_size = 16
    tiles_x = img.width // tile_size
    tiles_y = min(num_rows, img.height // tile_size)
    bytes_per_sprite = 256  # 16x16 = 256 pixels, 1 byte each

    output = f'''#ifndef SPRITESET_H
#define SPRITESET_H

#include <stdint.h>

// Sprite definitions (16x16, 8-bit per pixel)
// 256 bytes per sprite, 0xE3 = transparent
// Palette: ZX Spectrum colors 0-15

#define SPRITE_SIZE {bytes_per_sprite}
#define SPRITESET_WIDTH {tiles_x}
#define SPRITESET_HEIGHT {tiles_y}
#define SPRITE_TRANSPARENT 0xE3

'''

    sprite_arrays = []
    for ty in range(tiles_y):
        for tx in range(tiles_x):
            # Format sprite name with column letter and row number
            col_letter = chr(ord('A') + tx)
            name = f'sprite_{col_letter}{ty}'

            output += f'// Sprite at ({col_letter},{ty})\n'
            output += f'static const uint8_t {name}[{bytes_per_sprite}] = {{\n'

            for py in range(tile_size):
                row_pixels = []
                for px in range(tile_size):
                    rgba = img.getpixel((tx * tile_size + px, ty * tile_size + py))
                    rgb = rgba[:3]
                    alpha = rgba[3] if len(rgba) > 3 else 255

                    # Transparent if alpha < 128 or black with low alpha
                    if alpha < 128:
                        pixel_val = SPRITE_TRANSPARENT
                    else:
                        pixel_val = ZX_PALETTE.get(rgb, 0)
                        # If black and we want it transparent, use 0xE3
                        # (keeping black as 0 for actual black pixels)

                    row_pixels.append(f'0x{pixel_val:02X}')

                comma = ',' if py < tile_size - 1 else ''
                output += f'    {",".join(row_pixels)}{comma}\n'

            output += '};\n\n'
            sprite_arrays.append(name)

    # Generate lookup array
    output += f'// Array of all sprites for indexed access\n'
    output += f'static const uint8_t * const spriteset[{len(sprite_arrays)}] = {{\n'
    for i, name in enumerate(sprite_arrays):
        comma = ',' if i < len(sprite_arrays) - 1 else ''
        if i % 4 == 0:
            output += '    '
        output += f'{name}{comma} '
        if (i + 1) % 4 == 0:
            output += '\n'
    if len(sprite_arrays) % 4 != 0:
        output += '\n'
    output += '};\n\n'

    output += '''// Get sprite by grid position (col=A-P as 0-15, row=0-15)
#define SPRITESET_AT(col, row) spriteset[(row) * SPRITESET_WIDTH + (col)]

#endif
'''

    with open(output_path, 'w') as f:
        f.write(output)

    print(f'Generated {output_path}')
    print(f'  Mode: sprite (16x16, 8-bit per pixel)')
    print(f'  Sprites: {tiles_x}x{tiles_y} = {len(sprite_arrays)}')
    print(f'  Size: {len(sprite_arrays) * bytes_per_sprite} bytes')


def convert_tiles(input_path, output_path, num_rows):
    """Convert to 8x8 tiles, 4-bit packed (32 bytes each)"""
    img = Image.open(input_path)
    img = img.convert('RGB')

    tile_size = 8
    tiles_x = img.width // tile_size
    tiles_y = min(num_rows, img.height // tile_size)
    bytes_per_tile = 32  # 8x8 = 64 pixels, 2 pixels per byte = 32 bytes

    output = f'''#ifndef TILESET_H
#define TILESET_H

#include <stdint.h>

// Tile definitions (8x8, 4-bit packed)
// 32 bytes per tile, 2 pixels per byte (high nibble = left pixel)
// Palette: ZX Spectrum colors 0-15

#define TILE_SIZE {bytes_per_tile}
#define TILESET_WIDTH {tiles_x}
#define TILESET_HEIGHT {tiles_y}

'''

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

            # Format tile name with column letter and row number
            col_letter = chr(ord('A') + tx)
            name = f'tile_{col_letter}{ty}'
            output += f'// Tile at ({col_letter},{ty})\n'
            output += f'static const uint8_t {name}[{bytes_per_tile}] = {{\n'
            for i, row in enumerate(tile_data):
                comma = ',' if i < tile_size - 1 else ''
                output += f'    {row}{comma}\n'
            output += '};\n\n'
            tile_arrays.append(name)

    # Generate lookup array
    output += f'// Array of all tiles for indexed access\n'
    output += f'static const uint8_t * const tileset[{len(tile_arrays)}] = {{\n'
    for i, name in enumerate(tile_arrays):
        comma = ',' if i < len(tile_arrays) - 1 else ''
        if i % 4 == 0:
            output += '    '
        output += f'{name}{comma} '
        if (i + 1) % 4 == 0:
            output += '\n'
    if len(tile_arrays) % 4 != 0:
        output += '\n'
    output += '};\n\n'

    output += '''// Get tile by grid position (col=A-P as 0-15, row=0-15)
#define TILESET_AT(col, row) tileset[(row) * TILESET_WIDTH + (col)]

#endif
'''

    with open(output_path, 'w') as f:
        f.write(output)

    print(f'Generated {output_path}')
    print(f'  Mode: tile (8x8, 4-bit packed)')
    print(f'  Tiles: {tiles_x}x{tiles_y} = {len(tile_arrays)}')
    print(f'  Size: {len(tile_arrays) * bytes_per_tile} bytes')


if __name__ == '__main__':
    if len(sys.argv) < 4:
        print(__doc__)
        sys.exit(1)

    input_path = sys.argv[1]
    mode = sys.argv[2].lower()
    num_rows = int(sys.argv[3])

    if mode not in ('tile', 'sprite'):
        print(f"Error: mode must be 'tile' or 'sprite', got '{mode}'")
        sys.exit(1)

    script_dir = os.path.dirname(os.path.abspath(__file__))
    src_dir = os.path.join(os.path.dirname(script_dir), 'src')

    if mode == 'sprite':
        output_path = os.path.join(src_dir, 'spriteset.h')
        convert_sprites(input_path, output_path, num_rows)
    else:
        output_path = os.path.join(src_dir, 'tileset.h')
        convert_tiles(input_path, output_path, num_rows)
