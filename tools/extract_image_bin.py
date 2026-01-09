#!/usr/bin/env python3
"""
Extract raw binary data from layer2_image.h to a .bin file
"""

import re
import sys

def extract_image_data(header_path, output_path):
    with open(header_path, 'r') as f:
        content = f.read()

    # Find the array data between { and };
    match = re.search(r'layer2_image_data\[\d+\]\s*=\s*\{([^}]+)\}', content, re.DOTALL)
    if not match:
        print("Could not find image data array")
        return False

    data_str = match.group(1)

    # Extract all hex values
    hex_values = re.findall(r'0x([0-9A-Fa-f]{2})', data_str)

    # Convert to bytes
    data = bytes(int(h, 16) for h in hex_values)

    print(f"Extracted {len(data)} bytes")

    # Write binary file
    with open(output_path, 'wb') as f:
        f.write(data)

    print(f"Written to {output_path}")
    return True

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("Usage: extract_image_bin.py <input.h> <output.bin>")
        sys.exit(1)

    extract_image_data(sys.argv[1], sys.argv[2])
