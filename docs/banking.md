# Z88DK Banking for ZX Spectrum Next

## Overview

This document covers how to store large data (like images) in separate memory banks using z88dk for the ZX Spectrum Next.

## PNG to Working Binary: Step-by-Step

### 1. Create/Edit the Image

Use `tools/layer2_editor.py` to create or edit the image:

```bash
~/venv/bin/python3 tools/layer2_editor.py art/border.png
```

- Image height must be exactly 192 pixels
- Image width can be 1-256 pixels
- Save with Shift+S

### 2. Convert PNG to Assembly

Use `tools/png_to_asm.py` to convert the PNG directly to assembly:

```bash
~/venv/bin/python3 tools/png_to_asm.py art/border.png 40 src/border_data.asm
```

Parameters:
- `art/border.png` - Source PNG file
- `40` - Starting page number (8K pages)
- `src/border_data.asm` - Output assembly file

The script will:
- Validate image height is 192 pixels
- Convert to RGB332 format
- Split into 8K pages automatically
- Generate defb statements

### 3. Add to Makefile

```makefile
ASMS = src/border_data.asm
```

### 4. Add Extern Declarations in C

```c
extern uint8_t border_page40;
extern uint8_t border_page41;

void force_include(void) {
    volatile uint8_t *ptr = &border_page40;
    ptr = &border_page41;
    (void)ptr;
}
```

### 5. Build

```bash
make
```

Verify output shows bank usage:
```
Bank 20, 4864 tail bytes free
```

---

## Key Concepts

### Section Naming

For 8K pages, use `SECTION PAGE_N`:

- `PAGE_N` = 8K page (N = 0-223)
- Two consecutive pages (PAGE_40, PAGE_41) = 16K bank 20

### MMU Slot Selection

**CRITICAL**: Use slot 3 (0x6000-0x7FFF) for reading banked data.

Do NOT use slots 6-7 (0xC000-0xFFFF) - they may have ROM mapped and will cause system halt.

| Slot | Address Range | Register |
|------|---------------|----------|
| 0    | 0x0000-0x1FFF | 0x50     |
| 1    | 0x2000-0x3FFF | 0x51     |
| 2    | 0x4000-0x5FFF | 0x52     |
| 3    | 0x6000-0x7FFF | 0x53     |
| 4    | 0x8000-0x9FFF | 0x54     |
| 5    | 0xA000-0xBFFF | 0x55     |
| 6    | 0xC000-0xDFFF | 0x56     |
| 7    | 0xE000-0xFFFF | 0x57     |

### Build Output Verification

When banking works correctly, you'll see:
```
Bank 20, 4864 tail bytes free
```

If banks show "8192 tail bytes free", the data wasn't included.

### Crossing 8K Boundaries

When reading data that spans multiple pages, recalculate the page mapping:

```c
src_page = 40 + (src_offset / 8192);
src_addr = 0x6000 + (src_offset % 8192);

if (src_page != last_src_page) {
    IO_NEXTREG_REG = 0x53;
    IO_NEXTREG_DAT = src_page;
    last_src_page = src_page;
}
```

---

## Layer 2 Border Image Implementation

The border image (60x192 pixels, 11520 bytes RGB332) is stored across pages 40-41:
- Page 40: first 8192 bytes
- Page 41: remaining 3328 bytes

Drawing process:
1. Save current MMU slot states
2. Map source page to slot 3 (0x6000)
3. Map destination Layer 2 bank to slot 2 (0x4000)
4. Copy row by row, remapping pages as needed
5. For mirrored image, reverse column order during copy
6. Restore original MMU slot states
