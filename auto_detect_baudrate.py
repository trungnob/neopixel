#!/usr/bin/env python3
"""
Automatic baudrate detection based on data readability
"""
import sys
import time

try:
    import serial
except ImportError:
    print("Installing pyserial...")
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "pyserial"])
    import serial

def hex_dump(data, bytes_per_line=16):
    """Pretty print hex dump"""
    lines = []
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_str = ' '.join(f'{b:02x}' for b in chunk)
        ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)
        lines.append(f"{i:04x}: {hex_str:<48} | {ascii_str}")
    return '\n'.join(lines)

def analyze_data(data):
    """Analyze data quality - return score (higher is better)"""
    if not data or len(data) == 0:
        return 0

    # Count printable ASCII characters
    printable = sum(1 for b in data if 32 <= b <= 126 or b in [9, 10, 13])

    # Calculate ratio
    ratio = printable / len(data)

    # Look for common patterns (newlines, spaces, letters)
    has_newlines = b'\n' in data or b'\r' in data
    has_spaces = b' ' in data

    score = ratio * 100
    if has_newlines:
        score += 10
    if has_spaces:
        score += 5

    return score

def test_baudrate(port, baudrate, duration=1):
    """Test a baudrate and return data + quality score"""
    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=0.5,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )

        # Clear buffer
        ser.reset_input_buffer()
        time.sleep(0.1)

        # Collect data
        all_data = b''
        start_time = time.time()

        while time.time() - start_time < duration:
            if ser.in_waiting:
                chunk = ser.read(ser.in_waiting)
                all_data += chunk
            time.sleep(0.05)

        ser.close()

        score = analyze_data(all_data)

        return all_data, score

    except Exception as e:
        return None, 0

def main():
    port = "/dev/ttyUSB0"

    # Test these baudrates
    baudrates = [9600, 19200, 38400, 57600, 74880, 115200, 230400, 460800, 921600]

    print(f"Auto-detecting baudrate for {port}")
    print("Reading 1 second from each baudrate")
    print("=" * 70)
    print("\nTIP: Reset your device now to generate output!\n")
    time.sleep(1)

    results = []

    for baudrate in baudrates:
        print(f"Testing {baudrate:7} baud...", end=" ", flush=True)
        data, score = test_baudrate(port, baudrate, duration=1)

        if data and len(data) > 0:
            print(f"✓ {len(data):4} bytes, score: {score:5.1f}")
            results.append((baudrate, data, score))
        else:
            print("✗ No data")

    print("\n" + "=" * 70)
    print("RESULTS (sorted by quality score):")
    print("=" * 70)

    # Sort by score
    results.sort(key=lambda x: x[2], reverse=True)

    for i, (baudrate, data, score) in enumerate(results[:5], 1):
        print(f"\n#{i} - {baudrate} baud (score: {score:.1f}) - {len(data)} bytes")
        print("-" * 70)

        # Show hex dump (first 256 bytes)
        print("HEX DUMP:")
        print(hex_dump(data[:256]))
        if len(data) > 256:
            print(f"... ({len(data) - 256} more bytes)")

        # Also try to show as text
        print("\nAS TEXT:")
        try:
            text = data.decode('utf-8', errors='replace')
            sample = text[:200]
            print(repr(sample))
        except:
            print("[Cannot decode]")

    if results:
        best = results[0]
        print("\n" + "=" * 70)
        print(f"RECOMMENDED: {best[0]} baud (score: {best[2]:.1f})")
        print("=" * 70)
        print(f"\nTo connect:")
        print(f"  screen /dev/ttyUSB0 {best[0]}")
        print(f"  or")
        print(f"  minicom -D /dev/ttyUSB0 -b {best[0]}")
    else:
        print("\nNo data detected. Make sure:")
        print("  1. Device is powered on")
        print("  2. Device is transmitting (try resetting it)")
        print("  3. TX pin is connected")

if __name__ == "__main__":
    main()
