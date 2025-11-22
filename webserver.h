#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

struct SessionToken {
  String token;
  unsigned long expireTime;
};

extern SessionToken activeSession;
extern AsyncWebServer server;

void setupWebServer();

#endif