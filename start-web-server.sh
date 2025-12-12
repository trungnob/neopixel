#!/bin/bash
# Get the first local IP address
IP=$(hostname -I | awk '{print $1}')

echo "================================================================"
echo "📱 Pattern Designer Mobile Access"
echo "================================================================"
echo "You can now access the designer from your iPhone at:"
echo ""
echo "👉 http://$IP:8000/pattern-designer.html"
echo ""
echo "Keep this terminal open while you use the app."
echo "Press Ctrl+C to stop the server."
echo "================================================================"

# Start the custom proxy server
python3 server.py
