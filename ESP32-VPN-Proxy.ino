#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <WiFiUdp.h>
#include <WireGuard-ESP32.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <TFT_eSPI.h> 
#include "lwip/dns.h"
#include "WGProject.h"
#include "webui.h"
#include "config.h"
#include "forwarder.h"
#include "webserver.h"

#define AP_SSID_PREFIX "WG-VPN-Proxy_"
#define AP_PASSWORD    "configureme"

// throughput counters
uint64_t lastBytesSent = 0;
uint64_t lastBytesRecv = 0;
float lastKbps = 0;

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite framebuffers = TFT_eSprite(&tft);
LilyGo_Class amoled;

WireGuard wg;
bool wg_ready = false;
bool wg_running = false;

bool wifiInitialized = false;
bool wifiConnected = false;

bool wg_begin_wrapper() {
  wg_ready = true;
  return true;
}

bool wg_applyConfig_validate(const char* local_ip_c, const char* privateKey, const char* peerPublicKey, const char* endpoint, const char* allowedIps) {
  if (!wg_ready) return false;
  
  String ep = String(endpoint);
  int colon = ep.indexOf(':');
  if (colon < 0) {
    Serial.println("WG: endpoint format must be host:port");
    return false;
  }
  
  String host = ep.substring(0, colon);
  uint16_t port = ep.substring(colon + 1).toInt();
  if (port == 0) {
    Serial.println("WG: invalid endpoint port");
    return false;
  }
  
  IPAddress epIp;
  if (!WiFi.hostByName(host.c_str(), epIp)) {
    Serial.println("WG: could not resolve endpoint host (DNS)");
    return false;
  }
  
  String localip = String(local_ip_c);
  if (localip.length() < 7) {
    Serial.println("WG: local IP seems invalid");
    return false;
  }
  
  return true;
}

bool parseCIDR(const char* cidr, IPAddress &ip, IPAddress &mask) {
  char buf[20];
  strncpy(buf, cidr, sizeof(buf));
  buf[sizeof(buf)-1] = '\0';

  char* slash = strchr(buf, '/');
  if (!slash) return false;

  *slash = '\0';
  const char* prefixStr = slash + 1;

  ip.fromString(buf);

  int prefix = atoi(prefixStr);
  if (prefix < 0 || prefix > 32) return false;

  uint32_t maskInt = (prefix == 0) ? 0 : 0xFFFFFFFF << (32 - prefix);
  mask = IPAddress((maskInt >> 24) & 0xFF, 
                   (maskInt >> 16) & 0xFF, 
                   (maskInt >> 8) & 0xFF, 
                   maskInt & 0xFF);

  return true;
}

bool wg_up_wrapper() {
  if (!wg_ready) return false;

  String ep = String(cfg.wg_endpoint);
  int colon = ep.indexOf(':');
  if (colon < 0) {
    Serial.println("WG up: endpoint missing :port");
    return false;
  }
  
  String host = ep.substring(0, colon);
  uint16_t port = ep.substring(colon + 1).toInt();
  IPAddress epIp;
  
  if (!WiFi.hostByName(host.c_str(), epIp)) {
    Serial.println("WG up: DNS failed");
    return false;
  }
  
  IPAddress ip, mask;
  if (parseCIDR(cfg.wg_local_ip, ip, mask)) {
    Serial.print("IP: ");
    Serial.println(ip);
    Serial.print("Mask: ");
    Serial.println(mask);
  } else {
    Serial.println("Error parsing CIDR!");
    return false;
  }
  
  auto gateway = IPAddress(0, 0, 0, 0);
  bool ok = wg.begin(ip, mask, gateway,
                     String(cfg.wg_private_key).c_str(),
                     epIp.toString().c_str(),
                     String(cfg.wg_peer_pubkey).c_str(),
                     port);
  
  if (ok) {
    wg_running = true;
    Serial.println("WireGuard: begin OK");
  } else {
    Serial.println("WireGuard: begin FAILED");
  }
  
  return ok;
}

bool wg_down_wrapper() {
  wg.end();
  wg_running = false;
  return true;
}

WGStats wg_get_stats_wrapper() {
  WGStats s;
  s.online = wg_running;
  return s;
}

void setVpnDnsFromConfig() {
  if (strlen(cfg.wg_dns_ip) == 0) {
    Serial.println("No WG DNS configured");
    return;
  }

  IPAddress dnsIp;
  if (!dnsIp.fromString(cfg.wg_dns_ip)) {
    Serial.println("WG DNS IP invalid");
    return;
  }

  ip_addr_t dns;
  dns.type = IPADDR_TYPE_V4;
  dns.u_addr.ip4.addr = (uint32_t)dnsIp;
  dns_setserver(0, &dns);

  Serial.printf("DNS set to %s\n", cfg.wg_dns_ip);
}

void restoreDefaultDns() {
  ip_addr_t dns;
  dns.type = IPADDR_TYPE_V4;
  dns.u_addr.ip4.addr = WiFi.dnsIP(0);
  dns_setserver(0, &dns);
  Serial.println("DNS restored");
}

void initializeDisplay() {
  bool rslt = amoled.begin();
  if (!rslt) {
    while (1) {
      Serial.println("Display initialization failed!");
      delay(1000);
    }
  }
  
  framebuffers.createSprite(amoled.width(), amoled.height());
  framebuffers.setSwapBytes(1);
}

bool connectToWiFi() {
  if (strlen(cfg.wifi_ssid) > 0) {
    WiFi.begin(cfg.wifi_ssid, cfg.wifi_pass);
    unsigned long start = millis();
    
    while (millis() - start < 10000) {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(200);
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected. Shutting down AP");
      WiFi.softAPdisconnect(true);
      return true;
    }
  }
  WiFi.disconnect();
  Serial.println("Giving up on WiFi");
  return false;
}

void showInitMessage() {
  framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), TFT_BLACK);
  framebuffers.setTextColor(TFT_GREEN, TFT_BLACK);
  framebuffers.drawString("Waiting for WiFi and time sync...", 20, 50, 4);
  amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());
}

void syncTime() {
  const long gmtOffset_sec = 0;
  const int daylightOffset_sec = 0;

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.google.com");

  Serial.print("Waiting for time synchronization...");
  time_t now = time(nullptr);
  int retry = 0;
  
  while (now < 1609459200UL && retry < 20) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    retry++;
  }

  if (now < 1609459200UL) {
    Serial.println(" Time sync failed!");
  } else {
    Serial.println(" Time synchronized!");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.printf("Current time: %02d:%02d:%02d UTC\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  }
}

const char* formatBytes(uint64_t bytes, char* buffer, size_t bufferSize) {
  if (bytes < 1024) {
    snprintf(buffer, bufferSize, "%llu B", bytes);
  } else if (bytes < (1024ULL * 1024ULL)) {
    double kb = bytes / 1024.0;
    snprintf(buffer, bufferSize, "%.2f kB", kb);
  } else {
    double mb = bytes / 1024.0 / 1024.0;
    snprintf(buffer, bufferSize, "%.2f MB", mb);
  }
  return buffer;
}

void drawStatusOnDisplay() {
  WGStats s = wg_get_stats_wrapper();
  struct tm timeinfo;
  getLocalTime(&timeinfo);

  char buffer[64];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
  String timeStr = String(buffer);

  char rxBuffer[32];
  char txBuffer[32];
  
  framebuffers.fillRect(0, 0, amoled.width(), amoled.height(), 0x0C21);
  
  // Header
  framebuffers.fillRect(0, 0, amoled.width(), 45, TFT_DARKGREEN);
  framebuffers.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  framebuffers.drawString("VPN Relay Status", 10, 12, 4);
  
  // WiFi Signal
  int32_t rssi = WiFi.RSSI();
  int bars = 0;
  if (rssi >= -50) bars = 4;
  else if (rssi >= -60) bars = 3;
  else if (rssi >= -70) bars = 2;
  else if (rssi >= -80) bars = 1;
  else bars = 0;
  
  uint16_t wifiColor = (bars >= 3) ? 0x07E0 : (bars >= 2) ? 0xFD20 : 0xF800;
  int centerX = amoled.width() / 2;
  
  framebuffers.setTextColor(wifiColor, TFT_DARKGREEN);
  framebuffers.drawString("WiFi", centerX - 30, 12, 4);
  
  for (int i = 0; i < 4; i++) {
    int barX = centerX + 35 + (i * 6);
    int barHeight = 2 + (i * 4);
    uint16_t barColor = (i < bars) ? wifiColor : TFT_DARKGREEN;
    framebuffers.fillRect(barX, 30 - barHeight, 4, barHeight, barColor);
  }
  
  framebuffers.setTextColor(TFT_WHITE, TFT_DARKGREEN);
  framebuffers.drawString(timeStr.c_str(), amoled.width() - 105, 12, 4);
  
  // IP Card
  framebuffers.fillRect(8, 55, 250, 55, 0x2945);
  framebuffers.drawRect(8, 55, 250, 55, 0x667E);
  framebuffers.setTextColor(0xB5D6, 0x2945);
  framebuffers.drawString("IP Address", 15, 60, 2);
  framebuffers.setTextColor(TFT_WHITE, 0x2945);
  framebuffers.drawString(WiFi.localIP().toString().c_str(), 15, 77, 4);
  
  // DNS Card
  framebuffers.fillRect(8, 115, 250, 55, 0x2945);
  framebuffers.drawRect(8, 115, 250, 55, 0x667E);
  framebuffers.setTextColor(0xB5D6, 0x2945);
  framebuffers.drawString("DNS Server", 15, 120, 2);
  framebuffers.setTextColor(TFT_WHITE, 0x2945);
  framebuffers.drawString(WiFi.dnsIP().toString().c_str(), 15, 140, 4);

  // Forwarder Card
  framebuffers.fillRect(8, 175, 250, 55, 0x2945);
  framebuffers.drawRect(8, 175, 250, 55, 0x667E);
  framebuffers.setTextColor(0xB5D6, 0x2945);
  framebuffers.drawString("Forwarder", 15, 180, 2);
  framebuffers.setTextColor(TFT_WHITE, 0x2945);
  String fwd = String(cfg.forward_remote_host) + ":" + String(cfg.forward_remote_port);
  framebuffers.drawString(fwd.c_str(), 15, 200, 4);
  
  // WireGuard Status
  uint16_t wgColor = wg.is_initialized() ? 0x07E0 : 0xF800;
  uint16_t wgBgColor = wg.is_initialized() ? 0x0420 : 0x4800;
  framebuffers.fillRect(270, 55, 123, 35, wgBgColor);
  framebuffers.drawRect(270, 55, 123, 35, wgColor);
  framebuffers.setTextColor(wgColor, wgBgColor);
  framebuffers.drawString("WireGuard", 280, 57, 2);
  framebuffers.setTextColor(TFT_WHITE, wgBgColor);
  String wgState = wg.is_initialized() ? "CONNECTED" : "OFFLINE";
  framebuffers.drawString(wgState.c_str(), 280, 70, 2);
  
  // Forwarder Status
  uint16_t fwdColor = forwarder_connected ? 0x07E0 : 0xFD20;
  uint16_t fwdBgColor = forwarder_connected ? 0x0420 : 0x5400;
  framebuffers.fillRect(403, 55, 123, 35, fwdBgColor);
  framebuffers.drawRect(403, 55, 123, 35, fwdColor);
  framebuffers.setTextColor(fwdColor, fwdBgColor);
  framebuffers.drawString("Port Fwd", 411, 57, 2);
  framebuffers.setTextColor(TFT_WHITE, fwdBgColor);
  String fwdState = forwarder_connected ? "ACTIVE" : "IDLE";
  framebuffers.drawString(fwdState.c_str(), 411, 70, 2);
  
  // Data Transfer
  framebuffers.fillRect(270, 100, 256, 130, 0x2945);
  framebuffers.drawRect(270, 100, 256, 130, 0x667E);
  
  framebuffers.setTextColor(0xB5D6, 0x2945);
  framebuffers.drawString("Data Transfer", 280, 107, 2);
  
  framebuffers.setTextColor(TFT_WHITE, 0x2945);
  framebuffers.drawString("Recv:", 280, 130, 4);
  framebuffers.drawString(formatBytes(bytesRecvSince, rxBuffer, sizeof(rxBuffer)), 380, 130, 4);
  
  framebuffers.drawString("Sent:", 280, 160, 4);
  framebuffers.drawString(formatBytes(bytesSentSince, txBuffer, sizeof(txBuffer)), 380, 160, 4);
  
  framebuffers.setTextColor(0xFFE0, 0x2945);
  framebuffers.drawString("Speed:", 280, 190, 4);
  framebuffers.setTextColor(TFT_WHITE, 0x2945);
  String speedStr = String(lastKbps, 1) + " kb/s";
  framebuffers.drawString(speedStr.c_str(), 380, 190, 4);
  
  amoled.pushColors(0, 0, amoled.width(), amoled.height(), (uint16_t *)framebuffers.getPointer());
}

void setup() {
  Serial.begin(115200);

  Serial.println("Starting setup...");
  
  initializeDisplay();
  loadConfig();
  
  // Start AP
  String apName = String(AP_SSID_PREFIX) + String((uint32_t)ESP.getEfuseMac(), HEX);
  WiFi.softAP(apName.c_str(), AP_PASSWORD);
  
  Serial.println("Setup WebServer");
  setupWebServer();
  
  // Try to connect to WiFi
  Serial.println("Try connect Wifi...");
  bool connectedToWiFi = connectToWiFi();
  
  showInitMessage();
  
  wifiInitialized = true;
  wifiConnected = connectedToWiFi;
}

void loop() {
  unsigned long now = millis();
  
  // Monitor WiFi connection state changes
  static bool lastWiFiState = false;
  bool currentWiFiState = (WiFi.status() == WL_CONNECTED);
  
  // WiFi just connected
  if (currentWiFiState && !lastWiFiState && wifiInitialized) {
    Serial.println("WiFi connected - starting services...");
    wifiConnected = true;
    
    syncTime();
    wg_begin_wrapper();
    
    if (cfg.wg_enabled) {
      if (wg_applyConfig_validate(cfg.wg_local_ip, cfg.wg_private_key, cfg.wg_peer_pubkey, cfg.wg_endpoint, cfg.wg_allowed_ips)) {
        wg_up_wrapper();
      } else {
        Serial.println("Saved WG config invalid — please reconfigure via AP.");
      }
    }
    startForwarder();
  }
  
  // WiFi disconnected
  if (!currentWiFiState && lastWiFiState) {
    Serial.println("WiFi disconnected - stopping services...");
    wifiConnected = false;
    
    if (wg_running) {
      wg_down_wrapper();
      restoreDefaultDns();
    }
  }
  
  lastWiFiState = currentWiFiState;
  
  // Calculate throughput
  static unsigned long lastThroughputCalc = 0;
  if (now - lastThroughputCalc >= 1000) {
    lastThroughputCalc = now;
    uint64_t dtx = bytesSentSince - lastBytesSent;
    uint64_t drx = bytesRecvSince - lastBytesRecv;
    lastBytesSent = bytesSentSince;
    lastBytesRecv = bytesRecvSince;
    lastKbps = (dtx + drx) * 8.0 / 1000.0;
  }

  // Update display
  static unsigned long lastDisp = 0;
  if (wifiConnected && now - lastDisp > 1000) {
    lastDisp = now;
    drawStatusOnDisplay();
  }

  delay(1);
}