import http.server
import socketserver
import urllib.request
import urllib.parse
import json
import os
import sys

# Parse command line arguments
PORT = 8000
if len(sys.argv) > 1 and sys.argv[1] == '--port' and len(sys.argv) > 2:
    PORT = int(sys.argv[2])

ESP_IP = "192.168.1.130" # Default, can be overridden

class ProxyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def do_GET(self):
        print(f"Received GET request: {self.path}")
        if self.path.startswith("/api/setText"):
             # Relay GET /api/setText?text=... to http://ESP_IP/setText?text=...
            query = urllib.parse.urlparse(self.path).query
            target_url = f"http://{ESP_IP}/setText?{query}"
            print(f"Relaying to: {target_url}")
            self.relay_request(target_url)
        elif self.path.startswith("/api/set"):
            # Relay GET /api/set?m=... to http://ESP_IP/set?m=...
            query = urllib.parse.urlparse(self.path).query
            target_url = f"http://{ESP_IP}/set?{query}"
            print(f"Relaying to: {target_url}")
            self.relay_request(target_url)
        elif self.path.startswith("/api/patterns"):
            # Relay GET /api/patterns to http://ESP_IP/api/patterns
            target_url = f"http://{ESP_IP}/api/patterns"
            print(f"Relaying to: {target_url}")
            self.relay_request(target_url)
        elif self.path.startswith("/api/setPanels"):
            # Relay GET /api/setPanels to http://ESP_IP/setPanels
            query = urllib.parse.urlparse(self.path).query
            target_url = f"http://{ESP_IP}/setPanels?{query}"
            print(f"Relaying to: {target_url}")
            self.relay_request(target_url)
        else:
            # Serve static files from the 'web' directory
            if self.path == "/" or self.path == "":
                self.path = "/pattern-designer.html"
            
            # Adjust path to serve from 'web' subdirectory if not already there
            # The script is run from project root, so we map /file to web/file
            if not self.path.startswith("/web/"):
                 self.path = "/web" + self.path
            
            return http.server.SimpleHTTPRequestHandler.do_GET(self)

    def do_POST(self):
        print(f"Received POST request: {self.path}")
        if self.path.startswith("/api/uploadPattern"):
            # Relay POST /api/uploadPattern to http://ESP_IP/uploadPattern
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            target_url = f"http://{ESP_IP}/uploadPattern"
            print(f"Relaying POST to: {target_url}")
            
            try:
                req = urllib.request.Request(target_url, data=post_data, method='POST')
                req.add_header('Content-Type', 'application/json')
                
                with urllib.request.urlopen(req) as response:
                    print(f"ESP response: {response.status}")
                    self.send_response(response.status)
                    self.send_header('Content-type', 'application/json')
                    self.end_headers()
                    self.wfile.write(response.read())
            except Exception as e:
                print(f"Proxy error: {str(e)}")
                self.send_error(500, f"Proxy error: {str(e)}")
        elif self.path.startswith("/api/uploadRaw"):
            # Relay POST /api/uploadRaw to http://ESP_IP/uploadRaw
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length)
            
            # Preserve query parameters (e.g. ?speed=80)
            query = urllib.parse.urlparse(self.path).query
            target_url = f"http://{ESP_IP}/uploadRaw?{query}"
            print(f"Relaying RAW POST to: {target_url}")
            
            try:
                req = urllib.request.Request(target_url, data=post_data, method='POST')
                req.add_header('Content-Type', 'application/octet-stream')
                
                with urllib.request.urlopen(req) as response:
                    print(f"ESP response: {response.status}")
                    self.send_response(response.status)
                    self.send_header('Content-type', 'text/plain')
                    self.end_headers()
                    self.wfile.write(response.read())
            except Exception as e:
                print(f"Proxy error: {str(e)}")
                self.send_error(500, f"Proxy error: {str(e)}")
        else:
            self.send_error(404, "Not Found")

    def relay_request(self, url):
        try:
            with urllib.request.urlopen(url) as response:
                print(f"ESP response: {response.status}")
                self.send_response(response.status)
                self.send_header('Content-type', 'text/plain')
                self.end_headers()
                self.wfile.write(response.read())
        except Exception as e:
            print(f"Proxy error: {str(e)}")
            self.send_error(500, f"Proxy error: {str(e)}")

print(f"Starting server on port {PORT}...")
print(f"Relaying commands to ESP8266 at {ESP_IP}")
print(f"Access at http://localhost:{PORT}/pattern-designer.html")

# Allow address reuse to prevent "Address already in use" errors
socketserver.TCPServer.allow_reuse_address = True

with socketserver.TCPServer(("", PORT), ProxyHTTPRequestHandler) as httpd:
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nServer stopped.")
