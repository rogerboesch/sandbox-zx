# Dark Nebula - ZX Spectrum Next Prototype
# Build with z88dk

COMPILER = zcc
TARGET = +zxn
CFLAGS = -vn -SO3 -clib=sdcc_iy -startup=31
OUTPUT = dark_nebula
BIN_DIR = bin

# Source files
SRCS = src/main.c src/sprites.c src/game.c src/tiles.c

# Default target - creates NEX file for ZX Spectrum Next
all: $(BIN_DIR)/$(OUTPUT).nex

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Build NEX file (native ZX Spectrum Next format)
$(BIN_DIR)/$(OUTPUT).nex: $(SRCS) src/game.h src/tiles.h | $(BIN_DIR)
	$(COMPILER) $(TARGET) $(CFLAGS) $(SRCS) -o $(BIN_DIR)/$(OUTPUT) -create-app -subtype=nex

# Build TAP file (compatible with emulators)
tap: $(SRCS) src/game.h | $(BIN_DIR)
	$(COMPILER) $(TARGET) $(CFLAGS) $(SRCS) -o $(BIN_DIR)/$(OUTPUT) -create-app -subtype=tap

# Build SNA snapshot
sna: $(SRCS) src/game.h | $(BIN_DIR)
	$(COMPILER) $(TARGET) $(CFLAGS) $(SRCS) -o $(BIN_DIR)/$(OUTPUT) -create-app -subtype=sna

# Simple test program
test: src/test_simple.c | $(BIN_DIR)
	$(COMPILER) $(TARGET) $(CFLAGS) src/test_simple.c -o $(BIN_DIR)/test -create-app -subtype=nex

# Clean build artifacts
clean:
	rm -rf $(BIN_DIR)
	rm -f *.lis *.sym *.map *.o

# Run instructions
run: $(BIN_DIR)/$(OUTPUT).nex
	@echo ""
	@echo "=== Dark Nebula - ZX Spectrum Next ==="
	@echo ""
	@echo "To run the game:"
	@echo "1. Copy $(BIN_DIR)/$(OUTPUT).nex to your ZX Spectrum Next SD card"
	@echo "2. Or load in CSpect/ZEsarUX emulator"
	@echo ""
	@echo "Controls:"
	@echo "  Q = Up, A = Down"
	@echo "  O = Left, P = Right"
	@echo "  Space = Fire"
	@echo "  Kempston joystick also supported"
	@echo ""

# CSpect emulator path
CSPECT_DIR = /Applications/CSpect3_0_15_2
CSPECT = $(CSPECT_DIR)/CSpect.exe

# Start game in CSpect emulator
start: $(BIN_DIR)/$(OUTPUT).nex
	cd $(CSPECT_DIR) && mono $(CSPECT) -w4 -vsync -s28 -tv -basickeys -zxnext -nextrom "$(CURDIR)/$(BIN_DIR)/$(OUTPUT).nex"

.PHONY: all clean run tap sna test start
