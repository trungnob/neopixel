#!/usr/bin/env python3
"""
LED Pattern Designer - Logging Server
Serves the designer HTML and logs all pattern uploads for debugging
"""

from http.server import HTTPServer, SimpleHTTPRequestHandler
import json
from datetime import datetime
import sys
import os

# Log file path
LOG_FILE = os.path.join(os.path.dirname(__file__), 'pattern-upload.log')

class LoggingHTTPRequestHandler(SimpleHTTPRequestHandler):
    """HTTP handler that serves files AND logs pattern uploads"""

    def do_POST(self):
        """Handle POST requests to /log endpoint"""
        if self.path == '/log':
            # Read the POST body
            content_length = int(self.headers.get('Content-Length', 0))
            post_data = self.rfile.read(content_length)

            # Parse JSON
            try:
                data = json.loads(post_data.decode('utf-8'))

                # Print formatted log (both to stdout and file)
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                log_lines = []
                log_lines.append(f"\n{'='*60}")
                log_lines.append(f"[{timestamp}] PATTERN UPLOAD LOGGED")
                log_lines.append(f"{'='*60}")
                log_lines.append(f"Client: {self.client_address[0]}")

                if 'sparse' in data:
                    # Sparse format (new)
                    sparse = data['sparse']
                    log_lines.append(f"Format: SPARSE (efficient)")
                    log_lines.append(f"Pixels: {len(sparse)}")
                    log_lines.append(f"Scroll Speed: {data.get('scrollSpeed', 'N/A')} ms")
                    log_lines.append(f"\nPixel Details:")
                    for i, pixel in enumerate(sparse[:10]):  # Show first 10
                        led_idx, r, g, b = pixel
                        log_lines.append(f"  LED[{led_idx:4d}] = RGB({r:3d}, {g:3d}, {b:3d})")
                    if len(sparse) > 10:
                        log_lines.append(f"  ... and {len(sparse) - 10} more pixels")

                    # Show payload size
                    payload_size = len(post_data)
                    log_lines.append(f"\nPayload Size: {payload_size} bytes")
                    log_lines.append(f"Efficiency: {len(sparse)} pixels in {payload_size} bytes")

                elif 'pattern' in data:
                    # Full format (old - should not happen)
                    pattern = data['pattern']
                    log_lines.append(f"Format: FULL (inefficient - OLD VERSION!)")
                    log_lines.append(f"Total LEDs: {len(pattern)}")
                    log_lines.append(f"WARNING: This is the old format! Browser is cached!")

                    # Count non-black pixels
                    non_black = sum(1 for p in pattern if p[0] > 0 or p[1] > 0 or p[2] > 0)
                    log_lines.append(f"Non-black pixels: {non_black}")
                    payload_size = len(post_data)
                    log_lines.append(f"Payload Size: {payload_size} bytes")
                    log_lines.append(f"⚠️  INEFFICIENT: {non_black} pixels sent as {len(pattern)} LEDs!")

                else:
                    log_lines.append(f"Unknown format: {data.keys()}")
                    log_lines.append(f"Data: {json.dumps(data, indent=2)[:500]}")

                log_lines.append(f"{'='*60}\n")

                # Write to both stdout and file
                log_text = '\n'.join(log_lines)
                print(log_text)
                sys.stdout.flush()

                with open(LOG_FILE, 'a') as f:
                    f.write(log_text + '\n')
                    f.flush()

                # Send success response
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(b'{"status":"logged"}')

            except Exception as e:
                print(f"Error parsing log data: {e}")
                self.send_response(400)
                self.end_headers()
        else:
            # Not a log endpoint
            self.send_response(404)
            self.end_headers()

    def do_OPTIONS(self):
        """Handle CORS preflight"""
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def log_message(self, format, *args):
        """Suppress default request logging (too noisy)"""
        # Only log non-file requests
        if '/log' in args[0] or 'POST' in args[0]:
            sys.stderr.write(f"[{datetime.now().strftime('%H:%M:%S')}] {format % args}\n")

def run_server(port=8080):
    """Start the logging server"""
    server_address = ('', port)
    httpd = HTTPServer(server_address, LoggingHTTPRequestHandler)

    # Create/clear log file
    with open(LOG_FILE, 'w') as f:
        f.write(f"LED Pattern Upload Log - Started {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write("="*60 + "\n\n")

    print("="*60)
    print("LED Pattern Designer - Logging Server")
    print("="*60)
    print(f"\nServer running on port {port}")
    print(f"\nAccess from your phone:")
    print(f"  http://192.168.1.132:{port}/pattern-designer.html")
    print(f"\nLogs are being written to:")
    print(f"  {LOG_FILE}")
    print(f"\nTo watch logs in real-time, run:")
    print(f"  tail -f {LOG_FILE}")
    print(f"\nPress Ctrl+C to stop")
    print("="*60)
    print()

    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\n\nServer stopped.")
        httpd.shutdown()

if __name__ == '__main__':
    run_server(8080)
