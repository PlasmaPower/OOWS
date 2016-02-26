#pragma once

#include "sensor.h"
#include "wifi.h"

class ThingspeakOutput : public Output {
  public:
    ThingspeakOutput() {}

    void outputData(float data[], int dataLength) {
      String tsData = "";
      for (int i = 0; i < dataLength; i++) {
        tsData += String("field") + String(i + 1) + String("=") + String(data[i]);
        if (i < dataLength - 1) {
          tsData += "&";
        }
      }
      sendHTTP("api.thingspeak.com", 80, "POST /update", "X-THINGSPEAKAPIKEY: " + APIKEY + "\n", tsData);
    }
};
