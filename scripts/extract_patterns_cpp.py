#!/usr/bin/env python3
"""
Extract pattern code from main.cpp and create patterns.cpp for use with simulator
"""

import re
import sys

def extract_pattern_code(cpp_file):
    """Extract the pattern switch statement and related code from main.cpp"""

    with open(cpp_file, 'r') as f:
        lines = f.readlines()

    # Find the start of the pattern switch (around line 513)
    # We need to find: switch (currentPattern) {
    switch_start = None
    for i, line in enumerate(lines):
        if re.search(r'switch\s*\(\s*currentPattern\s*\)', line):
            switch_start = i
            break

    if switch_start is None:
        print("ERROR: Could not find pattern switch statement")
        return None

    # Find the end of the switch (should be before FastLED.show())
    # Look for the closing brace of the switch, then FastLED.show()
    switch_end = None
    brace_count = 0
    started = False

    for i in range(switch_start, len(lines)):
        line = lines[i]

        if '{' in line:
            brace_count += line.count('{')
            started = True
        if '}' in line:
            brace_count -= line.count('}')

        if started and brace_count == 0:
            switch_end = i
            break

    if switch_end is None:
        print("ERROR: Could not find end of switch statement")
        return None

    print(f"Found pattern switch from line {switch_start+1} to {switch_end+1}")

    # Extract the switch block
    switch_block = lines[switch_start:switch_end+1]

    # Find font data (lines 60-105 approximately)
    font_start = None
    font_end = None

    for i, line in enumerate(lines):
        if 'const uint8_t font5x7' in line:
            font_start = i
            # Find the end of the font array
            for j in range(i, len(lines)):
                if '};' in lines[j] and 'font' in lines[j-5:j+1][-1]:
                    font_end = j
                    break
            break

    font_data = lines[font_start:font_end+1] if (font_start is not None and font_end is not None) else []

    # Find getFontIndex function (should be after font data)
    getfont_start = None
    getfont_end = None

    for i, line in enumerate(lines):
        if 'int getFontIndex' in line or 'getFontIndex' in line:
            getfont_start = i
            # Find the closing brace
            brace_count = 0
            for j in range(i, len(lines)):
                if '{' in lines[j]:
                    brace_count += 1
                if '}' in lines[j]:
                    brace_count -= 1
                    if brace_count == 0:
                        getfont_end = j
                        break
            break

    getfont_data = lines[getfont_start:getfont_end+1] if (getfont_start is not None and getfont_end is not None) else []

    return {
        'switch_block': switch_block,
        'font_data': font_data,
        'getfont_data': getfont_data
    }

def generate_patterns_cpp(data, output_file):
    """Generate patterns.cpp from extracted data"""

    cpp_code = """// patterns.cpp - LED pattern implementations
// This file is shared between ESP8266 and simulator

#include "patterns.h"

// Font data for scrolling text
const uint8_t FONT_WIDTH = 5;
const uint8_t FONT_HEIGHT = 7;

"""

    # Add font data
    if data['font_data']:
        # Replace PROGMEM in font data for conditional compilation
        font_lines = ''.join(data['font_data'])
        font_lines = font_lines.replace('PROGMEM', '')
        cpp_code += font_lines + '\n\n'

    # Add getFontIndex function
    if data['getfont_data']:
        cpp_code += ''.join(data['getfont_data']) + '\n\n'

    # Create the runPattern function
    cpp_code += """// Run a specific pattern
// This contains the exact same code as main.cpp, but callable from simulator
void runPattern(int patternNum, PatternState& state) {
  CRGB* leds = state.leds;
  int activeLeds = state.activeLeds;
  uint8_t& hue = state.hue;

"""

    # Convert switch (currentPattern) to switch (patternNum)
    switch_code = ''.join(data['switch_block'])
    switch_code = switch_code.replace('currentPattern', 'patternNum')

    # Handle scrollText references (need to use state.scrollText)
    switch_code = re.sub(r'\bscrollText\b', 'state.scrollText', switch_code)
    switch_code = re.sub(r'\bscrollOffset\b', 'state.scrollOffset', switch_code)
    switch_code = re.sub(r'\bscrollSpeed\b', 'state.scrollSpeed', switch_code)

    cpp_code += switch_code

    cpp_code += """
}
"""

    # Write output
    with open(output_file, 'w') as f:
        f.write(cpp_code)

    print(f"Generated {output_file}")

def main():
    cpp_file = 'src/main.cpp'
    output_file = 'src/patterns.cpp'

    print(f"Extracting pattern code from {cpp_file}...")
    data = extract_pattern_code(cpp_file)

    if data:
        print(f"Generating {output_file}...")
        generate_patterns_cpp(data, output_file)
        print("Done!")
    else:
        print("Failed to extract patterns")
        sys.exit(1)

if __name__ == '__main__':
    main()
