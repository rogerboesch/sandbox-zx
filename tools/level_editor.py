#!/usr/bin/env python3
"""
Nebula 8 Level Editor (Pygame version)
- Left: Scrollable tilemap canvas (40 tiles wide, endless height)
- Right: Tile palette from tilemap.png (16x16 tiles)
- Place, delete, move tiles
- Save/Load as YAML

Controls:
- Left click: Place tile / Select from palette
- Option/Alt + Left click: Place 2x2 block (selected tile + right + 2 below)
- Right click: Delete tile
- Arrow Up/Down: Scroll level (bottom-up, row 0 at bottom)
- Page Up/Down: Fast scroll
- Home/End: Jump to start/end
- 1/2/3: Switch tools (Place/Delete/Move)
- Ctrl+S: Save
- Ctrl+O: Load
- Ctrl+N: New level
"""

import pygame
import yaml
import os
import sys

# Constants
TILE_SIZE = 8
DISPLAY_SCALE = 2  # Scale up for visibility
SCALED_TILE = TILE_SIZE * DISPLAY_SCALE

TILEMAP_WIDTH = 40  # tiles
PALETTE_COLS = 16
PALETTE_ROWS = 16
CANVAS_VISIBLE_ROWS = 40  # visible rows in editor

# Colors
BG_COLOR = (20, 20, 30)
GRID_COLOR = (40, 40, 50)
SELECT_COLOR = (255, 255, 0)
HIGHLIGHT_COLOR = (100, 100, 255)
TEXT_COLOR = (200, 200, 200)
BUTTON_COLOR = (60, 60, 80)
BUTTON_HOVER = (80, 80, 100)
BUTTON_ACTIVE = (100, 100, 150)


class LevelEditor:
    def __init__(self):
        pygame.init()
        pygame.display.set_caption("Nebula 8 Level Editor")

        # Calculate window size
        self.canvas_width = TILEMAP_WIDTH * SCALED_TILE
        self.canvas_height = CANVAS_VISIBLE_ROWS * SCALED_TILE
        self.palette_width = PALETTE_COLS * SCALED_TILE
        self.palette_height = PALETTE_ROWS * SCALED_TILE
        self.sidebar_width = self.palette_width + 40
        self.toolbar_height = 60

        self.window_width = self.canvas_width + self.sidebar_width
        self.window_height = self.canvas_height + self.toolbar_height

        self.screen = pygame.display.set_mode((self.window_width, self.window_height))
        self.clock = pygame.time.Clock()
        self.font = pygame.font.Font(None, 24)
        self.small_font = pygame.font.Font(None, 20)

        # Level data
        self.level_data = {}  # (x, y) -> tile_index
        self.level_height = 100

        # Editor state
        self.selected_tile = 0
        self.scroll_y = 0
        self.tool_mode = 'place'  # 'place', 'delete', 'move'
        self.move_start = None
        self.current_file = None
        self.unsaved = False

        # UI regions
        self.canvas_rect = pygame.Rect(0, self.toolbar_height, self.canvas_width, self.canvas_height)
        self.palette_rect = pygame.Rect(self.canvas_width + 20, self.toolbar_height + 100,
                                         self.palette_width, self.palette_height)

        # Load tilemap
        self.load_tilemap()

        # Buttons
        self.buttons = []
        self.create_buttons()

    def load_tilemap(self):
        """Load tilemap.png and slice into tiles"""
        script_dir = os.path.dirname(os.path.abspath(__file__))
        tilemap_path = os.path.join(script_dir, '..', 'art', 'tilemap.png')

        try:
            self.tilemap_img = pygame.image.load(tilemap_path).convert_alpha()
        except pygame.error as e:
            print(f"Cannot find tilemap.png at {tilemap_path}: {e}")
            sys.exit(1)

        # Calculate tile size from image
        img_w, img_h = self.tilemap_img.get_size()
        self.tile_src_size = img_w // PALETTE_COLS

        # Extract individual tiles
        self.tiles = []
        self.tiles_scaled = []

        for row in range(PALETTE_ROWS):
            for col in range(PALETTE_COLS):
                x = col * self.tile_src_size
                y = row * self.tile_src_size
                tile = self.tilemap_img.subsurface((x, y, self.tile_src_size, self.tile_src_size))

                # Original size for level canvas
                if self.tile_src_size != TILE_SIZE:
                    tile_small = pygame.transform.scale(tile, (TILE_SIZE, TILE_SIZE))
                else:
                    tile_small = tile.copy()
                self.tiles.append(tile_small)

                # Scaled for display
                tile_scaled = pygame.transform.scale(tile, (SCALED_TILE, SCALED_TILE))
                self.tiles_scaled.append(tile_scaled)

    def create_buttons(self):
        """Create UI buttons"""
        y = 10
        x = 10

        self.buttons = [
            {'rect': pygame.Rect(x, y, 60, 30), 'text': 'New', 'action': self.new_level},
            {'rect': pygame.Rect(x + 70, y, 60, 30), 'text': 'Load', 'action': self.load_level},
            {'rect': pygame.Rect(x + 140, y, 60, 30), 'text': 'Save', 'action': self.save_level},
            {'rect': pygame.Rect(x + 220, y, 80, 30), 'text': 'Place [1]', 'action': lambda: self.set_tool('place'), 'tool': 'place'},
            {'rect': pygame.Rect(x + 310, y, 80, 30), 'text': 'Delete [2]', 'action': lambda: self.set_tool('delete'), 'tool': 'delete'},
            {'rect': pygame.Rect(x + 400, y, 80, 30), 'text': 'Move [3]', 'action': lambda: self.set_tool('move'), 'tool': 'move'},
        ]

    def set_tool(self, tool):
        self.tool_mode = tool
        self.move_start = None

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

        if event.key == pygame.K_UP:
            self.scroll(-1)
        elif event.key == pygame.K_DOWN:
            self.scroll(1)
        elif event.key == pygame.K_PAGEUP:
            self.scroll(-10)
        elif event.key == pygame.K_PAGEDOWN:
            self.scroll(10)
        elif event.key == pygame.K_HOME:
            self.scroll_y = 0
        elif event.key == pygame.K_END:
            self.scroll_y = max(0, self.level_height - CANVAS_VISIBLE_ROWS)
        elif event.key == pygame.K_1:
            self.set_tool('place')
        elif event.key == pygame.K_2:
            self.set_tool('delete')
        elif event.key == pygame.K_3:
            self.set_tool('move')
        elif event.key == pygame.K_s and (mods & pygame.KMOD_CTRL or mods & pygame.KMOD_META):
            self.save_level()
        elif event.key == pygame.K_o and (mods & pygame.KMOD_CTRL or mods & pygame.KMOD_META):
            self.load_level()
        elif event.key == pygame.K_n and (mods & pygame.KMOD_CTRL or mods & pygame.KMOD_META):
            self.new_level()

    def handle_click(self, event):
        """Handle mouse click"""
        pos = event.pos

        # Check buttons
        for btn in self.buttons:
            if btn['rect'].collidepoint(pos):
                btn['action']()
                return

        # Check palette
        if self.palette_rect.collidepoint(pos):
            rel_x = pos[0] - self.palette_rect.x
            rel_y = pos[1] - self.palette_rect.y
            col = rel_x // SCALED_TILE
            row = rel_y // SCALED_TILE
            if 0 <= col < PALETTE_COLS and 0 <= row < PALETTE_ROWS:
                self.selected_tile = row * PALETTE_COLS + col
            return

        # Check canvas
        if self.canvas_rect.collidepoint(pos):
            self.handle_canvas_click(pos, event.button)

    def handle_canvas_click(self, pos, button):
        """Handle click on level canvas"""
        rel_x = pos[0] - self.canvas_rect.x
        rel_y = pos[1] - self.canvas_rect.y
        tx = rel_x // SCALED_TILE
        # Bottom-up: row 0 at bottom, so invert Y
        screen_row = rel_y // SCALED_TILE
        ty = self.scroll_y + (CANVAS_VISIBLE_ROWS - 1 - screen_row)

        if tx < 0 or tx >= TILEMAP_WIDTH or ty < 0:
            return

        if button == 3:  # Right click always deletes
            if (tx, ty) in self.level_data:
                del self.level_data[(tx, ty)]
                self.unsaved = True
            return

        if self.tool_mode == 'place':
            mods = pygame.key.get_mods()
            if mods & pygame.KMOD_ALT:
                # Place 2x2 block: selected, right, and 2 tiles below in palette
                self.place_block(tx, ty)
            else:
                self.level_data[(tx, ty)] = self.selected_tile
            self.expand_height(ty)
            self.unsaved = True

        elif self.tool_mode == 'delete':
            if (tx, ty) in self.level_data:
                del self.level_data[(tx, ty)]
                self.unsaved = True

        elif self.tool_mode == 'move':
            if self.move_start is None:
                if (tx, ty) in self.level_data:
                    self.move_start = (tx, ty)
            else:
                tile_idx = self.level_data.get(self.move_start)
                if tile_idx is not None:
                    del self.level_data[self.move_start]
                    self.level_data[(tx, ty)] = tile_idx
                    self.expand_height(ty)
                    self.unsaved = True
                self.move_start = None

    def place_block(self, tx, ty):
        """Place a 2x2 block (selected tile + right + 2 below in palette)"""
        base = self.selected_tile
        # Top-left: selected tile
        self.level_data[(tx, ty)] = base
        # Top-right: tile to the right in palette
        if (base % PALETTE_COLS) < PALETTE_COLS - 1:
            self.level_data[(tx + 1, ty)] = base + 1
        # Bottom-left: tile below in palette (next row)
        if (base // PALETTE_COLS) < PALETTE_ROWS - 1:
            self.level_data[(tx, ty - 1)] = base + PALETTE_COLS
        # Bottom-right: tile below-right in palette
        if (base % PALETTE_COLS) < PALETTE_COLS - 1 and (base // PALETTE_COLS) < PALETTE_ROWS - 1:
            self.level_data[(tx + 1, ty - 1)] = base + PALETTE_COLS + 1

    def handle_drag(self, event):
        """Handle mouse drag for painting"""
        if self.tool_mode != 'place':
            return

        pos = event.pos
        if self.canvas_rect.collidepoint(pos):
            rel_x = pos[0] - self.canvas_rect.x
            rel_y = pos[1] - self.canvas_rect.y
            tx = rel_x // SCALED_TILE
            # Bottom-up: row 0 at bottom
            screen_row = rel_y // SCALED_TILE
            ty = self.scroll_y + (CANVAS_VISIBLE_ROWS - 1 - screen_row)

            if 0 <= tx < TILEMAP_WIDTH and ty >= 0:
                mods = pygame.key.get_mods()
                if mods & pygame.KMOD_ALT:
                    self.place_block(tx, ty)
                else:
                    self.level_data[(tx, ty)] = self.selected_tile
                self.expand_height(ty)
                self.unsaved = True

    def scroll(self, delta):
        """Scroll the canvas"""
        self.scroll_y = max(0, min(self.scroll_y + delta,
                                   self.level_height - CANVAS_VISIBLE_ROWS))

    def expand_height(self, y):
        """Expand level height if needed"""
        if y >= self.level_height - 10:
            self.level_height = y + 50

    def draw(self):
        """Draw everything"""
        self.screen.fill(BG_COLOR)

        # Draw toolbar
        pygame.draw.rect(self.screen, (30, 30, 40), (0, 0, self.window_width, self.toolbar_height))

        # Draw buttons
        mouse_pos = pygame.mouse.get_pos()
        for btn in self.buttons:
            color = BUTTON_COLOR
            if btn['rect'].collidepoint(mouse_pos):
                color = BUTTON_HOVER
            if btn.get('tool') == self.tool_mode:
                color = BUTTON_ACTIVE
            pygame.draw.rect(self.screen, color, btn['rect'], border_radius=4)
            pygame.draw.rect(self.screen, (100, 100, 120), btn['rect'], 1, border_radius=4)
            text = self.small_font.render(btn['text'], True, TEXT_COLOR)
            text_rect = text.get_rect(center=btn['rect'].center)
            self.screen.blit(text, text_rect)

        # Draw file info
        file_text = f"File: {os.path.basename(self.current_file) if self.current_file else '(unsaved)'}"
        if self.unsaved:
            file_text += " *"
        text = self.small_font.render(file_text, True, TEXT_COLOR)
        self.screen.blit(text, (500, 15))

        # Draw canvas grid
        for x in range(0, self.canvas_width + 1, SCALED_TILE):
            pygame.draw.line(self.screen, GRID_COLOR,
                           (x, self.toolbar_height), (x, self.toolbar_height + self.canvas_height))
        for y in range(0, self.canvas_height + 1, SCALED_TILE):
            pygame.draw.line(self.screen, GRID_COLOR,
                           (0, self.toolbar_height + y), (self.canvas_width, self.toolbar_height + y))

        # Draw tiles on canvas (bottom-up: row 0 at bottom)
        for (tx, ty), tile_idx in self.level_data.items():
            # Convert tile Y to screen Y (inverted)
            rel_y = ty - self.scroll_y
            if 0 <= rel_y < CANVAS_VISIBLE_ROWS:
                screen_y = (CANVAS_VISIBLE_ROWS - 1 - rel_y) * SCALED_TILE
                self.screen.blit(self.tiles_scaled[tile_idx],
                               (tx * SCALED_TILE, self.toolbar_height + screen_y))

        # Highlight move source
        if self.move_start:
            mx, my = self.move_start
            rel_y = my - self.scroll_y
            if 0 <= rel_y < CANVAS_VISIBLE_ROWS:
                screen_y = (CANVAS_VISIBLE_ROWS - 1 - rel_y) * SCALED_TILE
                rect = pygame.Rect(mx * SCALED_TILE, self.toolbar_height + screen_y,
                                  SCALED_TILE, SCALED_TILE)
                pygame.draw.rect(self.screen, HIGHLIGHT_COLOR, rect, 2)

        # Draw sidebar
        sidebar_x = self.canvas_width + 10

        # Scroll info
        text = self.font.render(f"Row: {self.scroll_y}-{self.scroll_y + CANVAS_VISIBLE_ROWS}", True, TEXT_COLOR)
        self.screen.blit(text, (sidebar_x, self.toolbar_height + 10))

        text = self.font.render(f"Height: {self.level_height}", True, TEXT_COLOR)
        self.screen.blit(text, (sidebar_x, self.toolbar_height + 35))

        text = self.font.render(f"Tiles: {len(self.level_data)}", True, TEXT_COLOR)
        self.screen.blit(text, (sidebar_x, self.toolbar_height + 60))

        # Palette label
        text = self.small_font.render("Tile Palette:", True, TEXT_COLOR)
        self.screen.blit(text, (sidebar_x, self.toolbar_height + 85))

        # Draw palette
        for idx, tile in enumerate(self.tiles_scaled):
            col = idx % PALETTE_COLS
            row = idx // PALETTE_COLS
            x = self.palette_rect.x + col * SCALED_TILE
            y = self.palette_rect.y + row * SCALED_TILE
            self.screen.blit(tile, (x, y))

        # Palette selection highlight
        sel_col = self.selected_tile % PALETTE_COLS
        sel_row = self.selected_tile // PALETTE_COLS
        sel_rect = pygame.Rect(self.palette_rect.x + sel_col * SCALED_TILE,
                               self.palette_rect.y + sel_row * SCALED_TILE,
                               SCALED_TILE, SCALED_TILE)
        pygame.draw.rect(self.screen, SELECT_COLOR, sel_rect, 2)

        # Selected tile info
        text = self.small_font.render(f"Selected: {self.selected_tile}", True, TEXT_COLOR)
        self.screen.blit(text, (sidebar_x, self.palette_rect.bottom + 10))

        # Controls help
        help_y = self.palette_rect.bottom + 40
        help_lines = [
            "Controls:",
            "Arrows: Scroll",
            "PgUp/Dn: Fast scroll",
            "1/2/3: Tools",
            "Right-click: Delete",
            "Ctrl+S/O/N: Save/Open/New"
        ]
        for i, line in enumerate(help_lines):
            text = self.small_font.render(line, True, (150, 150, 150))
            self.screen.blit(text, (sidebar_x, help_y + i * 18))

    def new_level(self):
        """Create new empty level"""
        self.level_data = {}
        self.level_height = 100
        self.scroll_y = 0
        self.current_file = None
        self.unsaved = False

    def save_level(self):
        """Save level to YAML"""
        if not self.current_file:
            # Simple file dialog using input (pygame doesn't have native dialogs)
            try:
                import tkinter as tk
                from tkinter import filedialog
                root = tk.Tk()
                root.withdraw()
                filename = filedialog.asksaveasfilename(
                    defaultextension=".yaml",
                    filetypes=[("YAML files", "*.yaml"), ("All files", "*.*")],
                    initialdir=os.path.dirname(os.path.abspath(__file__))
                )
                root.destroy()
                if not filename:
                    return
                self.current_file = filename
            except:
                # Fallback: save to default location
                script_dir = os.path.dirname(os.path.abspath(__file__))
                self.current_file = os.path.join(script_dir, '..', 'levels', 'level.yaml')
                os.makedirs(os.path.dirname(self.current_file), exist_ok=True)

        data = {
            'width': TILEMAP_WIDTH,
            'height': self.level_height,
            'tiles': {f"{x},{y}": tile_idx for (x, y), tile_idx in self.level_data.items()}
        }

        with open(self.current_file, 'w') as f:
            yaml.dump(data, f, default_flow_style=False)

        self.unsaved = False
        print(f"Saved to {self.current_file}")

    def load_level(self):
        """Load level from YAML"""
        try:
            import tkinter as tk
            from tkinter import filedialog
            root = tk.Tk()
            root.withdraw()
            filename = filedialog.askopenfilename(
                filetypes=[("YAML files", "*.yaml"), ("All files", "*.*")],
                initialdir=os.path.dirname(os.path.abspath(__file__))
            )
            root.destroy()
            if not filename:
                return
        except:
            return

        try:
            with open(filename, 'r') as f:
                data = yaml.safe_load(f)

            self.level_height = data.get('height', 100)
            self.level_data = {}
            for key, tile_idx in data.get('tiles', {}).items():
                x, y = map(int, key.split(','))
                self.level_data[(x, y)] = tile_idx

            self.scroll_y = 0
            self.current_file = filename
            self.unsaved = False
            print(f"Loaded {filename}")
        except Exception as e:
            print(f"Error loading: {e}")


def main():
    editor = LevelEditor()
    editor.run()


if __name__ == '__main__':
    main()
