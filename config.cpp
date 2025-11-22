#include "config.h"

Config cfg;
Preferences prefs;

void loadConfig() {
  prefs.begin("wgdev", true);
  prefs.getString("ssid", "").toCharArray(cfg.wifi_ssid, sizeof(cfg.wifi_ssid));
  prefs.getString("pass", "").toCharArray(cfg.wifi_pass, sizeof(cfg.wifi_pass));
  prefs.getString("wg_local", "").toCharArray(cfg.wg_local_ip, sizeof(cfg.wg_local_ip));
  prefs.getString("wg_dns", "").toCharArray(cfg.wg_dns_ip, sizeof(cfg.wg_dns_ip));
  prefs.getString("wg_priv", "").toCharArray(cfg.wg_private_key, sizeof(cfg.wg_private_key));
  prefs.getString("wg_peer", "").toCharArray(cfg.wg_peer_pubkey, sizeof(cfg.wg_peer_pubkey));
  prefs.getString("wg_ep", "").toCharArray(cfg.wg_endpoint, sizeof(cfg.wg_endpoint));
  prefs.getString("wg_ai", "").toCharArray(cfg.wg_allowed_ips, sizeof(cfg.wg_allowed_ips));
  cfg.wg_enabled = prefs.getBool("wg_on", false);

  cfg.forward_local_port = prefs.getUShort("fwd_lp", 0);
  prefs.getString("fwd_rh", "").toCharArray(cfg.forward_remote_host, sizeof(cfg.forward_remote_host));
  cfg.forward_remote_port = prefs.getUShort("fwd_rp", 0);
  
  prefs.getString("web_pass", "").toCharArray(cfg.web_password, sizeof(cfg.web_password));
  prefs.end();
}

void saveConfig() {
  prefs.begin("wgdev", false);
  prefs.putString("ssid", String(cfg.wifi_ssid));
  prefs.putString("pass", String(cfg.wifi_pass));
  prefs.putString("wg_local", String(cfg.wg_local_ip));
  prefs.putString("wg_dns", String(cfg.wg_dns_ip));
  prefs.putString("wg_priv", String(cfg.wg_private_key));
  prefs.putString("wg_peer", String(cfg.wg_peer_pubkey));
  prefs.putString("wg_ep", String(cfg.wg_endpoint));
  prefs.putString("wg_ai", String(cfg.wg_allowed_ips));
  prefs.putBool("wg_on", cfg.wg_enabled);
  prefs.putUShort("fwd_lp", cfg.forward_local_port);
  prefs.putString("fwd_rh", String(cfg.forward_remote_host));
  prefs.putUShort("fwd_rp", cfg.forward_remote_port);
  prefs.putString("web_pass", String(cfg.web_password));
  prefs.end();
}