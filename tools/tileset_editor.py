#!/usr/bin/env python3
"""
Nebula 8 Tileset Editor
- Left: Source tilemap2.png (variable size)
- Right: Editable canvas (16x16 tiles, 128x128 pixels)
- Place, delete, rotate, mirror tiles
- Save as PNG

Controls:
- Left click on left: Select tile
- Left click on right: Place tile
- Option/Alt + Left click: Place 2x2 block
- Right click on right: Delete tile
- R: Rotate selected tile on right (90° clockwise cycle)
- H: Mirror horizontal (flip tile on right)
- V: Mirror vertical (flip tile on right)
- Ctrl+S: Save
- Ctrl+O: Load existing tileset
- Ctrl+N: Clear right side
"""

import pygame
import os
import sys

# Constants
TILE_SIZE = 8
DISPLAY_SCALE = 3  # Scale up for visibility
SCALED_TILE = TILE_SIZE * DISPLAY_SCALE

# Output grid is fixed at 16x16 tiles (128x128 pixels)
OUTPUT_COLS = 16
OUTPUT_ROWS = 16
OUTPUT_TOTAL = OUTPUT_COLS * OUTPUT_ROWS

# Colors
BG_COLOR = (30, 30, 40)
GRID_COLOR = (60, 60, 70)
SELECT_COLOR = (255, 255, 0)
HOVER_COLOR = (100, 100, 255)
TEXT_COLOR = (200, 200, 200)
BUTTON_COLOR = (60, 60, 80)
BUTTON_HOVER = (80, 80, 100)
BUTTON_ACTIVE = (100, 100, 150)
EMPTY_TILE_COLOR = (20, 20, 30)


class TilesetEditor:
    def __init__(self):
        pygame.init()
        pygame.display.set_caption("Nebula 8 Tileset Editor")

        self.toolbar_height = 60
        self.gap = 40  # Gap between left and right

        # Get source image dimensions first (without convert_alpha)
        script_dir = os.path.dirname(os.path.abspath(__file__))
        tilemap_path = os.path.join(script_dir, '..', 'art', 'tilemap2.png')
        try:
            temp_img = pygame.image.load(tilemap_path)
            img_w, img_h = temp_img.get_size()
        except pygame.error as e:
            print(f"Cannot find tilemap2.png at {tilemap_path}: {e}")
            sys.exit(1)

        self.tile_src_size = TILE_SIZE
        self.src_cols = img_w // self.tile_src_size
        self.src_rows = img_h // self.tile_src_size
        self.src_total = self.src_cols * self.src_rows

        # Calculate window size based on source + fixed output
        self.left_width = self.src_cols * SCALED_TILE
        self.left_height = self.src_rows * SCALED_TILE
        self.right_width = OUTPUT_COLS * SCALED_TILE
        self.right_height = OUTPUT_ROWS * SCALED_TILE

        self.window_width = self.left_width + self.gap + self.right_width
        self.window_height = max(self.left_height, self.right_height) + self.toolbar_height + 100

        self.screen = pygame.display.set_mode((self.window_width, self.window_height))
        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 24)
        self.small_font = pygame.font.Font(None, 20)

        # Now load and extract tiles with proper display
        self.load_source_tilemap(tilemap_path)

        # Regions
        self.left_rect = pygame.Rect(0, self.toolbar_height, self.left_width, self.left_height)
        self.right_rect = pygame.Rect(self.left_width + self.gap, self.toolbar_height,
                                       self.right_width, self.right_height)

        # Editor state
        self.selected_tile = 0
        self.current_file = None
        self.unsaved = False

        # Initialize right side (editable) - stores (tile_idx, rotation, h_flip, v_flip)
        # None means empty tile
        self.right_tiles = [None] * OUTPUT_TOTAL

        # Create buttons
        self.buttons = []
        self.create_buttons()

    def load_source_tilemap(self, tilemap_path):
        """Load tilemap2.png as source and extract tiles"""
        self.source_img = pygame.image.load(tilemap_path).convert_alpha()

        # Extract individual tiles
        self.source_tiles = []
        self.source_tiles_scaled = []

        for row in range(self.src_rows):
            for col in range(self.src_cols):
                x = col * self.tile_src_size
                y = row * self.tile_src_size
                tile = self.source_img.subsurface((x, y, self.tile_src_size, self.tile_src_size))
                self.source_tiles.append(tile.copy())

                # Scaled version for display
                tile_scaled = pygame.transform.scale(tile, (SCALED_TILE, SCALED_TILE))
                self.source_tiles_scaled.append(tile_scaled)

    def create_buttons(self):
        """Create UI buttons"""
        y = 10
        x = 10

        self.buttons = [
            {'rect': pygame.Rect(x, y, 60, 30), 'text': 'New', 'action': self.new_tileset},
            {'rect': pygame.Rect(x + 70, y, 60, 30), 'text': 'Load', 'action': self.load_tileset},
            {'rect': pygame.Rect(x + 140, y, 60, 30), 'text': 'Save', 'action': self.save_tileset},
            {'rect': pygame.Rect(x + 220, y, 80, 30), 'text': 'Rotate [R]', 'action': self.rotate_at_cursor},
            {'rect': pygame.Rect(x + 310, y, 80, 30), 'text': 'FlipH [H]', 'action': self.flip_h_at_cursor},
            {'rect': pygame.Rect(x + 400, y, 80, 30), 'text': 'FlipV [V]', 'action': self.flip_v_at_cursor},
        ]

    def get_tile_at_cursor(self):
        """Get tile index on right side under cursor"""
        pos = pygame.mouse.get_pos()
        if self.right_rect.collidepoint(pos):
            rel_x = pos[0] - self.right_rect.x
            rel_y = pos[1] - self.right_rect.y
            col = rel_x // SCALED_TILE
            row = rel_y // SCALED_TILE
            if 0 <= col < OUTPUT_COLS and 0 <= row < OUTPUT_ROWS:
                return row * OUTPUT_COLS + col
        return None

    def rotate_at_cursor(self):
        """Rotate tile at cursor position"""
        idx = self.get_tile_at_cursor()
        if idx is not None and self.right_tiles[idx] is not None:
            tile_idx, rotation, h_flip, v_flip = self.right_tiles[idx]
            rotation = (rotation + 90) % 360
            self.right_tiles[idx] = (tile_idx, rotation, h_flip, v_flip)
            self.unsaved = True

    def flip_h_at_cursor(self):
        """Flip tile horizontally at cursor position"""
        idx = self.get_tile_at_cursor()
        if idx is not None and self.right_tiles[idx] is not None:
            tile_idx, rotation, h_flip, v_flip = self.right_tiles[idx]
            h_flip = not h_flip
            self.right_tiles[idx] = (tile_idx, rotation, h_flip, v_flip)
            self.unsaved = True

    def flip_v_at_cursor(self):
        """Flip tile vertically at cursor position"""
        idx = self.get_tile_at_cursor()
        if idx is not None and self.right_tiles[idx] is not None:
            tile_idx, rotation, h_flip, v_flip = self.right_tiles[idx]
            v_flip = not v_flip
            self.right_tiles[idx] = (tile_idx, rotation, h_flip, v_flip)
            self.unsaved = True

    def run(self):
        """Main loop"""
        running = True
        while running:
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    running = False
                elif event.type == pygame.KEYDOWN:
                    self.handle_key(event)
                elif event.type == pygame.MOUSEBUTTONDOWN:
                    self.handle_click(event)
                elif event.type == pygame.MOUSEMOTION:
                    if pygame.mouse.get_pressed()[0]:
                        self.handle_drag(event)

            self.draw()
            pygame.display.flip()
            self.clock.tick(60)

        pygame.quit()

    def handle_key(self, event):
        """Handle keyboard input"""
        mods = pygame.key.get_mods()

        if event.key == pygame.K_r:
            self.rotate_at_cursor()
        elif event.key == pygame.K_h:
            self.flip_h_at_cursor()
        elif event.key == pygame.K_v:
            self.flip_v_at_cursor()
        elif event.key == pygame.K_s and (mods & pygame.KMOD_CTRL or mods & pygame.KMOD_META):
            self.save_tileset()
        elif event.key == pygame.K_o and (mods & pygame.KMOD_CTRL or mods & pygame.KMOD_META):
            self.load_tileset()
        elif event.key == pygame.K_n and (mods & pygame.KMOD_CTRL or mods & pygame.KMOD_META):
            self.new_tileset()

    def handle_click(self, event):
        """Handle mouse click"""
        pos = event.pos

        # Check buttons
        for btn in self.buttons:
            if btn['rect'].collidepoint(pos):
                btn['action']()
                return

        # Check left side (source) - select tile
        if self.left_rect.collidepoint(pos):
            rel_x = pos[0] - self.left_rect.x
            rel_y = pos[1] - self.left_rect.y
            col = rel_x // SCALED_TILE
            row = rel_y // SCALED_TILE
            if 0 <= col < self.src_cols and 0 <= row < self.src_rows:
                self.selected_tile = row * self.src_cols + col
            return

        # Check right side (editable)
        if self.right_rect.collidepoint(pos):
            self.handle_right_click(pos, event.button)

    def handle_right_click(self, pos, button):
        """Handle click on right (editable) side"""
        rel_x = pos[0] - self.right_rect.x
        rel_y = pos[1] - self.right_rect.y
        col = rel_x // SCALED_TILE
        row = rel_y // SCALED_TILE

        if not (0 <= col < OUTPUT_COLS and 0 <= row < OUTPUT_ROWS):
            return

        idx = row * OUTPUT_COLS + col

        if button == 3:  # Right click - delete
            self.right_tiles[idx] = None
            self.unsaved = True
            return

        if button == 1:  # Left click - place
            mods = pygame.key.get_mods()
            if mods & pygame.KMOD_ALT:
                self.place_block(col, row)
            else:
                # Place tile with no rotation/flip
                self.right_tiles[idx] = (self.selected_tile, 0, False, False)
            self.unsaved = True

    def place_block(self, col, row):
        """Place a 2x2 block"""
        base = self.selected_tile
        base_col = base % self.src_cols
        base_row = base // self.src_cols

        # Top-left
        idx = row * OUTPUT_COLS + col
        if idx < OUTPUT_TOTAL:
            self.right_tiles[idx] = (base, 0, False, False)

        # Top-right
        if col + 1 < OUTPUT_COLS and base_col + 1 < self.src_cols:
            idx = row * OUTPUT_COLS + (col + 1)
            if idx < OUTPUT_TOTAL:
                self.right_tiles[idx] = (base + 1, 0, False, False)

        # Bottom-left
        if row + 1 < OUTPUT_ROWS and base_row + 1 < self.src_rows:
            idx = (row + 1) * OUTPUT_COLS + col
            if idx < OUTPUT_TOTAL:
                self.right_tiles[idx] = (base + self.src_cols, 0, False, False)

        # Bottom-right
        if col + 1 < OUTPUT_COLS and row + 1 < OUTPUT_ROWS and base_col + 1 < self.src_cols and base_row + 1 < self.src_rows:
            idx = (row + 1) * OUTPUT_COLS + (col + 1)
            if idx < OUTPUT_TOTAL:
                self.right_tiles[idx] = (base + self.src_cols + 1, 0, False, False)

    def handle_drag(self, event):
        """Handle mouse drag"""
        pos = event.pos
        if self.right_rect.collidepoint(pos):
            rel_x = pos[0] - self.right_rect.x
            rel_y = pos[1] - self.right_rect.y
            col = rel_x // SCALED_TILE
            row = rel_y // SCALED_TILE

            if 0 <= col < OUTPUT_COLS and 0 <= row < OUTPUT_ROWS:
                idx = row * OUTPUT_COLS + col
                mods = pygame.key.get_mods()
                if mods & pygame.KMOD_ALT:
                    self.place_block(col, row)
                else:
                    self.right_tiles[idx] = (self.selected_tile, 0, False, False)
                self.unsaved = True

    def get_transformed_tile(self, tile_data):
        """Get a tile surface with rotation and flips applied"""
        tile_idx, rotation, h_flip, v_flip = tile_data
        tile = self.source_tiles_scaled[tile_idx].copy()

        if h_flip:
            tile = pygame.transform.flip(tile, True, False)
        if v_flip:
            tile = pygame.transform.flip(tile, False, True)
        if rotation != 0:
            tile = pygame.transform.rotate(tile, -rotation)  # Negative for clockwise

        return tile

    def draw(self):
        """Draw everything"""
        self.screen.fill(BG_COLOR)

        # Draw toolbar
        pygame.draw.rect(self.screen, (40, 40, 50), (0, 0, self.window_width, self.toolbar_height))

        # Draw buttons
        mouse_pos = pygame.mouse.get_pos()
        for btn in self.buttons:
            color = BUTTON_COLOR
            if btn['rect'].collidepoint(mouse_pos):
                color = BUTTON_HOVER
            pygame.draw.rect(self.screen, color, btn['rect'], border_radius=4)
            pygame.draw.rect(self.screen, (100, 100, 120), btn['rect'], 1, border_radius=4)
            text = self.small_font.render(btn['text'], True, TEXT_COLOR)
            text_rect = text.get_rect(center=btn['rect'].center)
            self.screen.blit(text, text_rect)

        # File info
        file_text = f"File: {os.path.basename(self.current_file) if self.current_file else '(unsaved)'}"
        if self.unsaved:
            file_text += " *"
        text = self.small_font.render(file_text, True, TEXT_COLOR)
        self.screen.blit(text, (500, 15))

        # Labels
        label_y = self.toolbar_height + max(self.left_height, self.right_height) + 10
        text = self.font.render("Source (click to select)", True, TEXT_COLOR)
        self.screen.blit(text, (10, label_y))

        text = self.font.render("Output (click to place, R/H/V to transform)", True, TEXT_COLOR)
        self.screen.blit(text, (self.left_width + self.gap + 10, label_y))

        # Draw left side (source)
        for idx, tile in enumerate(self.source_tiles_scaled):
            col = idx % self.src_cols
            row = idx // self.src_cols
            x = self.left_rect.x + col * SCALED_TILE
            y = self.left_rect.y + row * SCALED_TILE
            self.screen.blit(tile, (x, y))

        # Draw left grid
        for x in range(0, self.left_width + 1, SCALED_TILE):
            pygame.draw.line(self.screen, GRID_COLOR,
                           (self.left_rect.x + x, self.left_rect.y),
                           (self.left_rect.x + x, self.left_rect.y + self.left_height))
        for y in range(0, self.left_height + 1, SCALED_TILE):
            pygame.draw.line(self.screen, GRID_COLOR,
                           (self.left_rect.x, self.left_rect.y + y),
                           (self.left_rect.x + self.left_width, self.left_rect.y + y))

        # Selection highlight on left
        sel_col = self.selected_tile % self.src_cols
        sel_row = self.selected_tile // self.src_cols
        sel_rect = pygame.Rect(self.left_rect.x + sel_col * SCALED_TILE,
                               self.left_rect.y + sel_row * SCALED_TILE,
                               SCALED_TILE, SCALED_TILE)
        pygame.draw.rect(self.screen, SELECT_COLOR, sel_rect, 2)

        # Draw right side (output)
        for idx, tile_data in enumerate(self.right_tiles):
            col = idx % OUTPUT_COLS
            row = idx // OUTPUT_COLS
            x = self.right_rect.x + col * SCALED_TILE
            y = self.right_rect.y + row * SCALED_TILE

            if tile_data is None:
                pygame.draw.rect(self.screen, EMPTY_TILE_COLOR, (x, y, SCALED_TILE, SCALED_TILE))
            else:
                tile = self.get_transformed_tile(tile_data)
                self.screen.blit(tile, (x, y))

        # Draw right grid
        for x in range(0, self.right_width + 1, SCALED_TILE):
            pygame.draw.line(self.screen, GRID_COLOR,
                           (self.right_rect.x + x, self.right_rect.y),
                           (self.right_rect.x + x, self.right_rect.y + self.right_height))
        for y in range(0, self.right_height + 1, SCALED_TILE):
            pygame.draw.line(self.screen, GRID_COLOR,
                           (self.right_rect.x, self.right_rect.y + y),
                           (self.right_rect.x + self.right_width, self.right_rect.y + y))

        # Hover highlight on right
        if self.right_rect.collidepoint(mouse_pos):
            rel_x = mouse_pos[0] - self.right_rect.x
            rel_y = mouse_pos[1] - self.right_rect.y
            col = rel_x // SCALED_TILE
            row = rel_y // SCALED_TILE
            if 0 <= col < OUTPUT_COLS and 0 <= row < OUTPUT_ROWS:
                hover_rect = pygame.Rect(self.right_rect.x + col * SCALED_TILE,
                                        self.right_rect.y + row * SCALED_TILE,
                                        SCALED_TILE, SCALED_TILE)
                pygame.draw.rect(self.screen, HOVER_COLOR, hover_rect, 2)

        # Selected tile info
        text = self.small_font.render(f"Selected: {self.selected_tile} ({sel_col},{sel_row})", True, TEXT_COLOR)
        self.screen.blit(text, (10, label_y + 25))

        # Help
        help_text = "Alt+Click: 2x2 block | Right-click: Delete | R: Rotate | H/V: Flip"
        text = self.small_font.render(help_text, True, (150, 150, 150))
        self.screen.blit(text, (10, self.window_height - 25))

    def new_tileset(self):
        """Clear right side"""
        self.right_tiles = [None] * OUTPUT_TOTAL
        self.current_file = None
        self.unsaved = False

    def save_tileset(self):
        """Save right side as PNG"""
        if not self.current_file:
            try:
                import tkinter as tk
                from tkinter import filedialog
                root = tk.Tk()
                root.withdraw()
                filename = filedialog.asksaveasfilename(
                    defaultextension=".png",
                    filetypes=[("PNG files", "*.png"), ("All files", "*.*")],
                    initialdir=os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'art')
                )
                root.destroy()
                if not filename:
                    return
                self.current_file = filename
            except:
                script_dir = os.path.dirname(os.path.abspath(__file__))
                self.current_file = os.path.join(script_dir, '..', 'art', 'tileset_output.png')

        # Create output image at original resolution (128x128 = 16x16 tiles at 8x8)
        output_w = OUTPUT_COLS * self.tile_src_size
        output_h = OUTPUT_ROWS * self.tile_src_size
        output = pygame.Surface((output_w, output_h), pygame.SRCALPHA)
        output.fill((0, 0, 0, 0))  # Transparent background

        for idx, tile_data in enumerate(self.right_tiles):
            if tile_data is not None:
                tile_idx, rotation, h_flip, v_flip = tile_data
                col = idx % OUTPUT_COLS
                row = idx // OUTPUT_COLS

                # Get original size tile and transform
                tile = self.source_tiles[tile_idx].copy()
                if h_flip:
                    tile = pygame.transform.flip(tile, True, False)
                if v_flip:
                    tile = pygame.transform.flip(tile, False, True)
                if rotation != 0:
                    tile = pygame.transform.rotate(tile, -rotation)

                x = col * self.tile_src_size
                y = row * self.tile_src_size
                output.blit(tile, (x, y))

        pygame.image.save(output, self.current_file)
        self.unsaved = False
        print(f"Saved to {self.current_file}")

    def load_tileset(self):
        """Load an existing tileset PNG (maps colors back to source tiles)"""
        # For now, just clear - proper loading would need color matching
        print("Load not fully implemented - use New and rebuild")


def main():
    editor = TilesetEditor()
    editor.run()


if __name__ == '__main__':
    main()
