# Z88DK Banking for ZX Spectrum Next

## Overview

This document covers how to store large data (like images) in separate memory banks using z88dk for the ZX Spectrum Next.

## PNG to Working Binary: Step-by-Step

### 1. Create the Image

- Use `layer2_editor.py` tool to create/edit the image
- Export as header file (`layer2_image.h`) containing RGB332 pixel data
- Image dimensions: 60x191 pixels = 11460 bytes

### 2. Extract Binary from Header

The `tools/extract_image_bin.py` script extracts raw bytes from the header file:

```bash
python3 tools/extract_image_bin.py src/layer2_image.h bin/border_image.bin
```

### 3. Split Binary into 8K Chunks

Each memory page is 8192 bytes, so split the binary:

```bash
head -c 8192 bin/border_image.bin > src/border_bank40.bin
tail -c +8193 bin/border_image.bin > src/border_bank41.bin
```

### 4. Convert Binary to Assembly defb Statements

The `BINARY` directive did NOT work reliably. Convert binaries to inline `defb`:

```python
# Python snippet to convert binary to defb
with open('src/border_bank40.bin', 'rb') as f:
    data = f.read()

for i in range(0, len(data), 16):
    chunk = data[i:i+16]
    hex_bytes = ', '.join(f'0x{b:02x}' for b in chunk)
    print(f'    defb {hex_bytes}')
```

### 5. Create Assembly File

Create `src/border_data.asm`:

```asm
SECTION PAGE_40
PUBLIC _border_image_bank40
_border_image_bank40:
    defb 0x18, 0x18, 0x18, ...

SECTION PAGE_41
PUBLIC _border_image_bank41
_border_image_bank41:
    defb 0x00, 0x00, 0x00, ...
```

### 6. Add to Makefile

```makefile
ASMS = src/border_data.asm

$(BIN_DIR)/$(OUTPUT).nex: $(SRCS) $(ASMS) $(HDRS)
    $(COMPILER) $(TARGET) $(CFLAGS) $(SRCS) $(ASMS) -o $(BIN_DIR)/$(OUTPUT) -create-app -subtype=nex
```

### 7. Force Linker Inclusion in C

Add extern declarations and dummy reference:

```c
extern uint8_t border_image_bank40;
extern uint8_t border_image_bank41;

void force_include(void) {
    volatile uint8_t *ptr = &border_image_bank40;
    ptr = &border_image_bank41;
    (void)ptr;
}
```

### 8. Access Data at Runtime

Map the page to slot 3 (0x6000) and read:

```c
// Map page 40 to slot 3
IO_NEXTREG_REG = 0x53;  // MMU slot 3 register
IO_NEXTREG_DAT = 40;    // Page number

// Read from 0x6000
const uint8_t *src = (const uint8_t *)0x6000;
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
Bank 20, 4924 tail bytes free
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

The border image (60x191 pixels, 11460 bytes RGB332) is stored across pages 40-41:
- Page 40: first 8192 bytes
- Page 41: remaining 3268 bytes

Drawing process:
1. Save current MMU slot states
2. Map source page to slot 3 (0x6000)
3. Map destination Layer 2 bank to slot 2 (0x4000)
4. Copy row by row, remapping pages as needed
5. For mirrored image, reverse column order during copy
6. Restore original MMU slot states
