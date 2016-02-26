#pragma once

#include "sensor.h"

#define THERM_A_COEFFICIENT 0.003354015
#define THERM_B_COEFFICIENT 0.000256277
#define THERM_C_COEFFICIENT 0.000002082921
#define THERM_D_COEFFICIENT 0.000000073003206
#define THERM_PIN 2
#define AREF_VOLTAGE 5
#define THERM_RESISTOR 10000

class ThermistorSensor : public Sensor {
  protected:
    int pin;

  public:
    ThermistorSensor(int pin) {
      this->pin = pin;
    }

    int getNumberOfValues() {
      return 1;
    }

    float getValue(int num) {
      float Vout = readVoltage(pin);
      float rt = (THERM_RESISTOR * (Vout / AREF_VOLTAGE)) / (1 - (Vout / AREF_VOLTAGE));
      float ln = log(rt / THERM_RESISTOR);
      float tempK = pow(THERM_A_COEFFICIENT + THERM_B_COEFFICIENT * ln + THERM_C_COEFFICIENT * (pow(ln, 2)) + THERM_D_COEFFICIENT * (pow(ln, 3)), (-1));
      float thermTemperature = tempK - 273.5;
      return thermTemperature;
    }


    String getValueName(int num) {
      return "thermistor_pin_" + String(pin) + "_temperature";
    }

};

