#include <WiFi.h>
#include "config.h"

uint64_t bytesSentSince = 0;
uint64_t bytesRecvSince = 0;
bool forwarder_connected = false;

TaskHandle_t forwarderTask = NULL;

void forwarderTaskFn(void* arg) {
  WiFiServer *srv = (WiFiServer*)arg;
  srv->begin();
  
  while (true) {
    WiFiClient client = srv->available();
    if (client) {
      log_i("Local client connected");
      forwarder_connected = true;
      WiFiClient remote;
      
      if (!remote.connect(cfg.forward_remote_host, cfg.forward_remote_port)) {
        log_e("Unable to connect to remote host");
        client.stop();
        remote.stop();
        forwarder_connected = false;
        bytesSentSince = 0;
        bytesRecvSince = 0;
        vTaskDelay(pdMS_TO_TICKS(100));
        continue;
      }
      
      log_i("Remote host connected");
      
      while (client.connected() && remote.connected()) {
        if (client.available()) {
          uint8_t buf[512];
          int r = client.read(buf, sizeof(buf));
          if (r > 0) {
            remote.write(buf, r);
            bytesSentSince += r;
          }
        }
        
        if (remote.available()) {
          uint8_t buf[512];
          int r = remote.read(buf, sizeof(buf));
          if (r > 0) {
            client.write(buf, r);
            bytesRecvSince += r;
          }
        }
        
        vTaskDelay(pdMS_TO_TICKS(1));
      }
      
      log_i("Clients disconnected");
      remote.stop();
      client.stop();
      bytesSentSince = 0;
      bytesRecvSince = 0;
      forwarder_connected = false;
    } else {
      vTaskDelay(pdMS_TO_TICKS(10));
    }
  }
}

void startForwarder() {
  if (cfg.forward_local_port == 0 || strlen(cfg.forward_remote_host) == 0 || cfg.forward_remote_port == 0) {
    return;
  }
  
  static WiFiServer *srv = NULL;
  if (srv) return;
  
  srv = new WiFiServer(cfg.forward_local_port);
  xTaskCreatePinnedToCore(forwarderTaskFn, "forwarder", 8*1024, srv, 1, &forwarderTask, APP_CPU_NUM);
}
