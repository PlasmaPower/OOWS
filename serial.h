#pragma once

#include "output.h"

class SerialOutput : public Output {
  public:
    SerialOutput() {}

    void outputData(String headers[], float data[], int dataLength) {
      for (int i = 0; i < dataLength; i++) {
        Serial.print(headers[i]);
        Serial.print(": ");
        Serial.print(String(data[i]));
        if (i < dataLength - 1) {
          Serial.print(", ");
        }
      }
      Serial.println();
    }
};
