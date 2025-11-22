#!/usr/bin/env python3
"""
Test multiple baudrates - prompts to reset device for each test
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
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_str = ' '.join(f'{b:02x}' for b in chunk)
        ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)
        print(f"{i:04x}: {hex_str:<48} | {ascii_str}")

def test_baudrate(port, baudrate, duration=3):
    """Test a single baudrate"""
    print(f"\n{'='*70}")
    print(f"Testing {baudrate} baud")
    print(f"{'='*70}")
    print(f"RESET YOUR DEVICE NOW! Reading for {duration} seconds...")
    print()

    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=0.5,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )

        ser.reset_input_buffer()
        time.sleep(0.2)

        all_data = b''
        start_time = time.time()

        while time.time() - start_time < duration:
            if ser.in_waiting:
                chunk = ser.read(ser.in_waiting)
                all_data += chunk
                print(f"[Got {len(chunk)} bytes]", end=' ', flush=True)
            time.sleep(0.1)

        print()
        ser.close()

        if all_data:
            print(f"\nReceived {len(all_data)} bytes total")
            print("\nHEX DUMP:")
            print("-" * 70)
            hex_dump(all_data[:256])
            if len(all_data) > 256:
                print(f"... ({len(all_data) - 256} more bytes)")
            print("-" * 70)

            print("\nAS TEXT:")
            try:
                text = all_data.decode('utf-8', errors='replace')
                print(repr(text[:200]))
            except:
                print("[Cannot decode]")

            return True
        else:
            print("✗ No data received")
            return False

    except Exception as e:
        print(f"Error: {e}")
        return False

def main():
    port = "/dev/ttyUSB0"
    baudrates = [115200, 74880, 9600, 57600, 19200, 38400]

    print(f"Baudrate detector for {port}")
    print("This will test each baudrate one by one")
    print()

    successful = []

    for baudrate in baudrates:
        if test_baudrate(port, baudrate, duration=3):
            successful.append(baudrate)

        response = input("\nPress Enter to test next baudrate, or 'q' to quit: ")
        if response.lower() == 'q':
            break

    if successful:
        print("\n" + "="*70)
        print("Baudrates that received data:")
        for br in successful:
            print(f"  - {br}")
    else:
        print("\nNo data received at any baudrate")
        print("Check:")
        print("  1. Device is powered")
        print("  2. TX pin is connected")
        print("  3. Device transmits data (try resetting it)")

if __name__ == "__main__":
    main()
