#ifndef WGPROJECT_H
#define WGPROJECT_H

typedef struct WGStats {
  bool online;
  bool forwarder_connected;
};

extern bool wg_ready;
extern float lastKbps;

WGStats wg_get_stats_wrapper();
bool wg_down_wrapper();
void restoreDefaultDns();
void setVpnDnsFromConfig();
bool wg_up_wrapper();
bool wg_applyConfig_validate(const char* local_ip_c, const char* privateKey, const char* peerPublicKey, const char* endpoint, const char* allowedIps);
void startWiFi();

#endif