#!/bin/bash
# Test all baudrates - prompts for reset at each one

PORT="/dev/ttyUSB0"
DURATION=1

# Common baudrates for ESP8266
BAUDRATES=(9600 19200 38400 57600 74880 115200 230400 460800)

echo "======================================"
echo "Baudrate Detection - Testing All"
echo "======================================"
echo ""

# Test each baudrate
for BAUD in "${BAUDRATES[@]}"; do
    OUTFILE="/tmp/serial_${BAUD}.bin"

    echo ""
    echo "======================================"
    echo "Testing $BAUD baud"
    echo "======================================"
    echo "RESET YOUR DEVICE NOW!"
    sleep 2

    # Configure serial port
    stty -F $PORT $BAUD cs8 -cstopb -parenb raw -echo 2>/dev/null

    # Capture data
    timeout $DURATION cat $PORT > $OUTFILE 2>/dev/null

    # Check file size
    FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)

    if [ $FILESIZE -gt 0 ]; then
        echo "✓ Captured $FILESIZE bytes"
    else
        echo "✗ No data"
        rm -f $OUTFILE
    fi

    sleep 0.5
done

echo ""
echo ""
echo "======================================"
echo "FULL RESULTS - ALL BAUDRATES"
echo "======================================"
echo ""

# Show all results
for BAUD in "${BAUDRATES[@]}"; do
    OUTFILE="/tmp/serial_${BAUD}.bin"

    if [ -f "$OUTFILE" ]; then
        FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)

        if [ $FILESIZE -gt 0 ]; then
            # Calculate readability score
            PRINTABLE=$(tr -cd '[:print:]\n\r\t' < $OUTFILE | wc -c)
            SCORE=$((PRINTABLE * 100 / FILESIZE))

            echo ""
            echo "========================================"
            echo "$BAUD BAUD - $FILESIZE bytes (${SCORE}% readable)"
            echo "========================================"

            echo ""
            echo "HEX DUMP (first 128 bytes):"
            echo "----------------------------------------"
            hexdump -C $OUTFILE | head -10

            echo ""
            echo "AS TEXT (first 200 chars):"
            echo "----------------------------------------"
            head -c 200 $OUTFILE
            echo ""
            echo "----------------------------------------"
            echo ""
        fi
    fi
done

echo ""
echo "======================================"
echo "SUMMARY:"
echo "======================================"

for BAUD in "${BAUDRATES[@]}"; do
    OUTFILE="/tmp/serial_${BAUD}.bin"

    if [ -f "$OUTFILE" ]; then
        FILESIZE=$(stat -c%s "$OUTFILE" 2>/dev/null || echo 0)

        if [ $FILESIZE -gt 0 ]; then
            PRINTABLE=$(tr -cd '[:print:]\n\r\t' < $OUTFILE | wc -c)
            SCORE=$((PRINTABLE * 100 / FILESIZE))

            echo "$BAUD baud: $FILESIZE bytes, ${SCORE}% readable -> $OUTFILE"
        fi
    fi
done

echo ""
echo "Done! Look for the baudrate with highest readability %"
