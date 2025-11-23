#!/usr/bin/env python3
"""
Generate patterns.cpp from main.cpp by converting each case to a function
"""

import re

# Map case numbers to function names
PATTERN_MAP = {
    100: 'pattern_horizontal_bars',
    101: 'pattern_vertical_ripple',
    102: 'pattern_fire_rising',
    103: 'pattern_rain_drops',
    104: 'pattern_vertical_equalizer',
    105: 'pattern_scanning_lines',
    106: 'pattern_checkerboard',
    107: 'pattern_diagonal_sweep',
    108: 'pattern_vertical_wave',
    109: 'pattern_plasma_2d',
    110: 'pattern_matrix_rain',
    111: 'pattern_game_of_life',
    112: 'pattern_wave_pool',
    113: 'pattern_aurora_2d',
    114: 'pattern_lava_lamp',
    115: 'pattern_ripple_2d',
    116: 'pattern_starfield',
    117: 'pattern_side_fire',
    118: 'pattern_scrolling_rainbow',
    119: 'pattern_particle_fountain',
    120: 'pattern_scrolling_text',
}

def extract_case_code(lines, case_num):
    """Extract code for a specific case from lines"""
    # Find case line
    case_pattern = rf'case\s+{case_num}\s*:'
    start_idx = None

    for i, line in enumerate(lines):
        if re.search(case_pattern, line):
            start_idx = i
            break

    if start_idx is None:
        return None

    # Extract until 'break;'
    code_lines = []
    for i in range(start_idx, len(lines)):
        code_lines.append(lines[i])
        if 'break;' in lines[i]:
            break

    return code_lines

def case_to_function(case_lines, func_name, case_num):
    """Convert case block to function"""
    if not case_lines:
        return None

    # Extract the comment and code
    case_code = ''.join(case_lines)

    # Extract comment
    comment_match = re.search(r'//\s*(.+)', case_lines[0])
    comment = comment_match.group(1) if comment_match else f"Pattern {case_num}"

    # Remove case line and break
    code = case_code
    code = re.sub(r'case\s+\d+\s*:\s*//[^\n]*\n', '', code)
    code = re.sub(r'break;\s*$', '', code)
    code = code.strip()

    # Remove outer braces if present
    if code.startswith('{') and code.endswith('}'):
        code = code[1:-1].strip()

    # Determine function signature
    if case_num == 120:  # scrolling text needs extra params
        signature = f"void {func_name}(CRGB* leds, int activeLeds, uint8_t& hue,\n                           const char* text, int& scrollOffset, int scrollSpeed)"
    else:
        signature = f"void {func_name}(CRGB* leds, int activeLeds, uint8_t& hue)"

    func_code = f"""// {comment}
{signature} {{
{code}
}}
"""

    return func_code

def main():
    # Read main.cpp
    with open('src/main.cpp', 'r') as f:
        lines = f.readlines()

    # Generate patterns.cpp
    output = """// patterns.cpp - LED pattern implementations
// This file is compiled for both ESP8266 and simulator

#include "patterns.h"

// Font data for scrolling text
const uint8_t FONT_WIDTH = 5;
const uint8_t FONT_HEIGHT = 7;

"""

    # Add font data (find in main.cpp)
    in_font = False
    font_lines = []
    for line in lines:
        if 'const uint8_t font5x7' in line:
            in_font = True
        if in_font:
            font_lines.append(line)
            if '};' in line and len(font_lines) > 5:
                break

    if font_lines:
        # Remove PROGMEM for portability
        font_data = ''.join(font_lines).replace(' PROGMEM', '')
        output += font_data + '\n\n'

    # Add getFontIndex function
    getfont_start = None
    for i, line in enumerate(lines):
        if 'int getFontIndex' in line:
            getfont_start = i
            break

    if getfont_start:
        getfont_lines = []
        brace_count = 0
        for i in range(getfont_start, len(lines)):
            line = lines[i]
            getfont_lines.append(line)
            if '{' in line:
                brace_count += line.count('{')
            if '}' in line:
                brace_count -= line.count('}')
                if brace_count == 0:
                    break
        output += ''.join(getfont_lines) + '\n\n'

    # Generate pattern functions
    for case_num, func_name in sorted(PATTERN_MAP.items()):
        case_lines = extract_case_code(lines, case_num)
        if case_lines:
            func_code = case_to_function(case_lines, func_name, case_num)
            if func_code:
                output += func_code + '\n'
                print(f"Generated {func_name} (case {case_num})")

    # Write output
    with open('src/patterns.cpp', 'w') as f:
        f.write(output)

    print(f"\nGenerated src/patterns.cpp with {len(PATTERN_MAP)} pattern functions")

if __name__ == '__main__':
    main()
