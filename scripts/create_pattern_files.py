#!/usr/bin/env python3
"""
Extract patterns from main.cpp and create individual pattern files
"""

import re
import os

# Ensure directory exists
os.makedirs('src/patterns', exist_ok=True)

# Read main.cpp
with open('src/main.cpp', 'r') as f:
    lines = f.readlines()
    content = ''.join(lines)

# Pattern map
PATTERNS = {
    100: 'horizontal_bars',
    101: 'vertical_ripple',
    102: 'fire_rising',
    103: 'rain_drops',
    104: 'vertical_equalizer',
    105: 'scanning_lines',
    106: 'checkerboard',
    107: 'diagonal_sweep',
    108: 'vertical_wave',
    109: 'plasma_2d',
    110: 'matrix_rain',
    111: 'game_of_life',
    112: 'wave_pool',
    113: 'aurora_2d',
    114: 'lava_lamp',
    115: 'ripple_2d',
    116: 'starfield',
    117: 'side_fire',
    118: 'scrolling_rainbow',
    119: 'particle_fountain',
    120: 'scrolling_text',
}

# Extract each pattern
for case_num, func_name in PATTERNS.items():
    # Find the case block
    pattern = rf'case\s+{case_num}\s*:(.*?)break;'
    match = re.search(pattern, content, re.DOTALL)

    if not match:
        print(f"Warning: Could not find case {case_num}")
        continue

    case_content = match.group(1).strip()

    # Extract comment
    comment_match = re.search(r'//\s*(.+)', case_content)
    comment = comment_match.group(1) if comment_match else f"Pattern {case_num}"

    # Remove comment line
    case_content = re.sub(r'//[^\n]*\n', '', case_content, count=1)

    # Remove outer braces if present
    case_content = case_content.strip()
    if case_content.startswith('{') and case_content.endswith('}'):
        case_content = case_content[1:-1].strip()

    # Determine function signature
    if case_num == 120:
        # Scrolling text needs extra parameters
        sig = f"void pattern_{func_name}(CRGB* leds, int activeLeds, uint8_t& hue,\n                           const char* text, int& scrollOffset, int scrollSpeed)"
    else:
        sig = f"void pattern_{func_name}(CRGB* leds, int activeLeds, uint8_t& hue)"

    # Generate file
    file_content = f"""// pattern_{case_num}_{func_name}.cpp
#include "../patterns.h"

// {comment}
{sig} {{
{case_content}
}}
"""

    # Special fix for pattern 120
    if case_num == 120:
        file_content = file_content.replace('scrollText.length()', 'strlen(text)')
        file_content = file_content.replace('scrollText[charIdx]', 'text[charIdx]')
        file_content = file_content.replace('#include "../patterns.h"', '#include "../patterns.h"\n#include <string.h>')
        file_content = file_content.replace('int textWidth = strlen(text) * charWidth;',
                                           'int textLen = strlen(text);\n          int textWidth = textLen * charWidth;')
        file_content = file_content.replace('for(int charIdx = 0; charIdx < strlen(text); charIdx++)',
                                            'for(int charIdx = 0; charIdx < textLen; charIdx++)')

    filename = f'src/patterns/pattern_{case_num}_{func_name}.cpp'
    with open(filename, 'w') as f:
        f.write(file_content)

    print(f"Created {filename}")

# Create font.cpp (no pattern code, just font data)
font_content = """// font.cpp - Shared font data for scrolling text patterns
#include "../patterns.h"

const uint8_t FONT_WIDTH = 5;
const uint8_t FONT_HEIGHT = 7;

const uint8_t font5x7[][5] = {
  {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
  {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
  {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
  {0x00, 0x00, 0x00, 0x00, 0x00}, // space
  {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
  {0x00, 0x60, 0x60, 0x00, 0x00}, // .
  {0x08, 0x08, 0x08, 0x08, 0x08}, // -
};

int getFontIndex(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a';
  if (c >= '0' && c <= '9') return 26 + (c - '0');
  if (c == ' ') return 36;
  if (c == '!') return 37;
  if (c == '.') return 38;
  if (c == '-') return 39;
  return 36;
}
"""

with open('src/patterns/font.cpp', 'w') as f:
    f.write(font_content)

print("Created src/patterns/font.cpp")
print(f"\nDone! Created {len(PATTERNS) + 1} files")
