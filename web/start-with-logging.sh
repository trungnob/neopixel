#!/bin/bash
# LED Pattern Designer with Debug Logging
# This serves the designer AND logs all pattern uploads

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"

echo "========================================="
echo "LED Pattern Designer - Logging Server"
echo "========================================="
echo ""
echo "Starting server on port 8080..."
echo ""
echo "Access from your devices:"
echo "  Desktop: http://localhost:8080/pattern-designer.html"
echo "  Phone:   http://$(hostname -I | awk '{print $1}'):8080/pattern-designer.html"
echo ""
echo "Logs written to: pattern-upload.log"
echo "Watch logs: tail -f pattern-upload.log"
echo ""
echo "Press Ctrl+C to stop"
echo "========================================="
echo ""

python3 log-server.py
