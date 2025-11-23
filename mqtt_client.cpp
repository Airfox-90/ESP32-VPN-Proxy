#include "mqtt_client.h"
#include "config.h"
#include <PubSubClient.h>
#include <WiFi.h>

static WiFiClient espClient;
static PubSubClient mqttClient(espClient);
static unsigned long lastMqttPublish = 0;
static unsigned long lastMqttConnectionAttempt = 0;
static const unsigned long MQTT_PUBLISH_INTERVAL = 30000; // Publish every 30 seconds
static const unsigned long MQTT_RECONNECT_INTERVAL = 5000; // Try to reconnect every 5 seconds

void mqtt_init() {
  if (!mqtt_is_enabled()) {
    return;
  }
  
  // Parse MQTT server address
  String mqttServer = String(cfg.mqtt_server);
  if (mqttServer.length() > 0) {
    IPAddress mqttIp;
    if (mqttIp.fromString(mqttServer)) {
      mqttClient.setServer(mqttIp, cfg.mqtt_port);
    } else {
      // Try to resolve hostname
      WiFi.hostByName(mqttServer.c_str(), mqttIp);
      mqttClient.setServer(mqttIp, cfg.mqtt_port);
    }
    Serial.println("MQTT: Initialized");
  }
}

void mqtt_connect() {
  if (!mqtt_is_enabled()) {
    return;
  }
  
  if (mqttClient.connected()) {
    return;
  }
  
  Serial.println("MQTT: Attempting to connect...");
  
  String clientId = "ESP32_" + String((uint32_t)ESP.getEfuseMac(), HEX);
  
  // Connect with username and password if provided
  bool connected = false;
  if (strlen(cfg.mqtt_username) > 0 && strlen(cfg.mqtt_password) > 0) {
    connected = mqttClient.connect(clientId.c_str(), cfg.mqtt_username, cfg.mqtt_password);
  } else {
    connected = mqttClient.connect(clientId.c_str());
  }
  
  if (connected) {
    Serial.println("MQTT: Connected successfully");
  } else {
    Serial.print("MQTT: Connection failed, state: ");
    Serial.println(mqttClient.state());
  }
}

void mqtt_publish_sensor_data(float temperature, float humidity) {
  if (!mqtt_is_enabled() || !mqttClient.connected()) {
    return;
  }
  
  unsigned long now = millis();
  if (now - lastMqttPublish < MQTT_PUBLISH_INTERVAL) {
    return;
  }
  
  lastMqttPublish = now;
  
  // Create topics using configured prefix
  String deviceId = String((uint32_t)ESP.getEfuseMac(), HEX);
  String prefix = String(cfg.mqtt_topic_prefix);
  String tempTopic = prefix + "/" + deviceId + "/temperature";
  String humidityTopic = prefix + "/" + deviceId + "/humidity";
  
  // Format and publish values
  char tempStr[16];
  char humidityStr[16];
  snprintf(tempStr, sizeof(tempStr), "%.2f", temperature);
  snprintf(humidityStr, sizeof(humidityStr), "%.2f", humidity);
  
  mqttClient.publish(tempTopic.c_str(), tempStr);
  mqttClient.publish(humidityTopic.c_str(), humidityStr);
  
  Serial.printf("MQTT: Published temp=%.2f, humidity=%.2f\n", temperature, humidity);
}

bool mqtt_is_enabled() {
  return cfg.mqtt_enabled && strlen(cfg.mqtt_server) > 0;
}

bool mqtt_is_connected() {
  return mqttClient.connected();
}

void mqtt_loop() {
  if (!mqtt_is_enabled()) {
    return;
  }
  
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    // Only attempt to connect if enough time has passed since last attempt
    if (now - lastMqttConnectionAttempt >= MQTT_RECONNECT_INTERVAL) {
      lastMqttConnectionAttempt = now;
      mqtt_connect();
    }
  } else {
    mqttClient.loop();
  }
}
