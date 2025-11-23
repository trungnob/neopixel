#!/usr/bin/env python3
"""
Extract pattern code from main.cpp and generate simulator HTML
"""

import re
import sys

def extract_patterns(cpp_file):
    """Extract all pattern case statements from main.cpp"""

    with open(cpp_file, 'r') as f:
        content = f.read()

    # Find all case statements for patterns 100+
    # Match: case NUMBER: // COMMENT ... break;
    pattern_regex = r'case\s+(\d+):\s*//\s*([^\n]+)\s*\{?(.*?)break;'

    patterns = []
    for match in re.finditer(pattern_regex, content, re.DOTALL):
        case_num = int(match.group(1))
        description = match.group(2).strip()
        code_block = match.group(3).strip()

        if case_num >= 100:  # Only 2D patterns
            patterns.append({
                'number': case_num,
                'description': description,
                'code': code_block
            })

    return sorted(patterns, key=lambda x: x['number'])

def cpp_to_js(cpp_code, pattern_num):
    """Convert C++ pattern code to JavaScript"""

    js_code = cpp_code
    static_vars = []
    var_mappings = {}  # Map original names to global names
    defines = []

    # Extract #define statements first and convert to const
    define_pattern = r'#define\s+(\w+)\s+(.+?)(?:\n|$)'

    def replace_define(match):
        name = match.group(1)
        value = match.group(2).strip()
        defines.append(f"const {name} = {value};")
        return ''

    js_code = re.sub(define_pattern, replace_define, js_code)

    # Extract static variable declarations and move them to global scope
    # Handle both 1D and 2D arrays: var[N] or var[N][M]
    static_pattern = r'static\s+((?:unsigned\s+)?(?:int|uint8_t|uint16_t|long|bool|byte|float|double))\s+(\w+)(?:\[([^\]]+)\](?:\[([^\]]+)\])?)?(?:\s*=\s*([^;]+))?;'

    def replace_static(match):
        var_type = match.group(1)
        var_name = match.group(2)
        array_size1 = match.group(3)
        array_size2 = match.group(4)
        init_value = match.group(5)

        # Generate unique global variable name
        global_name = f"{var_name}_p{pattern_num}"
        var_mappings[var_name] = global_name

        if array_size2:
            # 2D Array declaration
            init_code = f"""if (typeof {global_name} === 'undefined') {{
            {global_name} = [];
            for(let i = 0; i < {array_size1}; i++) {{
              {global_name}[i] = new Array({array_size2}).fill(0);
            }}
          }}"""
            static_vars.append(init_code)
            return ''
        elif array_size1:
            # 1D Array declaration
            static_vars.append(f"if (typeof {global_name} === 'undefined') {global_name} = new Array({array_size1}).fill(0);")
            return ''
        else:
            # Simple variable
            init = init_value if init_value else '0'
            static_vars.append(f"if (typeof {global_name} === 'undefined') {global_name} = {init};")
            return ''

    # Remove static declarations and collect them
    js_code = re.sub(static_pattern, replace_static, js_code)

    # Remove any remaining 'static' keywords that might have been missed
    js_code = re.sub(r'\bstatic\s+', '', js_code)

    # Replace static variable references with global names
    for orig_name, global_name in var_mappings.items():
        # Replace whole word references
        js_code = re.sub(r'\b' + orig_name + r'\b', global_name, js_code)

    # Convert variable declarations
    js_code = re.sub(r'\bint\s+', 'let ', js_code)
    js_code = re.sub(r'\buint8_t\s+', 'let ', js_code)
    js_code = re.sub(r'\buint16_t\s+', 'let ', js_code)
    js_code = re.sub(r'\bfloat\s+', 'let ', js_code)
    js_code = re.sub(r'\bdouble\s+', 'let ', js_code)
    js_code = re.sub(r'\bbool\s+', 'let ', js_code)
    js_code = re.sub(r'\bbyte\s+', 'let ', js_code)

    # Convert true/false
    js_code = re.sub(r'\btrue\b', 'true', js_code)
    js_code = re.sub(r'\bfalse\b', 'false', js_code)

    # Convert millis()
    js_code = re.sub(r'\bmillis\(\)', 'Date.now()', js_code)

    # Convert CRGB::ColorName to CRGBColors.ColorName
    js_code = re.sub(r'CRGB::(\w+)', r'CRGBColors.\1', js_code)

    # Fix array declarations that have let in front: "let arr[N]" -> "arr"
    js_code = re.sub(r'\blet\s+(\w+)\[', r'\1[', js_code)

    # Remove C++ style casts: (int), (float), (uint8_t), etc.
    js_code = re.sub(r'\((?:int|uint8_t|uint16_t|long|float|double|byte|bool)\)', '', js_code)

    # Prepend defines first, then static variable initializations, then the rest of the code
    preamble = []
    if defines:
        preamble.extend(defines)
    if static_vars:
        preamble.extend(static_vars)

    if preamble:
        js_code = '\n          '.join(preamble) + '\n          ' + js_code

    return js_code

def generate_simulator_html(patterns, template_file, output_file):
    """Generate HTML file with extracted patterns"""

    with open(template_file, 'r') as f:
        template = f.read()

    # Generate switch cases
    cases = []
    for p in patterns:
        js_code = cpp_to_js(p['code'], p['number'])
        case_code = f"""
        case {p['number']}: // {p['description']}
          {js_code}
          break;
"""
        cases.append(case_code)

    # Replace placeholder
    all_cases = '\n'.join(cases)
    html = template.replace('${PATTERN_CASES_PLACEHOLDER}', all_cases)

    # Write output
    with open(output_file, 'w') as f:
        f.write(html)

    print(f"Generated {output_file} with {len(patterns)} patterns")
    return len(patterns)

def main():
    cpp_file = 'src/main.cpp'
    template_file = 'cpp-simulator.html'
    output_file = 'cpp-simulator-generated.html'

    print(f"Extracting patterns from {cpp_file}...")
    patterns = extract_patterns(cpp_file)

    print(f"Found {len(patterns)} 2D patterns:")
    for p in patterns:
        print(f"  {p['number']}: {p['description']}")

    print(f"\nGenerating simulator...")
    generate_simulator_html(patterns, template_file, output_file)

    print(f"\nDone! Open {output_file} in your browser.")

if __name__ == '__main__':
    main()
