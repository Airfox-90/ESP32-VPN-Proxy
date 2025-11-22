#ifndef FORWARDER_H
#define FORWARDER_H

#include <Arduino.h>

extern uint64_t bytesSentSince;
extern uint64_t bytesRecvSince;
extern bool forwarder_connected;

void startForwarder();

#endif

