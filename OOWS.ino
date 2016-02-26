#include <SPI.h>
#include <Wire.h>

#include "config.h"
#include "sensor.h"
#include "output.h"
#include "wifi.h"

void setup() {}

void loop() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Starting up");
  initShields();
  startWiFi();
  Sensor* sensors[] = SENSORS;
  int sensorsLength = SENSORS_LENGTH;
  Output* outputs[] = OUTPUTS;
  int outputsLength = OUTPUTS_LENGTH;
  while (true) {
    int valsLength = 0;
    for (int i = 0; i < sensorsLength; i++) {
      valsLength += sensors[i]->getNumberOfValues();
    }
    float vals[valsLength];
    String headers[valsLength];
    int currVal = 0;
    int currHeader = 0;
    for (int i = 0; i < sensorsLength; i++) {
      int num = sensors[i]->getNumberOfValues();
      for (int n = 0; n < num; n++) {
        vals[currVal++] = sensors[i]->getValue(n);
      }
      for (int n = 0; n < num; n++) {
        String tmp = sensors[i]->getValueName(n);
        headers[currHeader++] = tmp;
      }
    }
    for (int n = 0; n < outputsLength; n++) {
      outputs[n]->outputData(headers, vals, valsLength);
    }
    delay(5000);
  }
}
