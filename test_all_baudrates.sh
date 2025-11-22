#!/bin/bash
# Test multiple baudrates by capturing data at each

PORT="/dev/ttyUSB0"
DURATION=1

# Common baudrates for ESP8266
BAUDRATES=(9600 19200 38400 57600 74880 115200 230400)

echo "Testing multiple baudrates on $PORT"
echo "======================================"
echo ""

for BAUD in "${BAUDRATES[@]}"; do
    OUTFILE="/tmp/serial_${BAUD}.bin"

    echo "Testing $BAUD baud..."
    echo "RESET YOUR DEVICE NOW!"

    # Configure serial port
    stty -F $PORT $BAUD cs8 -cstopb -parenb raw -echo 2>/dev/null

    # Capture data
    timeout $DURATION cat $PORT > $OUTFILE 2>/dev/null

    # Check file size
    FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)

    if [ $FILESIZE -gt 0 ]; then
        echo "✓ Captured $FILESIZE bytes at $BAUD baud"
        echo ""
        echo "HEX DUMP (first 128 bytes):"
        echo "--------------------------------------"
        hexdump -C $OUTFILE | head -10
        echo ""
        echo "AS TEXT (first 100 chars):"
        echo "--------------------------------------"
        head -c 100 $OUTFILE
        echo ""
        echo "======================================"
        echo ""
    else
        echo "✗ No data at $BAUD"
        echo ""
    fi

    sleep 0.5
done

echo ""
echo "Summary:"
echo "======================================"
for BAUD in "${BAUDRATES[@]}"; do
    OUTFILE="/tmp/serial_${BAUD}.bin"
    FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)
    if [ $FILESIZE -gt 0 ]; then
        echo "$BAUD baud: $FILESIZE bytes - saved to $OUTFILE"
    fi
done
