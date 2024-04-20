// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>

constexpr char* ssid = "YourWifiSSID";
constexpr char* password = "YourWifiPassword";
constexpr char* httpEndpoint = "http://YOURIP:YOURPORT/image?displayId=1";
constexpr uint64_t deepSleepTime = 2 * 60 * 1000 * 1000;//min * sec * millisec * microsec
constexpr int imageBytes = 48000;//don't change

// Pin definitions for connecting the screen adapter board (DESPI-C02) to the ESP32
constexpr int cs_pin = 7;
constexpr int dc_pin = 5;
constexpr int rst_pin = 4;
constexpr int busy_pin = 8;

#endif