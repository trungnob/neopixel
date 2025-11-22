#!/bin/bash
# Quick baudrate detection - tests all baudrates rapidly after device reset

PORT="/dev/ttyUSB0"

echo "Quick Baudrate Detector"
echo "======================================"
echo "Press Enter, then immediately RESET your device..."
read

echo "Testing all baudrates quickly..."
echo ""

# Test each baudrate for 0.3 seconds
for BAUD in 9600 19200 38400 57600 74880 115200 230400; do
    OUTFILE="/tmp/quick_${BAUD}.bin"

    # Configure and capture
    stty -F $PORT $BAUD cs8 -cstopb -parenb raw -echo 2>/dev/null
    timeout 0.3 cat $PORT > $OUTFILE 2>/dev/null

    FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)

    if [ $FILESIZE -gt 0 ]; then
        # Calculate quality score (ratio of printable ASCII)
        PRINTABLE=$(tr -cd '[:print:]\n\r\t' < $OUTFILE | wc -c)
        SCORE=$((PRINTABLE * 100 / FILESIZE))

        echo "$BAUD baud: $FILESIZE bytes (${SCORE}% readable)"
    fi
done

echo ""
echo "======================================"
echo "DETAILED RESULTS:"
echo "======================================"
echo ""

# Show detailed results for files with data, sorted by size
for FILE in /tmp/quick_*.bin; do
    if [ -f "$FILE" ]; then
        FILESIZE=$(stat -c%s "$FILE" 2>/dev/null || echo 0)
        if [ $FILESIZE -gt 0 ]; then
            BAUD=$(basename $FILE | sed 's/quick_\(.*\)\.bin/\1/')

            echo "========================================"
            echo "$BAUD BAUD - $FILESIZE bytes"
            echo "========================================"
            echo "HEX:"
            hexdump -C $FILE | head -8
            echo ""
            echo "TEXT:"
            cat $FILE | head -c 150
            echo ""
            echo ""
        fi
    fi
done
