#!/usr/bin/env python3
"""
Script to detect the correct baudrate for a serial device
"""
import sys
import time

try:
    import serial
except ImportError:
    print("pyserial not installed. Installing...")
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "pyserial"])
    import serial

# Common baudrates to test (ordered by popularity for ESP8266/Arduino)
BAUDRATES = [115200, 74880, 9600, 19200, 38400, 57600, 230400, 460800, 921600]

def test_baudrate(port, baudrate, timeout=2):
    """Test a specific baudrate and return received data"""
    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=timeout,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )

        print(f"Testing {baudrate} baud...", end=" ")

        # Clear any existing data
        ser.reset_input_buffer()

        # Read for a short time
        time.sleep(0.5)
        data = ser.read(ser.in_waiting or 100)

        ser.close()

        if data:
            # Check if data contains printable characters
            printable_ratio = sum(32 <= b < 127 or b in [10, 13] for b in data) / len(data)

            print(f"Got {len(data)} bytes, {printable_ratio*100:.1f}% printable")

            if printable_ratio > 0.7:  # If more than 70% printable, likely correct
                return True, data
            return False, data
        else:
            print("No data")
            return False, None

    except Exception as e:
        print(f"Error: {e}")
        return False, None

def main():
    port = "/dev/ttyUSB0"

    print(f"Detecting baudrate for {port}")
    print("=" * 50)

    results = []

    for baudrate in BAUDRATES:
        success, data = test_baudrate(port, baudrate)
        if success and data:
            results.append((baudrate, data))

    print("\n" + "=" * 50)
    print("RESULTS:")
    print("=" * 50)

    if results:
        print(f"\nLikely baudrate(s) found:")
        for baudrate, data in results:
            print(f"\n{baudrate} baud:")
            try:
                decoded = data.decode('utf-8', errors='replace')
                print(f"  Sample: {decoded[:200]}")
            except:
                print(f"  Raw: {data[:100]}")
    else:
        print("\nNo clear baudrate detected.")
        print("The device might not be transmitting data.")
        print("Try:")
        print("  1. Reset the device")
        print("  2. Check if the device transmits on startup")
        print("  3. Send commands to trigger output")

    print("\nTo manually test a baudrate:")
    print("  screen /dev/ttyUSB0 115200")
    print("  or")
    print("  minicom -D /dev/ttyUSB0 -b 115200")

if __name__ == "__main__":
    main()
