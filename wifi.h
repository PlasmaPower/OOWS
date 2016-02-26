#pragma once

#include "output.h"

#define WIFI_CONNECT_ATTEMPTS 4

#ifdef WIFI_SHIELD
  #include <WiFi.h>
#endif

#ifdef CC3000_SHIELD
  #define CC3000_TINY_DRIVER
  #include <Adafruit_CC3000.h>
#endif

#ifdef WIFI_SHIELD
  WiFiClient client;
#endif

#ifdef CC3000_SHIELD
  // These are the interrupt and control pins
  #define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
  // These can be any two pins
  #define ADAFRUIT_CC3000_VBAT  5
  #define ADAFRUIT_CC3000_CS    10
  // Use hardware SPI for the remaining pins
  // On an UNO, SCK = 13, MISO = 12, and MOSI = 11
  Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);
#endif

int failedCounter = 0;

void initShields() {
  #ifdef CC3000_SHIELD
    if (!cc3000.begin()) {
      Serial.println("Could not connect to cc3000, please check your wiring");
    }
  #endif
}

void startWiFi() {
  #ifdef WIFI_SHIELD
    Serial.println("Connecting to " + String(NETWORK));
    if (client.connected()) {
      Serial.println("Currently connected, stopping");
      client.stop();
    }
    delay(1000);
    int error = WL_CONNECTED;
    for (int i = 0; i < WIFI_CONNECT_ATTEMPTS; i++) {
      error = WiFi.begin(NETWORK);
      if (error == WL_CONNECTED) {
        Serial.println("Connected to " + String(NETWORK));
        break;
      }
      if (i < WIFI_CONNECT_ATTEMPTS - 1) {
        Serial.println("Failed to connect to " + String(NETWORK) + " with error code " + String(error) + ", trying again...");
      } else {
        Serial.println("!!ERROR!! Failed to connect to " + String(NETWORK) + " " + String(WIFI_CONNECT_ATTEMPTS) + " times ending with error code " + String(error) +
          ", will NOT try again until wifi is used again");
      }
      if (error == WL_CONNECTED) {
        delay(1000);
      }
    }
  #endif

  #ifdef CC3000_SHIELD
    Serial.println("Connecting to " + String(NETWORK));
    if (cc3000.connectToAP(NETWORK, "", WLAN_SEC_UNSEC, WIFI_CONNECT_ATTEMPTS)) {
      Serial.println("Connected to " + String(NETWORK));
    } else {
      Serial.println("Failed to connect to " + String(NETWORK));
    }
    delay(5000);
  #endif
}

boolean sendHTTP(String host, int port, String request, String headers, String string) {
  #ifdef WIFI_SHIELD
    Serial.println("Sending HTTP message...");
    client.stop();
    if (client.connect(host.c_str(), port)) {
      client.print(request + " HTTP/1.1\n");
      client.print("Host: " + host + "\n");
      client.print("Connection: close\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      if (string.length() > 0) {
        client.print("Content-Length: ");
        client.print(string.length());
      }
      client.print(headers);
      client.print("\n\n");

      client.print(string);

      if (client.connected()) {
        Serial.println("Successfully sent message!");

        failedCounter = 0;
        return true;
      }
      else {
        failedCounter++;

        Serial.println("Connection to host failed (" + String(failedCounter, DEC) + ")");
        Serial.println();
      }

    } else {
      failedCounter++;

      Serial.println("Connection to host Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  #endif
  if (failedCounter >= 20) {
    startWiFi();
    failedCounter = 0;
    return false;
  }
}

boolean sendHTTP(uint32_t ip, int port, String request, String headers, String string) {
  #ifdef CC3000_SHIELD
    Serial.println("Sending HTTP message...");
    Adafruit_CC3000_Client socket = cc3000.connectTCP(ip, port);
    if (socket.connected()) {
      socket.print(request + " HTTP/1.0\n");
      socket.print("Connection: close\n");
      socket.print("Content-Type: application/x-www-form-urlencoded\n");
      if (string.length() > 0) {
        socket.print("Content-Length: ");
        socket.print(string.length());
      }
      socket.print(headers);
      socket.print("\n\n");
      socket.print(string);

      delay(1000);

      if (socket.connected()) {
        Serial.println("Successfully sent message!");

        failedCounter = 0;
        socket.close();
        return true;
      } else {
        failedCounter++;

        Serial.println("Connection to host failed (" + String(failedCounter, DEC) + ")");
        Serial.println();
      }
    } else {
      failedCounter++;

      Serial.println("Connection to host Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  #endif
  if (failedCounter >= 20) {
    startWiFi();
    failedCounter = 0;
    return false;
  }
}
