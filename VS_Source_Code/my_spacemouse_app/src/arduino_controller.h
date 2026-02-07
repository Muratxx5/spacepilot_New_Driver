#ifndef ARDUINO_CONTROLLER_H
#define ARDUINO_CONTROLLER_H

#include <windows.h>

typedef enum {
    ARDUINO_DISCONNECTED,
    ARDUINO_CONNECTED,
    ARDUINO_ERROR
} ArduinoStatus;

ArduinoStatus init_arduino();
void send_to_arduino(const char* message);
void close_arduino();
const char* get_arduino_port();
ArduinoStatus get_arduino_status();

#endif