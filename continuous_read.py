#!/usr/bin/env python3
"""
Continuously read from serial port and display in hex
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

def hex_dump(data, offset=0, bytes_per_line=16):
    """Pretty print hex dump"""
    for i in range(0, len(data), bytes_per_line):
        chunk = data[i:i+bytes_per_line]
        hex_str = ' '.join(f'{b:02x}' for b in chunk)
        ascii_str = ''.join(chr(b) if 32 <= b <= 126 else '.' for b in chunk)
        print(f"{offset+i:04x}: {hex_str:<48} | {ascii_str}")

def main():
    port = "/dev/ttyUSB0"

    # Get baudrate from command line or use default
    if len(sys.argv) > 1:
        baudrate = int(sys.argv[1])
    else:
        baudrate = 115200

    print(f"Reading from {port} at {baudrate} baud")
    print("Press Ctrl+C to stop")
    print("=" * 70)
    print("RESET YOUR DEVICE NOW!")
    print("=" * 70)
    print()

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

        total_bytes = 0

        while True:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                if data:
                    print(f"\n[Received {len(data)} bytes at offset {total_bytes}]")
                    hex_dump(data, offset=total_bytes)
                    total_bytes += len(data)
            time.sleep(0.05)

    except KeyboardInterrupt:
        print(f"\n\nTotal bytes received: {total_bytes}")
        print("Done!")
    except Exception as e:
        print(f"Error: {e}")
    finally:
        if 'ser' in locals():
            ser.close()

if __name__ == "__main__":
    main()
