#!/bin/bash
# Capture serial data and display as hexdump

PORT="/dev/ttyUSB0"
BAUDRATE=${1:-115200}
DURATION=${2:-1}
OUTFILE="/tmp/serial_capture_${BAUDRATE}.bin"

echo "Capturing data from $PORT at $BAUDRATE baud for $DURATION seconds"
echo "RESET YOUR DEVICE NOW!"
echo "=========================================="

# Configure serial port
stty -F $PORT $BAUDRATE cs8 -cstopb -parenb raw -echo

# Capture data for specified duration
timeout $DURATION cat $PORT > $OUTFILE 2>/dev/null

# Check if we got any data
FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)

if [ $FILESIZE -gt 0 ]; then
    echo "✓ Captured $FILESIZE bytes"
    echo ""
    echo "HEX DUMP:"
    echo "=========================================="
    hexdump -C $OUTFILE
    echo "=========================================="
    echo ""
    echo "AS TEXT:"
    echo "=========================================="
    cat $OUTFILE
    echo ""
    echo "=========================================="
    echo ""
    echo "Data saved to: $OUTFILE"
else
    echo "✗ No data captured"
fi
