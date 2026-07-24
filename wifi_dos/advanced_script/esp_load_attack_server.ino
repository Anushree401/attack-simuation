#include <WiFi.h>
#include <WebServer.h>

// Set up a totally isolated, private Wi-Fi network name and password
const char* apSSID = "Wireless_Lab_Dashboard";
const char* apPassword = "SecureLabPassword123";

WebServer server(80);

// Telemetry state tracking
String currentStatus = "System Idle (Awaiting Selection)";
String selectedTarget = "None (Run Discovery First)";
int simulatedPacketsSent = 0;
bool simulationActive = false;

// HTML/CSS Dashboard Interface
const char HTML_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Wireless Lab Dashboard</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #1e1e24; color: #f0f0f5; text-align: center; padding: 20px; }
        .card { background-color: #2a2a35; padding: 20px; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.5); max-width: 500px; margin: 20px auto; }
        h1 { color: #4caf50; }
        .btn { display: block; width: 80%; margin: 15px auto; padding: 12px; background-color: #3b3b4f; color: white; border: none; border-radius: 4px; font-size: 16px; cursor: pointer; text-decoration: none;}
        .btn:hover { background-color: #4caf50; }
        .btn-stop { background-color: #f44336; }
        .btn-stop:hover { background-color: #d32f2f; }
        .status-box { font-weight: bold; padding: 10px; border: 1px solid #444; border-radius: 4px; background: #111; color: #00ff00; }
    </style>
</head>
<body>
    <div class='card'>
        <h1>Wireless Lab Simulation Hub</h1>
        <hr style='border-color: #444;'>
        <h3>Target Profile</h3>
        <p><strong>Active Network Target:</strong> <span id='target'>Loading...</span></p>
        <p><strong>Engine State:</strong> <span id='status' class='status-box'>Loading...</span></p>
        <p><strong>Simulated Frames Injected:</strong> <span id='counter'>0</span></p>
        <hr style='border-color: #444;'>
        <a href='/scan' class='btn'>1. Execute Network Discovery Scan</a>
        <a href='/beacon' class='btn'>2. Launch Beacon Flood Simulation</a>
        <a href='/deauth' class='btn'>3. Launch Deauthentication Simulation</a>
        <a href='/stop' class='btn btn-stop'>Abruptly Halt Simulation Engine</a>
    </div>

    <script>
        // Keep status data fresh every 2 seconds without reloading page
        setInterval(function() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('status').innerText = data.status;
                    document.getElementById('target').innerText = data.target;
                    document.getElementById('counter').innerText = data.count;
                });
        }, 2000);
    </script>
</body>
</html>
)=====";

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void handleStatusAPI() {
  // Simulate transmission counting safely if active
  if (simulationActive) {
    simulatedPacketsSent += random(15, 45);
  }
  String json = "{\"status\":\"" + currentStatus + "\",\"target\":\"" + selectedTarget + "\",\"count\":" + String(simulatedPacketsSent) + "}";
  server.send(200, "application/json", json);
}

void handleScan() {
  currentStatus = "Scanning 2.4GHz Airwaves...";
  // Safe scan wrapper
  int n = WiFi.scanNetworks();
  if(n > 0) {
    selectedTarget = WiFi.SSID(0) + " [Ch: " + String(WiFi.channel(0)) + "]";
    currentStatus = "Target Profile Auto-Locked Successfully.";
  } else {
    selectedTarget = "No Authorized Lab Target Found.";
    currentStatus = "Scan Complete. Environment Empty.";
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleBeacon() {
  simulationActive = true;
  currentStatus = "Simulating Beacon Flood (DIAGNOSTIC MODE)";
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDeauth() {
  if (selectedTarget == "No Authorized Lab Target Found." || selectedTarget == "None (Run Discovery First)") {
    currentStatus = "Aborted: Define safe lab target target first.";
  } else {
    simulationActive = true;
    currentStatus = "Simulating Deauth Injection (DIAGNOSTIC MODE)";
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleStop() {
  simulationActive = false;
  simulatedPacketsSent = 0;
  currentStatus = "System Idle (Awaiting Selection)";
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("[*] Initializing Safe Wireless Lab Web Host...");

  // Spin up password protected private network environment access point
  WiFi.softAP(apSSID, apPassword);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("[+] Private Control Network Active. SSID: ");
  Serial.println(apSSID);
  Serial.print("[+] Direct Dashboard Link: http://");
  Serial.println(myIP);

  // Link system route navigation paths
  server.on("/", handleRoot);
  server.on("/api/status", handleStatusAPI);
  server.on("/scan", handleScan);
  server.on("/beacon", handleBeacon);
  server.on("/deauth", handleDeauth);
  server.on("/stop", handleStop);

  server.begin();
  Serial.println("[+] Control server active and listening for web connections.");
}

void loop() {
  server.handleClient();
  delay(2); // Yield execution time back to core background OS tasks
}
