#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>

// Initialize MQTT client
void mqtt_init();

// Connect to MQTT server (called from main loop)
void mqtt_connect();

// Publish sensor data to MQTT
void mqtt_publish_sensor_data(float temperature, float humidity);

// Check if MQTT is enabled
bool mqtt_is_enabled();

// Check if MQTT is connected
bool mqtt_is_connected();

// Handle MQTT callbacks and reconnection
void mqtt_loop();

#endif
