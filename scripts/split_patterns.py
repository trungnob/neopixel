#!/usr/bin/env python3
"""
Split patterns.cpp into individual files (one per pattern)
"""

import re
import os

PATTERN_MAP = {
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

def split_patterns():
    # Read patterns.cpp
    with open('src/patterns.cpp', 'r') as f:
        content = f.read()

    # Extract font data and getFontIndex
    font_match = re.search(r'(const uint8_t FONT_WIDTH.*?^}\n)', content, re.DOTALL | re.MULTILINE)
    getfont_match = re.search(r'(int getFontIndex\(.*?^}\n)', content, re.DOTALL | re.MULTILINE)

    font_data = font_match.group(1) if font_match else ''
    getfont_data = getfont_match.group(1) if getfont_match else ''

    # Create font.cpp with shared font data
    font_cpp = f"""// font.cpp - Shared font data for scrolling text patterns
#include "patterns.h"

{font_data}

{getfont_data}
"""

    with open('src/patterns/font.cpp', 'w') as f:
        f.write(font_cpp)
    print("Created src/patterns/font.cpp")

    # Extract each pattern function
    pattern_regex = r'(// .+?\nvoid pattern_(\w+)\(.*?\n\})\n'

    for match in re.finditer(pattern_regex, content, re.DOTALL):
        func_code = match.group(1)
        func_name = match.group(2)

        # Find case number
        case_num = None
        for num, name in PATTERN_MAP.items():
            if name == func_name:
                case_num = num
                break

        if case_num is None:
            print(f"Warning: Could not find case number for {func_name}")
            continue

        # Create individual file
        pattern_cpp = f"""// pattern_{case_num}_{func_name}.cpp
#include "../patterns.h"

{func_code}
"""

        filename = f'src/patterns/pattern_{case_num}_{func_name}.cpp'
        with open(filename, 'w') as f:
            f.write(pattern_cpp)
        print(f"Created {filename}")

    print(f"\nSplit {len(PATTERN_MAP)} patterns into individual files")

if __name__ == '__main__':
    split_patterns()
