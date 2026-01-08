#!/usr/bin/env python3
"""
Layer2 Image Editor for ZX Spectrum Next
- Open PNG images and convert to Layer2 format (8-bit RGB332)
- Supports any size up to 256x192 (preserves original dimensions)
- Paint with 16 ZX Spectrum colors
- Export as C header file for inclusion in game

Usage: python3 layer2_editor.py <input.png> [output_name]
  input.png: Source PNG image (max 256x192, keeps original size)
  output_name: Name for the image (default: layer2_image)

Controls:
  Click on canvas: Paint with selected color
  Opt+Click: Flood fill area
  Ctrl+Click: Replace all pixels of clicked color
  Right-click: Pick color from canvas
  Click on palette: Select color
  S: Save PNG
  E: Export to header file
  N: New blank canvas (resets to original size)
  Z: Undo
  ESC: Quit
"""

import pygame
import sys
import os
from collections import deque

# Layer2 max dimensions
LAYER2_MAX_WIDTH = 256
LAYER2_MAX_HEIGHT = 192

# ZX Spectrum 16-color palette (RGB values)
ZX_PALETTE = [
    (0, 0, 0),        # 0: Black
    (0, 0, 192),      # 1: Blue
    (192, 0, 0),      # 2: Red
    (192, 0, 192),    # 3: Magenta
    (0, 192, 0),      # 4: Green
    (0, 192, 192),    # 5: Cyan
    (192, 192, 0),    # 6: Yellow
    (192, 192, 192),  # 7: White
    (0, 0, 0),        # 8: Bright Black (same as black)
    (0, 0, 255),      # 9: Bright Blue
    (255, 0, 0),      # 10: Bright Red
    (255, 0, 255),    # 11: Bright Magenta
    (0, 255, 0),      # 12: Bright Green
    (0, 255, 255),    # 13: Bright Cyan
    (255, 255, 0),    # 14: Bright Yellow
    (255, 255, 255),  # 15: Bright White
]

# ZX Spectrum to RGB332 mapping (for Layer2 hardware)
ZX_TO_RGB332 = [
    0x00,  # 0: Black
    0x02,  # 1: Blue
    0xC0,  # 2: Red
    0xC2,  # 3: Magenta
    0x18,  # 4: Green
    0x1A,  # 5: Cyan
    0xD8,  # 6: Yellow
    0xDA,  # 7: White
    0x00,  # 8: Bright Black
    0x03,  # 9: Bright Blue
    0xE0,  # 10: Bright Red
    0xE3,  # 11: Bright Magenta
    0x1C,  # 12: Bright Green
    0x1F,  # 13: Bright Cyan
    0xFC,  # 14: Bright Yellow
    0xFF,  # 15: Bright White
]

# Color names for comments
ZX_COLOR_NAMES = [
    "Black", "Blue", "Red", "Magenta",
    "Green", "Cyan", "Yellow", "White",
    "Bright Black", "Bright Blue", "Bright Red", "Bright Magenta",
    "Bright Green", "Bright Cyan", "Bright Yellow", "Bright White"
]


class Layer2Editor:
    def __init__(self, input_path=None, output_name="layer2_image"):
        pygame.init()

        self.input_path = input_path
        self.output_name = output_name
        self.selected_color = 0
        self.undo_stack = []
        self.max_undo = 20

        # Default canvas size
        self.width = LAYER2_MAX_WIDTH
        self.height = LAYER2_MAX_HEIGHT

        # Load and convert input image if provided
        if input_path and os.path.exists(input_path):
            self.load_image(input_path)
        else:
            # Create default canvas
            self.canvas = pygame.Surface((self.width, self.height))
            self.canvas.fill(ZX_PALETTE[0])

        # Calculate scale factor based on canvas size
        # Aim for a reasonable display size (min 400px, max 768px on largest dimension)
        max_dim = max(self.width, self.height)
        if max_dim < 100:
            self.scale = 6
        elif max_dim < 150:
            self.scale = 4
        else:
            self.scale = 3

        # Layout
        self.padding = 20
        self.canvas_display_width = self.width * self.scale
        self.canvas_display_height = self.height * self.scale
        self.palette_height = 50
        self.info_height = 60

        # Minimum window width for palette (16 colors)
        min_palette_width = 16 * 36 + 15 * 4 + self.padding * 2

        self.window_width = max(self.canvas_display_width + self.padding * 2, min_palette_width)
        self.window_height = (self.canvas_display_height + self.palette_height +
                              self.info_height + self.padding * 4)

        self.screen = pygame.display.set_mode((self.window_width, self.window_height))
        title = f"Layer2 Editor - {os.path.basename(input_path) if input_path else 'New'} ({self.width}x{self.height})"
        pygame.display.set_caption(title)

        # Font
        self.font = pygame.font.Font(None, 24)
        self.small_font = pygame.font.Font(None, 18)

    def load_image(self, path):
        """Load and convert image to Layer2 format (preserving size)"""
        img = pygame.image.load(path)
        img_w, img_h = img.get_size()

        # Clamp to max Layer2 dimensions
        self.width = min(img_w, LAYER2_MAX_WIDTH)
        self.height = min(img_h, LAYER2_MAX_HEIGHT)

        # Create canvas at actual image size
        self.canvas = pygame.Surface((self.width, self.height))
        self.canvas.fill(ZX_PALETTE[0])

        # If image is larger than max, crop from top-left
        if img_w > LAYER2_MAX_WIDTH or img_h > LAYER2_MAX_HEIGHT:
            # Crop to fit
            crop_rect = pygame.Rect(0, 0, self.width, self.height)
            self.canvas.blit(img, (0, 0), crop_rect)
        else:
            self.canvas.blit(img, (0, 0))

        # Convert all pixels to nearest ZX Spectrum color
        self.quantize_to_zx_palette()

    def quantize_to_zx_palette(self):
        """Convert all pixels to nearest ZX Spectrum palette color"""
        for y in range(self.height):
            for x in range(self.width):
                rgb = self.canvas.get_at((x, y))[:3]
                idx = self.get_nearest_color_index(rgb)
                self.canvas.set_at((x, y), ZX_PALETTE[idx])

    def get_nearest_color_index(self, rgb):
        """Find nearest ZX Spectrum color index for RGB value"""
        r, g, b = rgb[:3]
        min_dist = float('inf')
        closest = 0
        for i, (pr, pg, pb) in enumerate(ZX_PALETTE):
            dist = (r - pr) ** 2 + (g - pg) ** 2 + (b - pb) ** 2
            if dist < min_dist:
                min_dist = dist
                closest = i
        return closest

    def get_color_index_at(self, x, y):
        """Get ZX palette index at canvas position"""
        if 0 <= x < self.width and 0 <= y < self.height:
            rgb = self.canvas.get_at((x, y))[:3]
            return self.get_nearest_color_index(rgb)
        return 0

    def save_undo(self):
        """Save current state for undo"""
        self.undo_stack.append(self.canvas.copy())
        if len(self.undo_stack) > self.max_undo:
            self.undo_stack.pop(0)

    def undo(self):
        """Restore previous state"""
        if self.undo_stack:
            self.canvas = self.undo_stack.pop()

    def paint_pixel(self, x, y):
        """Paint a single pixel"""
        if 0 <= x < self.width and 0 <= y < self.height:
            self.save_undo()
            self.canvas.set_at((x, y), ZX_PALETTE[self.selected_color])

    def flood_fill(self, start_x, start_y):
        """Flood fill from point"""
        if not (0 <= start_x < self.width and 0 <= start_y < self.height):
            return

        self.save_undo()
        target_color = self.canvas.get_at((start_x, start_y))[:3]
        fill_color = ZX_PALETTE[self.selected_color]

        if target_color == fill_color:
            return

        queue = deque([(start_x, start_y)])
        visited = set()

        while queue:
            x, y = queue.popleft()
            if (x, y) in visited:
                continue
            if not (0 <= x < self.width and 0 <= y < self.height):
                continue

            current = self.canvas.get_at((x, y))[:3]
            if current != target_color:
                continue

            visited.add((x, y))
            self.canvas.set_at((x, y), fill_color)

            queue.extend([(x + 1, y), (x - 1, y), (x, y + 1), (x, y - 1)])

    def replace_color(self, x, y):
        """Replace all pixels of clicked color with selected color"""
        if not (0 <= x < self.width and 0 <= y < self.height):
            return

        self.save_undo()
        target_color = self.canvas.get_at((x, y))[:3]
        fill_color = ZX_PALETTE[self.selected_color]

        for py in range(self.height):
            for px in range(self.width):
                if self.canvas.get_at((px, py))[:3] == target_color:
                    self.canvas.set_at((px, py), fill_color)

    def pick_color(self, x, y):
        """Pick color from canvas"""
        if 0 <= x < self.width and 0 <= y < self.height:
            self.selected_color = self.get_color_index_at(x, y)

    def new_canvas(self):
        """Create new blank canvas (same size)"""
        self.save_undo()
        self.canvas.fill(ZX_PALETTE[0])

    def save_png(self):
        """Save canvas as PNG"""
        if self.input_path:
            path = self.input_path
        else:
            path = f"{self.output_name}.png"
        pygame.image.save(self.canvas, path)
        print(f"Saved: {path}")

    def export_header(self):
        """Export canvas as C header file"""
        script_dir = os.path.dirname(os.path.abspath(__file__))
        src_dir = os.path.join(os.path.dirname(script_dir), 'src')
        output_path = os.path.join(src_dir, f'{self.output_name}.h')

        total_size = self.width * self.height
        name_upper = self.output_name.upper()

        # Build header content
        guard_name = name_upper + '_H'
        output = f'''#ifndef {guard_name}
#define {guard_name}

#include <stdint.h>

// Layer2 image: {self.output_name}
// Resolution: {self.width}x{self.height} (8-bit RGB332)
// Total size: {total_size} bytes
//
// RGB332 format: RRRGGGBB
// ZX Spectrum 16-color palette mapped to RGB332

#define {name_upper}_WIDTH {self.width}
#define {name_upper}_HEIGHT {self.height}
#define {name_upper}_SIZE {total_size}

'''

        # For full-screen images, use bank layout
        if self.width == LAYER2_MAX_WIDTH and self.height == LAYER2_MAX_HEIGHT:
            output += f'#define {name_upper}_BANK_SIZE 8192\n'
            output += f'#define {name_upper}_NUM_BANKS 6\n\n'

            # Generate data for each bank (32 lines each)
            for bank in range(6):
                start_line = bank * 32
                end_line = start_line + 32
                bank_name = f'{self.output_name}_bank{bank}'

                output += f'// Bank {bank}: lines {start_line}-{end_line - 1}\n'
                output += f'static const uint8_t {bank_name}[8192] = {{\n'

                for y in range(start_line, end_line):
                    row_data = []
                    for x in range(self.width):
                        rgb = self.canvas.get_at((x, y))[:3]
                        idx = self.get_nearest_color_index(rgb)
                        rgb332 = ZX_TO_RGB332[idx]
                        row_data.append(f'0x{rgb332:02X}')

                    # Split into groups of 16 for readability
                    for i in range(0, self.width, 16):
                        chunk = row_data[i:i + 16]
                        is_last = (y == end_line - 1) and (i + 16 >= self.width)
                        comma = '' if is_last else ','
                        output += f'    {",".join(chunk)}{comma}\n'

                output += '};\n\n'

            # Generate bank pointer array
            output += f'// Array of bank pointers for easy access\n'
            output += f'static const uint8_t * const {self.output_name}_banks[6] = {{\n'
            for bank in range(6):
                comma = ',' if bank < 5 else ''
                output += f'    {self.output_name}_bank{bank}{comma}\n'
            output += '};\n\n'

            output += f'''// Usage: Copy each bank to Layer2 memory
// Bank 0 -> 8K bank 16 (lines 0-31)
// Bank 1 -> 8K bank 17 (lines 32-63)
// Bank 2 -> 8K bank 18 (lines 64-95)
// Bank 3 -> 8K bank 19 (lines 96-127)
// Bank 4 -> 8K bank 20 (lines 128-159)
// Bank 5 -> 8K bank 21 (lines 160-191)

'''
        else:
            # For smaller images, use a single contiguous array
            output += f'// Image data (row-major order, top to bottom)\n'
            output += f'static const uint8_t {self.output_name}_data[{total_size}] = {{\n'

            for y in range(self.height):
                row_data = []
                for x in range(self.width):
                    rgb = self.canvas.get_at((x, y))[:3]
                    idx = self.get_nearest_color_index(rgb)
                    rgb332 = ZX_TO_RGB332[idx]
                    row_data.append(f'0x{rgb332:02X}')

                # Split into groups of 16 for readability
                for i in range(0, self.width, 16):
                    chunk = row_data[i:i + 16]
                    is_last = (y == self.height - 1) and (i + 16 >= self.width)
                    comma = '' if is_last else ','
                    output += f'    {",".join(chunk)}{comma}\n'

            output += '};\n\n'

            output += f'''// Usage: Draw to Layer2 at position (x, y)
// for (int row = 0; row < {name_upper}_HEIGHT; row++) {{
//     for (int col = 0; col < {name_upper}_WIDTH; col++) {{
//         layer2_plot(x + col, y + row, {self.output_name}_data[row * {name_upper}_WIDTH + col]);
//     }}
// }}

'''

        output += f'#endif // {guard_name}\n'

        with open(output_path, 'w') as f:
            f.write(output)

        print(f"Exported: {output_path}")
        print(f"  Size: {self.width}x{self.height} = {total_size} bytes")

    def draw(self):
        """Draw the editor interface"""
        self.screen.fill((48, 48, 48))

        # Center canvas if window is wider
        canvas_x = (self.window_width - self.canvas_display_width) // 2
        canvas_y = self.padding

        # Draw checkerboard background (for transparency reference)
        for cy in range(0, self.canvas_display_height, 16):
            for cx in range(0, self.canvas_display_width, 16):
                checker = ((cx // 16 + cy // 16) % 2) * 20 + 40
                rect = pygame.Rect(canvas_x + cx, canvas_y + cy, 16, 16)
                pygame.draw.rect(self.screen, (checker, checker, checker), rect)

        # Draw canvas
        scaled_canvas = pygame.transform.scale(
            self.canvas,
            (self.canvas_display_width, self.canvas_display_height)
        )
        self.screen.blit(scaled_canvas, (canvas_x, canvas_y))

        # Canvas border
        pygame.draw.rect(self.screen, (100, 100, 100),
                        (canvas_x - 1, canvas_y - 1,
                         self.canvas_display_width + 2, self.canvas_display_height + 2), 1)

        # Palette area (centered)
        palette_y = canvas_y + self.canvas_display_height + self.padding
        color_size = 36
        color_gap = 4
        total_palette_width = 16 * color_size + 15 * color_gap
        palette_x = (self.window_width - total_palette_width) // 2

        for i, color in enumerate(ZX_PALETTE):
            rect = pygame.Rect(
                palette_x + i * (color_size + color_gap),
                palette_y,
                color_size, color_size
            )
            pygame.draw.rect(self.screen, color, rect)

            # Selection highlight
            if i == self.selected_color:
                pygame.draw.rect(self.screen, (255, 255, 0), rect, 3)
            else:
                pygame.draw.rect(self.screen, (80, 80, 80), rect, 1)

            # Color index
            text_color = (255, 255, 255) if sum(color) < 400 else (0, 0, 0)
            label = self.small_font.render(str(i), True, text_color)
            self.screen.blit(label, (rect.x + 2, rect.y + 2))

        # Selected color info
        info_y = palette_y + color_size + 10
        color_info = f"Color {self.selected_color}: {ZX_COLOR_NAMES[self.selected_color]} (RGB332: 0x{ZX_TO_RGB332[self.selected_color]:02X}) | Size: {self.width}x{self.height}"
        label = self.font.render(color_info, True, (200, 200, 200))
        self.screen.blit(label, (self.padding, info_y))

        # Instructions
        inst_y = info_y + 25
        instructions = "Click: Paint | Opt+Click: Fill | Ctrl+Click: Replace | Right-click: Pick | S: Save | E: Export | N: New | Z: Undo"
        label = self.small_font.render(instructions, True, (150, 150, 150))
        self.screen.blit(label, (self.padding, inst_y))

        pygame.display.flip()

    def run(self):
        """Main editor loop"""
        clock = pygame.time.Clock()
        running = True
        mouse_down = False
        last_paint_pos = None

        # Calculate canvas position (centered)
        canvas_x = (self.window_width - self.canvas_display_width) // 2
        canvas_y = self.padding

        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                    elif event.key == pygame.K_s:
                        self.save_png()
                    elif event.key == pygame.K_e:
                        self.export_header()
                    elif event.key == pygame.K_n:
                        self.new_canvas()
                    elif event.key == pygame.K_z:
                        self.undo()

                elif event.type == pygame.MOUSEBUTTONDOWN:
                    mx, my = event.pos

                    # Canvas click
                    if (canvas_x <= mx < canvas_x + self.canvas_display_width and
                        canvas_y <= my < canvas_y + self.canvas_display_height):

                        px = (mx - canvas_x) // self.scale
                        py = (my - canvas_y) // self.scale

                        if event.button == 1:  # Left click
                            mods = pygame.key.get_mods()
                            if mods & pygame.KMOD_ALT:
                                self.flood_fill(px, py)
                            elif mods & pygame.KMOD_CTRL:
                                self.replace_color(px, py)
                            else:
                                mouse_down = True
                                last_paint_pos = (px, py)
                                self.paint_pixel(px, py)
                        elif event.button == 3:  # Right click - pick color
                            self.pick_color(px, py)

                    # Palette click
                    color_size = 36
                    color_gap = 4
                    total_palette_width = 16 * color_size + 15 * color_gap
                    palette_x = (self.window_width - total_palette_width) // 2
                    palette_y = canvas_y + self.canvas_display_height + self.padding

                    for i in range(16):
                        rect = pygame.Rect(
                            palette_x + i * (color_size + color_gap),
                            palette_y,
                            color_size, color_size
                        )
                        if rect.collidepoint(mx, my):
                            self.selected_color = i
                            break

                elif event.type == pygame.MOUSEBUTTONUP:
                    if event.button == 1:
                        mouse_down = False
                        last_paint_pos = None

                elif event.type == pygame.MOUSEMOTION:
                    if mouse_down:
                        mx, my = event.pos

                        if (canvas_x <= mx < canvas_x + self.canvas_display_width and
                            canvas_y <= my < canvas_y + self.canvas_display_height):

                            px = (mx - canvas_x) // self.scale
                            py = (my - canvas_y) // self.scale

                            if (px, py) != last_paint_pos:
                                last_paint_pos = (px, py)
                                self.paint_pixel(px, py)

            self.draw()
            clock.tick(60)

        pygame.quit()


def convert_only(input_path, output_name):
    """Convert PNG to header without GUI"""
    pygame.init()

    if not os.path.exists(input_path):
        print(f"Error: File not found: {input_path}")
        return False

    editor = Layer2Editor.__new__(Layer2Editor)
    editor.output_name = output_name

    # Load image to get dimensions
    img = pygame.image.load(input_path)
    img_w, img_h = img.get_size()

    # Clamp to max dimensions
    editor.width = min(img_w, LAYER2_MAX_WIDTH)
    editor.height = min(img_h, LAYER2_MAX_HEIGHT)

    editor.canvas = pygame.Surface((editor.width, editor.height))
    editor.canvas.fill(ZX_PALETTE[0])

    # Crop if needed
    if img_w > LAYER2_MAX_WIDTH or img_h > LAYER2_MAX_HEIGHT:
        crop_rect = pygame.Rect(0, 0, editor.width, editor.height)
        editor.canvas.blit(img, (0, 0), crop_rect)
    else:
        editor.canvas.blit(img, (0, 0))

    # Quantize and export
    editor.quantize_to_zx_palette()
    editor.export_header()
    return True


if __name__ == '__main__':
    pygame.init()

    if len(sys.argv) < 2:
        print(__doc__)
        print("\nStarting with blank 256x192 canvas...")
        editor = Layer2Editor()
        editor.run()
    elif sys.argv[1] == '--convert':
        # Command-line conversion mode: --convert input.png output_name
        if len(sys.argv) < 4:
            print("Usage: python3 layer2_editor.py --convert <input.png> <output_name>")
            sys.exit(1)
        convert_only(sys.argv[2], sys.argv[3])
    else:
        input_path = sys.argv[1]
        output_name = sys.argv[2] if len(sys.argv) > 2 else "layer2_image"

        if not os.path.exists(input_path):
            print(f"Error: File not found: {input_path}")
            sys.exit(1)

        editor = Layer2Editor(input_path, output_name)
        editor.run()
