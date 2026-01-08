#!/bin/bash
#
# Prerequisites
#
# venv:
#  python3 -m venv ~/venv
#  source ~/venv/bin/activate
#
# modules:
#  python3 -m pip install pygame
#  python3 -m pip install Pillow
#  python3 -m pip install PyYAML
#
# tile editor: tools/sprite_editor.py art/spriteset.png 16
#

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
ART_DIR="$SCRIPT_DIR"
TOOLS_DIR="$PROJECT_DIR/tools"

# 8x8 tiles (default)
python3 "$TOOLS_DIR/tilemap_grid.py" "$ART_DIR/tileset.png" "$ART_DIR/tileset_grid.png"

# Sprites - 16x16
python3 "$TOOLS_DIR/tilemap_grid.py" "$ART_DIR/spriteset.png" "$ART_DIR/spriteset_grid.png" 16

# Create header files
python3 "$TOOLS_DIR/tiles_to_header.py" "$ART_DIR/tileset.png" tile 7
python3 "$TOOLS_DIR/tiles_to_header.py" "$ART_DIR/spriteset.png" sprite 2