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
# 8x8 tiles (default)
python3 ../tools/tilemap_grid.py tileset.png tileset_grid.png

# Sprites - 16x16
python3 ../tools/tilemap_grid.py spriteset.png spriteset_grid.png 16

# Create header files
python3 ../tools/tiles_to_header.py tileset.png tile 7
python3 ../tools/tiles_to_header.py spriteset.png sprite 1