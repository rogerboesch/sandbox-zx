#include <arch/zxn.h>
#include <z80.h>
#include <intrinsic.h>
#include <stdint.h>
#include <string.h>

// Tilemap registers
#define REG_TILEMAP_CTRL     0x6B
#define REG_TILEMAP_ATTR     0x6C
#define REG_TILEMAP_BASE     0x6E
#define REG_TILEMAP_TILES    0x6F
#define REG_TILEMAP_TRANS    0x4C
#define REG_TILEMAP_YSCROLL  0x31

// Memory addresses
#define TILEMAP_ADDR    0x6000
#define TILES_ADDR      0x6600
#define TILE_SIZE       32
#define TILEMAP_WIDTH   40

// Simple 3x5 digit font (each digit is 3 pixels wide, stored as 5 bytes)
static const uint8_t digits[10][5] = {
    {0x7, 0x5, 0x5, 0x5, 0x7},  // 0
    {0x2, 0x6, 0x2, 0x2, 0x7},  // 1
    {0x7, 0x1, 0x7, 0x4, 0x7},  // 2
    {0x7, 0x1, 0x7, 0x1, 0x7},  // 3
    {0x5, 0x5, 0x7, 0x1, 0x1},  // 4
    {0x7, 0x4, 0x7, 0x1, 0x7},  // 5
    {0x7, 0x4, 0x7, 0x5, 0x7},  // 6
    {0x7, 0x1, 0x1, 0x1, 0x1},  // 7
    {0x7, 0x5, 0x7, 0x5, 0x7},  // 8
    {0x7, 0x5, 0x7, 0x1, 0x7},  // 9
};

// Create a numbered tile (8x8, 4-bit) with number 1-64
static void make_number_tile(uint8_t *dest, uint8_t num) {
    uint8_t y;
    uint8_t tens = num / 10;
    uint8_t ones = num % 10;

    // Clear tile to black (color 0)
    for (y = 0; y < TILE_SIZE; y++) {
        dest[y] = 0x00;
    }

    // Draw digits in white (color 15)
    for (y = 0; y < 5; y++) {
        uint8_t row_data = 0;

        // Tens digit (if non-zero) at x=0-2
        if (tens > 0) {
            uint8_t d = digits[tens][y];
            if (d & 0x4) row_data |= 0xF0;  // x=0
            if (d & 0x2) row_data |= 0x0F;  // x=1
        }
        dest[y * 4] = row_data;

        row_data = 0;
        if (tens > 0) {
            uint8_t d = digits[tens][y];
            if (d & 0x1) row_data |= 0xF0;  // x=2
        }
        // Ones digit at x=4-6
        {
            uint8_t d = digits[ones][y];
            if (d & 0x4) row_data |= 0x0F;  // x=4
        }
        dest[y * 4 + 1] = row_data;

        row_data = 0;
        {
            uint8_t d = digits[ones][y];
            if (d & 0x2) row_data |= 0xF0;  // x=5
            if (d & 0x1) row_data |= 0x0F;  // x=6
        }
        dest[y * 4 + 2] = row_data;

        // x=7 empty
        dest[y * 4 + 3] = 0x00;
    }
}

// Set up palette
static void setup_palette(void) {
    ZXN_NEXTREG(0x43, 0x30);  // Tilemap palette
    ZXN_NEXTREG(0x40, 0);     // Start at index 0

    IO_NEXTREG_REG = 0x41;
    IO_NEXTREG_DAT = 0x00;  // 0: Black
    IO_NEXTREG_DAT = 0x03;  // 1: Blue
    IO_NEXTREG_DAT = 0xE0;  // 2: Red
    IO_NEXTREG_DAT = 0xFC;  // 3: Yellow
    IO_NEXTREG_DAT = 0x1C;  // 4: Green
    IO_NEXTREG_DAT = 0x1F;  // 5: Cyan
    IO_NEXTREG_DAT = 0xE3;  // 6: Magenta
    IO_NEXTREG_DAT = 0xFF;  // 7: White
    // Fill rest with white for digit display
    {
        uint8_t i;
        for (i = 8; i < 16; i++) {
            IO_NEXTREG_DAT = 0xFF;
        }
    }

    ZXN_NEXTREG(0x43, 0x00);
}

int main(void) {
    uint8_t tile_data[TILE_SIZE];
    uint8_t i;
    int16_t scroll_y = 0;

    intrinsic_ei();
    ZXN_NEXTREG(0x07, 0x02);  // 14MHz
    z80_outp(0xFE, 0x00);     // Black border
    ZXN_NEXTREG(0x14, 0xFF);  // Global transparency

    // Create 64 numbered tiles (tiles 0-63)
    for (i = 0; i < 64; i++) {
        make_number_tile(tile_data, i + 1);  // Numbers 1-64
        memcpy((uint8_t *)(TILES_ADDR + (i+1) * TILE_SIZE), tile_data, TILE_SIZE); // MARK: Since tile 0 is the transparenet one i move one up
    }

    setup_palette();

    // Set tilemap addresses
    ZXN_NEXTREG(REG_TILEMAP_BASE, 0x20);   // 0x6000
    ZXN_NEXTREG(REG_TILEMAP_TILES, 0x26);  // 0x6600
    ZXN_NEXTREG(REG_TILEMAP_ATTR, 0x00);
    ZXN_NEXTREG(REG_TILEMAP_TRANS, 0x00);  // Index 0 (black) is transparent

    // Fill tilemap: put all 32 numbered tiles to see where each row appears
    {
        uint8_t *tmap = (uint8_t *)TILEMAP_ADDR;
        uint16_t pos;
        uint8_t row;

        // Clear all to tile 0 (transparent)
        for (pos = 0; pos < 40 * 32; pos++) {
            tmap[pos] = 0;
        }

        tmap[20] = 1; // first tile must always be here
        tmap[31 * TILEMAP_WIDTH + 20] = 2; // to garantee correct "fill up", the next must be placed here at first, creates an overlap on screen but lets solve that later

        // Put tile showing row number at each row
        // Row 0 gets tile 1 (showing "1"), row 1 gets tile 2 (showing "2"), etc.
        //for (row = 0; row < 32; row++) {
        //    tmap[row * TILEMAP_WIDTH + 20] = row + 1;
        //}
    }

    // Make tile 0 transparent
    {
        uint8_t *tile0 = (uint8_t *)TILES_ADDR;
        uint8_t j;
        for (j = 0; j < TILE_SIZE; j++) {
            tile0[j] = 0x00;
        }
    }

    // Fill ULA with dark blue so we can see tilemap area
    {
        uint8_t *attr = (uint8_t *)0x5800;
        uint16_t pos;
        for (pos = 0; pos < 768; pos++) {
            attr[pos] = 0x08;  // Blue paper
        }
    }

    // Set tilemap clip window to ULA visible area + half border
    // Register 0x1B: X1, X2, Y1, Y2 (written sequentially)
    // ULA is 256x192, tilemap is 320x256
    // Vertical: tilemap has 4 tiles (32px) above and below ULA
    // Include half border: skip 16px (2 tiles) instead of 32px (4 tiles)
    // Y1=16, Y2=239 (192 + 16 + 16 + 15 = 239)
    ZXN_NEXTREG(0x1B, 0);     // X1 - start from left
    ZXN_NEXTREG(0x1B, 255);   // X2 - to right edge
    ZXN_NEXTREG(0x1B, 16);    // Y1 - skip top 2 rows (16 pixels), show 2 rows in border
    ZXN_NEXTREG(0x1B, 239);   // Y2 - show 2 rows in bottom border (256 - 16 - 1)

    // Enable tilemap
    ZXN_NEXTREG(REG_TILEMAP_CTRL, 0xA0);
    ZXN_NEXTREG(0x15, 0x08);  // Tilemap visible

    // Scroll loop - scroll pixel by pixel, add new tiles as they scroll in
    //
    // scroll_y-- makes content move DOWN on screen
    // When scroll_y crosses -8, -16, -24, etc. a new row appears at top
    //
    // At scroll_y = 0: row 0 at top (tile 1)
    // At scroll_y = -8: row 31 at top (tile 2 already placed there)
    // At scroll_y = -16: row 30 at top (need to place tile 3)
    // At scroll_y = -24: row 29 at top (need to place tile 4)
    // etc.
    //
    // Row at top = (32 + scroll_y/8) & 0x1F = (32 - abs(scroll_y)/8) & 0x1F
    {
        uint8_t last_space = 0;
        uint8_t last_r = 0;
        uint8_t next_tile_num = 3;  // Tiles 1 and 2 already placed
        uint8_t *tmap = (uint8_t *)TILEMAP_ADDR;

        IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
        IO_NEXTREG_DAT = 0;

        while (1) {
            uint8_t space_pressed;
            uint8_t r_pressed;

            intrinsic_halt();

            // Read space key (port 0x7FFE, bit 0)
            space_pressed = (z80_inp(0x7FFE) & 0x01) == 0;

            // Read R key (port 0xFBFE, bit 3)
            r_pressed = (z80_inp(0xFBFE) & 0x08) == 0;

            // Scroll on space press (with debounce)
            if (space_pressed && !last_space) {
                uint8_t old_top_row, new_top_row;

                // Calculate row at top before scroll
                // scroll_y is 0 or negative, & 0xFF gives hardware value
                // Row at top = ((scroll_y & 0xFF) / 8) & 0x1F... but that's wrong for negative
                // Actually: row at top when scroll=-N is row (32 - N/8) & 0x1F = (256 - N) / 8 & 0x1F
                old_top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);

                scroll_y--;  // Content moves DOWN

                new_top_row = (uint8_t)(((256 + scroll_y) / 8) & 0x1F);

                // Set hardware scroll
                IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
                IO_NEXTREG_DAT = (uint8_t)(scroll_y & 0xFF);

                // When we cross a row boundary, place next tile at the new top row
                // Skip only row 31 on first pass (tile 2 is pre-placed there)
                if (new_top_row != old_top_row && next_tile_num <= 64) {
                    // Skip row 31 only when next_tile_num is 3 (first row crossing)
                    if (new_top_row == 31 && next_tile_num == 3) {
                        // Tile 2 already there, don't overwrite, don't increment
                    } else {
                        tmap[new_top_row * TILEMAP_WIDTH + 20] = next_tile_num;
                        next_tile_num++;
                    }
                }
            }

            // Reset on R press (with debounce)
            if (r_pressed && !last_r) {
                uint16_t pos;

                scroll_y = 0;
                next_tile_num = 3;

                // Clear tilemap
                for (pos = 0; pos < 40 * 32; pos++) {
                    tmap[pos] = 0;
                }

                // Restore initial tiles
                tmap[20] = 1;  // Tile 1 at row 0
                tmap[31 * TILEMAP_WIDTH + 20] = 2;  // Tile 2 at row 31

                // Set hardware scroll
                IO_NEXTREG_REG = REG_TILEMAP_YSCROLL;
                IO_NEXTREG_DAT = 0;
            }

            last_space = space_pressed;
            last_r = r_pressed;
        }
    }

    return 0;
}
