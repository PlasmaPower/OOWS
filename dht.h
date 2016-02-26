#pragma once

#include <DHT.h>

#include "sensor.h"

class DHT22Sensor : public Sensor {
  protected:
    DHT dht;
    int pin;
  public:
    DHT22Sensor(int pin) : dht(pin, DHT22) {
      dht.begin();
      this->pin = pin;
    }

    int getNumberOfValues() {
      return 2;
    }

    float getValue(int num) {
      switch (num) {
        case 0:
          return dht.readTemperature();
        case 1:
          return dht.readHumidity();
        default:
          return 0;
      }
    }

    String getValueName(int num) {
      switch (num) {
        case 0:
          return "DHT_pin_" + String(pin) + "_temperature";
        case 1:
          return "DHT_pin_" + String(pin) + "_humidity";
        default:
          return "";
      }
    }
};
