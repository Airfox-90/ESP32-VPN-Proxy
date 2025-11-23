#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "webserver.h"
#include "webui.h"
#include "config.h"
#include "WGProject.h"
#include "forwarder.h"
#include "mqtt_client.h"

AsyncWebServer server(80);
SessionToken activeSession;

// Session timeout
const unsigned long SESSION_TIMEOUT = 3600000; // 1 hour in ms

String strReplace(String tpl, const char* key, const char* val) {
  tpl.replace(key, val);
  return tpl;
}

String generateSessionToken() {
  String token = "";
  for (int i = 0; i < 32; i++) {
    token += String(random(0, 16), HEX);
  }
  return token;
}

void createSession() {
  activeSession.token = generateSessionToken();
  activeSession.expireTime = millis() + SESSION_TIMEOUT;
}

bool isSessionValid() {
  if (activeSession.token.length() == 0) return false;
  if (millis() > activeSession.expireTime) {
    activeSession.token = "";
    return false;
  }
  return true;
}

bool checkSession(AsyncWebServerRequest *request) {
  if (request->hasHeader("Cookie")) {
    String cookie = request->header("Cookie");
    if (cookie.indexOf("session=" + activeSession.token) != -1) {
      return isSessionValid();
    }
  }
  return false;
}

bool protectedCheck(AsyncWebServerRequest *request) {
  if (strlen(cfg.web_password) > 0 && !checkSession(request)) {
    request->send(403, "text/plain", "Unauthorized");
    return false;
  }
  return true;
}

void handleRoot(AsyncWebServerRequest *request) {
  if (strlen(cfg.web_password) > 0 && !checkSession(request)) {
    request->redirect("/login");
    return;
  }

  String html = String(PAGE_INDEX);
  html = strReplace(html, "%SSID%", cfg.wifi_ssid);
  html = strReplace(html, "%PASS%", cfg.wifi_pass);
  html = strReplace(html, "%SSID2%", cfg.wifi_ssid_secondary);
  html = strReplace(html, "%PASS2%", cfg.wifi_pass_secondary);
  html = strReplace(html, "%WG_LOCAL%", cfg.wg_local_ip);
  html = strReplace(html, "%WG_DNS%", cfg.wg_dns_ip);
  html = strReplace(html, "%WG_PRIV%", cfg.wg_private_key);
  html = strReplace(html, "%WG_PEER%", cfg.wg_peer_pubkey);
  html = strReplace(html, "%WG_EP%", cfg.wg_endpoint);
  html = strReplace(html, "%WG_AI%", cfg.wg_allowed_ips);
  html = strReplace(html, "%WG_TOGGLE%", cfg.wg_enabled ? "WireGuard aus" : "WireGuard an");
  html = strReplace(html, "%FWD_LPORT%", String(cfg.forward_local_port).c_str());
  html = strReplace(html, "%FWD_RHOST%", cfg.forward_remote_host);
  html = strReplace(html, "%FWD_RPORT%", String(cfg.forward_remote_port).c_str());
  html = strReplace(html, "%MQTT_SERVER%", cfg.mqtt_server);
  html = strReplace(html, "%MQTT_PORT%", String(cfg.mqtt_port).c_str());
  html = strReplace(html, "%MQTT_USER%", cfg.mqtt_username);
  html = strReplace(html, "%MQTT_PASS%", cfg.mqtt_password);
  html = strReplace(html, "%MQTT_PREFIX%", cfg.mqtt_topic_prefix);
  html = strReplace(html, "%MQTT_TOGGLE%", cfg.mqtt_enabled ? "MQTT aus" : "MQTT an");
  html = strReplace(html, "%SETTINGS_PWD%", cfg.web_password);
  html = strReplace(html, "%LOGOUT_BTN%", strlen(cfg.web_password) > 0 ? "<a href=\"/logout\" style=\"display: inline-block; margin-top: 10px; padding: 10px 20px; background: #999; color: white; text-decoration: none; border-radius: 6px; font-weight: 600;\">Logout</a>" : "");
  
  auto resp = request->beginResponse(200, "text/html", html);
  resp->addHeader("Set-Cookie", "session=" + activeSession.token + "; Path=/; HttpOnly; Max-Age=3600");
  request->send(resp);
}

void handleLogin(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_GET) {
    String html = String(PAGE_LOGIN);
    html = strReplace(html, "%ERROR_MSG%", "");
    auto resp = request->beginResponse(200, "text/html", html);
    request->send(resp);
    return;
  }

  if (request->hasParam("password", true)) {
    String submittedPass = request->getParam("password", true)->value();
    if (submittedPass == String(cfg.web_password)) {
      createSession();
      auto resp = request->beginResponse(302);
      resp->addHeader("Set-Cookie", "session=" + activeSession.token + "; Path=/; HttpOnly; Max-Age=3600");
      resp->addHeader("Location", "/");
      request->send(resp);
      return;
    }
  }
  request->redirect("/login?error=1");
}

void handleLogout(AsyncWebServerRequest *request) {
  activeSession.token = "";
  auto resp = request->beginResponse(302);
  resp->addHeader("Set-Cookie", "session=; Path=/; HttpOnly; Max-Age=0");
  resp->addHeader("Location", "/login");
  request->send(resp);
}

void handleSaveWifi(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  if (request->hasParam("ssid", true)) {
    request->getParam("ssid", true)->value().toCharArray(cfg.wifi_ssid, sizeof(cfg.wifi_ssid));
    request->getParam("pass", true)->value().toCharArray(cfg.wifi_pass, sizeof(cfg.wifi_pass));
    
    // Save secondary WiFi if provided
    if (request->hasParam("ssid2", true)) {
      request->getParam("ssid2", true)->value().toCharArray(cfg.wifi_ssid_secondary, sizeof(cfg.wifi_ssid_secondary));
      request->getParam("pass2", true)->value().toCharArray(cfg.wifi_pass_secondary, sizeof(cfg.wifi_pass_secondary));
    }
    
    saveConfig();
    request->redirect("/");
    startWiFi();
  } else {
    request->send(400, "text/plain", "missing ssid");
  }
}

void handleSaveWG(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  if (request->hasParam("wg_local", true)) request->getParam("wg_local", true)->value().toCharArray(cfg.wg_local_ip, sizeof(cfg.wg_local_ip));
  if (request->hasParam("wg_dns", true)) request->getParam("wg_dns", true)->value().toCharArray(cfg.wg_dns_ip, sizeof(cfg.wg_dns_ip));
  if (request->hasParam("priv", true)) request->getParam("priv", true)->value().toCharArray(cfg.wg_private_key, sizeof(cfg.wg_private_key));
  if (request->hasParam("peer", true)) request->getParam("peer", true)->value().toCharArray(cfg.wg_peer_pubkey, sizeof(cfg.wg_peer_pubkey));
  if (request->hasParam("ep", true)) request->getParam("ep", true)->value().toCharArray(cfg.wg_endpoint, sizeof(cfg.wg_endpoint));
  if (request->hasParam("ai", true)) request->getParam("ai", true)->value().toCharArray(cfg.wg_allowed_ips, sizeof(cfg.wg_allowed_ips));
  saveConfig();
  request->redirect("/");
}

void handleWgToggle(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  if (!wg_ready) {
    request->send(500, "text/plain", "WG backend not available");
    return;
  }
  
  if (cfg.wg_enabled) {
    wg_down_wrapper();
    restoreDefaultDns();
    cfg.wg_enabled = false;
  } else {
    if (!wg_applyConfig_validate(cfg.wg_local_ip, cfg.wg_private_key, cfg.wg_peer_pubkey, cfg.wg_endpoint, cfg.wg_allowed_ips)) {
      request->send(500, "text/plain", "WG configuration invalid");
      return;
    }
    if (!wg_up_wrapper()) {
      request->send(500, "text/plain", "WG up failed");
      return;
    }
    cfg.wg_enabled = true;
    setVpnDnsFromConfig();
  }
  
  saveConfig();
  request->redirect("/");
}

void handleSaveFwd(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  if (request->hasParam("lport", true)) cfg.forward_local_port = request->getParam("lport", true)->value().toInt();
  if (request->hasParam("rhost", true)) request->getParam("rhost", true)->value().toCharArray(cfg.forward_remote_host, sizeof(cfg.forward_remote_host));
  if (request->hasParam("rport", true)) cfg.forward_remote_port = request->getParam("rport", true)->value().toInt();
  saveConfig();
  request->redirect("/");
}

void handleStatus(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  String html = String(PAGE_STATUS);
  html = strReplace(html, "%LOCAL_IP%", WiFi.localIP().toString().c_str());
  html = strReplace(html, "%WIFI_SSID%", WiFi.SSID().c_str());
  html = strReplace(html, "%TEMPERATURE%", String(currentTemperature, 1).c_str());
  html = strReplace(html, "%HUMIDITY%", String(currentHumidity, 1).c_str());
  WGStats s = wg_get_stats_wrapper();
  html = strReplace(html, "%WG_STATE%", s.online ? "Connected" : "Offline");
  html = strReplace(html, "%WG_RX%", String(bytesRecvSince).c_str());
  html = strReplace(html, "%WG_TX%", String(bytesSentSince).c_str());
  html = strReplace(html, "%THR%", String(lastKbps, 2).c_str());
  html = strReplace(html, "%F_LPORT%", String(cfg.forward_local_port).c_str());
  html = strReplace(html, "%F_RHOST%", cfg.forward_remote_host);
  html = strReplace(html, "%F_RPORT%", String(cfg.forward_remote_port).c_str());
  
  auto resp = request->beginResponse(200, "text/html", html);
  resp->addHeader("Set-Cookie", "session=" + activeSession.token + "; Path=/; HttpOnly; Max-Age=3600");
  request->send(resp);
}

void handleSaveSettings(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;
  
  if (request->hasParam("settings_pwd", true)) {
    request->getParam("settings_pwd", true)->value().toCharArray(cfg.web_password, sizeof(cfg.web_password));
    saveConfig();
    request->redirect("/");
  } else {
    request->send(400, "text/plain", "missing settings_pwd");
  }
}

void handleReboot(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;
  
  // Send response before rebooting
  request->send(200, "text/html", "<!DOCTYPE html><html><head><meta charset='utf-8'><title>Rebooting...</title><style>body{font-family:Arial,sans-serif;display:flex;justify-content:center;align-items:center;height:100vh;margin:0;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%)}div{text-align:center;color:white}.spinner{border:4px solid rgba(255,255,255,0.3);border-top:4px solid white;border-radius:50%;width:40px;height:40px;animation:spin 1s linear infinite;margin:20px auto}@keyframes spin{0%{transform:rotate(0deg)}100%{transform:rotate(360deg)}}</style></head><body><div><div class='spinner'></div><h1>Rebooting Device...</h1><p>Please wait, the device will restart in a few seconds.</p></body></html>");
  
  // Schedule reboot after a short delay
  delay(500);
  ESP.restart();
}

void handleSaveMqtt(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  if (request->hasParam("mqtt_server", true)) request->getParam("mqtt_server", true)->value().toCharArray(cfg.mqtt_server, sizeof(cfg.mqtt_server));
  if (request->hasParam("mqtt_port", true)) cfg.mqtt_port = request->getParam("mqtt_port", true)->value().toInt();
  if (request->hasParam("mqtt_user", true)) request->getParam("mqtt_user", true)->value().toCharArray(cfg.mqtt_username, sizeof(cfg.mqtt_username));
  if (request->hasParam("mqtt_pass", true)) request->getParam("mqtt_pass", true)->value().toCharArray(cfg.mqtt_password, sizeof(cfg.mqtt_password));
  if (request->hasParam("mqtt_prefix", true)) request->getParam("mqtt_prefix", true)->value().toCharArray(cfg.mqtt_topic_prefix, sizeof(cfg.mqtt_topic_prefix));
  
  saveConfig();
  request->redirect("/");
}

void handleMqttToggle(AsyncWebServerRequest *request) {
  if (!protectedCheck(request)) return;

  cfg.mqtt_enabled = !cfg.mqtt_enabled;
  saveConfig();
  
  // Re-initialize MQTT client with new settings
  mqtt_init();
  
  request->redirect("/");
}


void setupWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/login", HTTP_GET, handleLogin);
  server.on("/login", HTTP_POST, handleLogin);
  server.on("/logout", HTTP_GET, handleLogout);
  server.on("/save_wifi", HTTP_POST, handleSaveWifi);
  server.on("/save_wg", HTTP_POST, handleSaveWG);
  server.on("/wg_toggle", HTTP_POST, handleWgToggle);
  server.on("/save_fwd", HTTP_POST, handleSaveFwd);
  server.on("/save_mqtt", HTTP_POST, handleSaveMqtt);
  server.on("/mqtt_toggle", HTTP_POST, handleMqttToggle);
  server.on("/save_settings", HTTP_POST, handleSaveSettings);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/reboot", HTTP_POST, handleReboot);
  server.begin();
}