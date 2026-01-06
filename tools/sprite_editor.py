#!/usr/bin/env python3
"""
Sprite/Tile Editor for ZX Spectrum Next
- Load a spritesheet/tilesheet
- Select sprite/tile to edit in paint grid
- Paint with 16 ZX Spectrum colors
- Fill functions (flood fill, replace all same color)
- Save modified sheet

Usage: python3 sprite_editor.py <input.png> [tile_size]
  tile_size: 8 or 16 (default: 16)

Controls:
  Click on sheet: Select sprite/tile to edit
  Click on grid: Paint with selected color
  Opt+Click on grid: Flood fill area
  Ctrl+Click on grid: Replace all pixels of clicked color with selected color
  Click on palette: Select color
  Mouse wheel on sheet: Scroll if sheet is large
  S: Save
  Z: Undo last change
  ESC: Quit
"""

import pygame
import sys
import os
from collections import deque

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

# Palette index 11 (bright magenta) is the transparent color in the game
# Paint it as solid color - the game hardware handles transparency

class SpriteEditor:
    def __init__(self, input_path, tile_size=16):
        pygame.init()

        self.input_path = input_path
        self.tile_size = tile_size
        self.grid_scale = 20  # Pixels per grid cell in editor
        self.scroll_y = 0
        self.selected_color = 0
        self.selected_tile = None
        self.undo_stack = []
        self.max_undo = 20

        # Load source image
        self.source_image = pygame.image.load(input_path)
        self.source_width = self.source_image.get_width()
        self.source_height = self.source_image.get_height()

        # Calculate tiles
        self.tiles_x = self.source_width // tile_size
        self.tiles_y = self.source_height // tile_size

        # Layout dimensions
        self.sheet_display_width = min(self.source_width * 2, 400)  # Scale up sheet view
        self.sheet_scale = self.sheet_display_width / self.source_width
        self.sheet_display_height = int(self.source_height * self.sheet_scale)

        self.grid_size = tile_size * self.grid_scale  # Paint grid size
        self.bottom_area_height = 100  # Space for palette and instructions
        self.padding = 20

        # Window size
        # Palette needs: 16 colors * (36 + 4) = 640 + padding
        palette_width = 16 * 40 + self.padding * 2
        self.window_width = max(self.sheet_display_width + self.grid_size + self.padding * 3, palette_width)
        self.image_area_height = max(self.sheet_display_height, self.grid_size)
        self.window_height = self.image_area_height + self.bottom_area_height + self.padding * 3

        # Visible area for sheet (above bottom area)
        self.visible_sheet_height = self.image_area_height

        self.screen = pygame.display.set_mode((self.window_width, self.window_height))
        pygame.display.set_caption(f"Sprite Editor - {os.path.basename(input_path)} ({tile_size}x{tile_size})")

        # Convert source for editing
        self.source_image = self.source_image.convert_alpha()

        # Create working copy for editing
        self.working_image = self.source_image.copy()

        # Current tile being edited (as pygame Surface)
        self.current_tile = None

        # Font for labels
        self.font = pygame.font.Font(None, 24)
        self.small_font = pygame.font.Font(None, 18)

    def get_color_index(self, rgb):
        """Find closest ZX Spectrum color index for RGB value"""
        r, g, b = rgb[:3]
        min_dist = float('inf')
        closest = 0
        for i, (pr, pg, pb) in enumerate(ZX_PALETTE):
            dist = (r - pr) ** 2 + (g - pg) ** 2 + (b - pb) ** 2
            if dist < min_dist:
                min_dist = dist
                closest = i
        return closest

    def select_tile(self, tx, ty):
        """Select a tile for editing"""
        if 0 <= tx < self.tiles_x and 0 <= ty < self.tiles_y:
            self.selected_tile = (tx, ty)
            # Extract tile from working image
            self.current_tile = pygame.Surface((self.tile_size, self.tile_size), pygame.SRCALPHA)
            src_rect = pygame.Rect(tx * self.tile_size, ty * self.tile_size,
                                   self.tile_size, self.tile_size)
            self.current_tile.blit(self.working_image, (0, 0), src_rect)

    def save_undo(self):
        """Save current state for undo"""
        if self.current_tile:
            self.undo_stack.append(self.current_tile.copy())
            if len(self.undo_stack) > self.max_undo:
                self.undo_stack.pop(0)

    def undo(self):
        """Restore previous state"""
        if self.undo_stack and self.current_tile:
            self.current_tile = self.undo_stack.pop()
            self.apply_tile_to_working()

    def apply_tile_to_working(self):
        """Apply current tile back to working image"""
        if self.selected_tile and self.current_tile:
            tx, ty = self.selected_tile
            dest_rect = pygame.Rect(tx * self.tile_size, ty * self.tile_size,
                                    self.tile_size, self.tile_size)
            self.working_image.blit(self.current_tile, dest_rect)

    def paint_pixel(self, px, py):
        """Paint a single pixel with selected color"""
        if self.current_tile and 0 <= px < self.tile_size and 0 <= py < self.tile_size:
            self.save_undo()
            color = ZX_PALETTE[self.selected_color]
            # All colors are solid - bright magenta (11) will be transparent in the game
            self.current_tile.set_at((px, py), (*color, 255))
            self.apply_tile_to_working()

    def flood_fill(self, px, py):
        """Flood fill from point with selected color"""
        if not self.current_tile or not (0 <= px < self.tile_size and 0 <= py < self.tile_size):
            return

        self.save_undo()
        target_color = self.current_tile.get_at((px, py))
        fill_color = (*ZX_PALETTE[self.selected_color], 255)

        # Don't fill if same color
        if target_color[:3] == fill_color[:3]:
            return

        # BFS flood fill
        queue = deque([(px, py)])
        visited = set()

        while queue:
            x, y = queue.popleft()
            if (x, y) in visited:
                continue
            if not (0 <= x < self.tile_size and 0 <= y < self.tile_size):
                continue

            current = self.current_tile.get_at((x, y))
            # Match color (with some tolerance for alpha)
            if abs(current[0] - target_color[0]) > 10 or \
               abs(current[1] - target_color[1]) > 10 or \
               abs(current[2] - target_color[2]) > 10:
                continue

            visited.add((x, y))
            self.current_tile.set_at((x, y), fill_color)

            queue.append((x + 1, y))
            queue.append((x - 1, y))
            queue.append((x, y + 1))
            queue.append((x, y - 1))

        self.apply_tile_to_working()

    def replace_color(self, px, py):
        """Replace all pixels of clicked color with selected color"""
        if not self.current_tile or not (0 <= px < self.tile_size and 0 <= py < self.tile_size):
            return

        self.save_undo()
        target_color = self.current_tile.get_at((px, py))
        fill_color = (*ZX_PALETTE[self.selected_color], 255)

        # Replace all matching pixels
        for y in range(self.tile_size):
            for x in range(self.tile_size):
                current = self.current_tile.get_at((x, y))
                if abs(current[0] - target_color[0]) <= 10 and \
                   abs(current[1] - target_color[1]) <= 10 and \
                   abs(current[2] - target_color[2]) <= 10:
                    self.current_tile.set_at((x, y), fill_color)

        self.apply_tile_to_working()

    def save(self):
        """Save working image back to file"""
        pygame.image.save(self.working_image, self.input_path)
        print(f"Saved: {self.input_path}")

    def draw(self):
        """Draw the editor interface"""
        self.screen.fill((48, 48, 48))

        # Draw sheet area (left side)
        sheet_x = self.padding
        sheet_y = self.padding

        # Scale and draw visible portion of sheet
        scaled_sheet = pygame.transform.scale(self.working_image,
                                              (int(self.source_width * self.sheet_scale),
                                               int(self.source_height * self.sheet_scale)))

        # Clip to visible area
        clip_rect = pygame.Rect(0, self.scroll_y,
                                self.sheet_display_width,
                                min(self.visible_sheet_height, self.sheet_display_height))

        self.screen.blit(scaled_sheet, (sheet_x, sheet_y - self.scroll_y))

        # Draw grid on sheet
        scaled_tile = int(self.tile_size * self.sheet_scale)
        for x in range(self.tiles_x + 1):
            px = sheet_x + x * scaled_tile
            pygame.draw.line(self.screen, (100, 100, 100),
                           (px, sheet_y), (px, sheet_y + self.sheet_display_height - self.scroll_y))
        for y in range(self.tiles_y + 1):
            py = sheet_y + y * scaled_tile - self.scroll_y
            if sheet_y <= py <= sheet_y + self.visible_sheet_height:
                pygame.draw.line(self.screen, (100, 100, 100),
                               (sheet_x, py), (sheet_x + self.sheet_display_width, py))

        # Highlight selected tile
        if self.selected_tile:
            tx, ty = self.selected_tile
            sel_rect = pygame.Rect(sheet_x + tx * scaled_tile,
                                   sheet_y + ty * scaled_tile - self.scroll_y,
                                   scaled_tile, scaled_tile)
            pygame.draw.rect(self.screen, (255, 255, 0), sel_rect, 3)

        # Draw tile index below sheet
        if self.selected_tile:
            tx, ty = self.selected_tile
            col_letter = chr(ord('A') + tx)
            coord_text = f"Tile: {col_letter}{ty}"
            coord_label = self.font.render(coord_text, True, (255, 255, 0))
            self.screen.blit(coord_label, (sheet_x, sheet_y + self.image_area_height + 5))

        # Draw paint grid (right side)
        grid_x = self.sheet_display_width + self.padding * 2
        grid_y = self.padding

        # Checkerboard background for transparency
        for py in range(self.tile_size):
            for px in range(self.tile_size):
                checker = ((px + py) % 2) * 30 + 60
                rect = pygame.Rect(grid_x + px * self.grid_scale,
                                  grid_y + py * self.grid_scale,
                                  self.grid_scale, self.grid_scale)
                pygame.draw.rect(self.screen, (checker, checker, checker), rect)

        # Draw current tile pixels
        if self.current_tile:
            for py in range(self.tile_size):
                for px in range(self.tile_size):
                    color = self.current_tile.get_at((px, py))
                    if color[3] > 0:  # Not transparent
                        rect = pygame.Rect(grid_x + px * self.grid_scale,
                                          grid_y + py * self.grid_scale,
                                          self.grid_scale, self.grid_scale)
                        pygame.draw.rect(self.screen, color[:3], rect)

        # Draw grid lines
        for x in range(self.tile_size + 1):
            px = grid_x + x * self.grid_scale
            pygame.draw.line(self.screen, (80, 80, 80),
                           (px, grid_y), (px, grid_y + self.grid_size))
        for y in range(self.tile_size + 1):
            py = grid_y + y * self.grid_scale
            pygame.draw.line(self.screen, (80, 80, 80),
                           (grid_x, py), (grid_x + self.grid_size, py))

        # Draw grid border
        pygame.draw.rect(self.screen, (200, 200, 200),
                        (grid_x, grid_y, self.grid_size, self.grid_size), 2)

        # Draw tile coordinates below grid
        if self.selected_tile:
            tx, ty = self.selected_tile
            col_letter = chr(ord('A') + tx)
            coord_text = f"Tile: {col_letter}{ty}"
            coord_label = self.font.render(coord_text, True, (255, 255, 0))
            self.screen.blit(coord_label, (grid_x, grid_y + self.grid_size + 5))

        # Bottom area: palette and instructions
        bottom_y = self.padding + self.image_area_height + self.padding

        # Draw separator line
        pygame.draw.line(self.screen, (80, 80, 80),
                        (self.padding, bottom_y - 10),
                        (self.window_width - self.padding, bottom_y - 10))

        # Draw color palette (16 colors in one row)
        palette_x = self.padding
        palette_y = bottom_y
        color_size = 36

        for i, color in enumerate(ZX_PALETTE):
            rect = pygame.Rect(palette_x + i * (color_size + 4),
                              palette_y,
                              color_size, color_size)

            # Checkerboard for color 11 (transparent)
            if i == 11:
                for cy in range(4):
                    for cx in range(4):
                        checker = ((cx + cy) % 2) * 40 + 80
                        sub_rect = pygame.Rect(rect.x + cx * 9, rect.y + cy * 9, 9, 9)
                        pygame.draw.rect(self.screen, (checker, checker, checker), sub_rect)
            else:
                pygame.draw.rect(self.screen, color, rect)

            # Selection highlight
            if i == self.selected_color:
                pygame.draw.rect(self.screen, (255, 255, 0), rect, 3)
            else:
                pygame.draw.rect(self.screen, (100, 100, 100), rect, 1)

            # Color index label
            idx_label = self.small_font.render(str(i), True,
                                               (0, 0, 0) if sum(color) > 400 else (255, 255, 255))
            self.screen.blit(idx_label, (rect.x + 2, rect.y + 2))

        # Draw instructions below palette
        inst_y = palette_y + color_size + 10
        instructions = "Click: Paint | Opt+Click: Flood fill | Ctrl+Click: Replace color | S: Save | Z: Undo | ESC: Quit"
        label = self.small_font.render(instructions, True, (150, 150, 150))
        self.screen.blit(label, (palette_x, inst_y))

        pygame.display.flip()

    def run(self):
        """Main editor loop"""
        clock = pygame.time.Clock()
        running = True
        mouse_down = False
        last_paint_pos = None

        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False

                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_ESCAPE:
                        running = False
                    elif event.key == pygame.K_s:
                        self.save()
                    elif event.key == pygame.K_z:
                        self.undo()

                elif event.type == pygame.MOUSEBUTTONDOWN:
                    mx, my = event.pos

                    # Check sheet click
                    sheet_x = self.padding
                    sheet_y = self.padding
                    scaled_tile = int(self.tile_size * self.sheet_scale)

                    if (sheet_x <= mx < sheet_x + self.sheet_display_width and
                        sheet_y <= my < sheet_y + self.visible_sheet_height):
                        # Clicked on sheet
                        tx = int((mx - sheet_x) / scaled_tile)
                        ty = int((my - sheet_y + self.scroll_y) / scaled_tile)
                        self.select_tile(tx, ty)

                    # Check grid click
                    grid_x = self.sheet_display_width + self.padding * 2
                    grid_y = self.padding

                    if (grid_x <= mx < grid_x + self.grid_size and
                        grid_y <= my < grid_y + self.grid_size):
                        px = int((mx - grid_x) / self.grid_scale)
                        py = int((my - grid_y) / self.grid_scale)

                        if event.button == 1:  # Left click
                            mods = pygame.key.get_mods()
                            if mods & pygame.KMOD_ALT:  # Option + click - flood fill
                                self.flood_fill(px, py)
                            elif mods & pygame.KMOD_CTRL:  # Ctrl + click - replace color
                                self.replace_color(px, py)
                            else:  # Normal click - paint
                                mouse_down = True
                                last_paint_pos = (px, py)
                                self.paint_pixel(px, py)

                    # Check palette click (16 colors in one row at bottom)
                    palette_x = self.padding
                    palette_y = self.padding + self.image_area_height + self.padding
                    color_size = 36

                    for i in range(16):
                        rect = pygame.Rect(palette_x + i * (color_size + 4),
                                          palette_y,
                                          color_size, color_size)
                        if rect.collidepoint(mx, my):
                            self.selected_color = i
                            break

                    # Scroll with mouse wheel
                    if event.button == 4:  # Scroll up
                        self.scroll_y = max(0, self.scroll_y - 32)
                    elif event.button == 5:  # Scroll down
                        max_scroll = max(0, self.sheet_display_height - self.visible_sheet_height)
                        self.scroll_y = min(max_scroll, self.scroll_y + 32)

                elif event.type == pygame.MOUSEBUTTONUP:
                    if event.button == 1:
                        mouse_down = False
                        last_paint_pos = None

                elif event.type == pygame.MOUSEMOTION:
                    if mouse_down and self.current_tile:
                        mx, my = event.pos
                        grid_x = self.sheet_display_width + self.padding * 2
                        grid_y = self.padding

                        if (grid_x <= mx < grid_x + self.grid_size and
                            grid_y <= my < grid_y + self.grid_size):
                            px = int((mx - grid_x) / self.grid_scale)
                            py = int((my - grid_y) / self.grid_scale)

                            if (px, py) != last_paint_pos:
                                last_paint_pos = (px, py)
                                self.paint_pixel(px, py)

            self.draw()
            clock.tick(60)

        pygame.quit()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(1)

    input_path = sys.argv[1]
    tile_size = int(sys.argv[2]) if len(sys.argv) > 2 else 16

    if tile_size not in (8, 16):
        print("Error: tile_size must be 8 or 16")
        sys.exit(1)

    if not os.path.exists(input_path):
        print(f"Error: File not found: {input_path}")
        sys.exit(1)

    editor = SpriteEditor(input_path, tile_size)
    editor.run()
