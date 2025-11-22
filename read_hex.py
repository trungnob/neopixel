#!/usr/bin/env python3
"""
Read raw data from serial port and display in hex
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

        # Hex representation
        hex_str = ' '.join(f'{b:02x}' for b in chunk)

        # ASCII representation (printable chars only)
        ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)

        print(f"{i:04x}: {hex_str:<48} | {ascii_str}")

def read_serial(port, baudrate, duration=3):
    """Read from serial port"""
    print(f"\nReading from {port} at {baudrate} baud for {duration} seconds...")
    print("=" * 80)

    try:
        ser = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=0.5,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE
        )

        # Clear any existing data
        ser.reset_input_buffer()
        time.sleep(0.2)

        all_data = b''
        start_time = time.time()

        print("Reading data (reset your device now!)...\n")

        while time.time() - start_time < duration:
            if ser.in_waiting:
                chunk = ser.read(ser.in_waiting)
                all_data += chunk
                print(f"[Got {len(chunk)} bytes]")
            time.sleep(0.1)

        ser.close()

        if all_data:
            print(f"\nTotal bytes received: {len(all_data)}")
            print("\nHex dump:")
            print("-" * 80)
            hex_dump(all_data)
            print("-" * 80)

            # Also try to show as text
            print("\nAs text (with errors replaced):")
            print("-" * 80)
            try:
                text = all_data.decode('utf-8', errors='replace')
                print(text)
            except:
                print("[Cannot decode as text]")
            print("-" * 80)
        else:
            print("\n⚠ No data received!")

        return all_data

    except Exception as e:
        print(f"Error: {e}")
        return None

def main():
    port = "/dev/ttyUSB0"

    if len(sys.argv) > 1:
        baudrate = int(sys.argv[1])
        baudrates = [baudrate]
    else:
        # Test multiple baudrates
        baudrates = [115200, 74880, 9600, 57600]

    for baudrate in baudrates:
        data = read_serial(port, baudrate, duration=3)

        if len(baudrates) > 1:
            response = input("\nPress Enter to test next baudrate, or 'q' to quit: ")
            if response.lower() == 'q':
                break

if __name__ == "__main__":
    main()
