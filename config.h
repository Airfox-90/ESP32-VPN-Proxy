#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Preferences.h>

struct Config {
  char wifi_ssid[64];
  char wifi_pass[64];

  // Secondary WiFi (fallback)
  char wifi_ssid_secondary[64];
  char wifi_pass_secondary[64];

  // WireGuard
  char wg_local_ip[32];
  char wg_dns_ip[32];
  char wg_private_key[128];
  char wg_peer_pubkey[128];
  char wg_endpoint[128];
  char wg_allowed_ips[64];
  bool wg_enabled;

  // Port forward
  uint16_t forward_local_port;
  char forward_remote_host[64];
  uint16_t forward_remote_port;
  
  // Web UI Password
  char web_password[64];

  // MQTT Configuration
  bool mqtt_enabled;
  char mqtt_server[64];
  uint16_t mqtt_port;
  char mqtt_username[64];
  char mqtt_password[64];
  char mqtt_topic_prefix[128];
};

extern Config cfg;
extern Preferences prefs;

void loadConfig();
void saveConfig();

#endif