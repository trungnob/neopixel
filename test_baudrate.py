#!/usr/bin/env python3
"""
Interactive baudrate tester - shows live output
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

def test_baudrate(port, baudrate, duration=5):
    """Test a baudrate and show output"""
    print(f"\n{'='*60}")
    print(f"Testing {baudrate} baud (reading for {duration} seconds)")
    print(f"{'='*60}")

    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=1,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )

        # Clear buffer
        ser.reset_input_buffer()
        time.sleep(0.1)

        # Read data
        start_time = time.time()
        data_received = False

        while time.time() - start_time < duration:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                if data:
                    data_received = True
                    try:
                        # Try to decode as text
                        text = data.decode('utf-8', errors='replace')
                        print(text, end='', flush=True)
                    except:
                        print(f"[Binary: {data.hex()}]", end='', flush=True)
            time.sleep(0.1)

        print()  # New line

        if not data_received:
            print("[No data received]")

        ser.close()

    except Exception as e:
        print(f"Error: {e}")

def main():
    port = "/dev/ttyUSB0"

    # Common ESP8266 baudrates
    baudrates = [115200, 74880, 9600, 57600, 230400]

    print(f"Baudrate detection for {port}")
    print("Note: Try resetting your ESP8266 to see boot messages")
    print()

    for baudrate in baudrates:
        test_baudrate(port, baudrate, duration=3)

        response = input("\nPress Enter to test next baudrate, or 'q' to quit: ")
        if response.lower() == 'q':
            break

    print("\nDone!")

if __name__ == "__main__":
    main()
