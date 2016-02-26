#pragma once

#include "output.h"

class CustomDataServerOutput : public Output {
  public:
    CustomDataServerOutput() {
      sendHTTP(CUSTOM_DATA_SERVER_IP, 80, "GET /arduinos/" + ARDUINO_NAME + "/init", "X-Password: " + CUSTOM_DATA_SERVER_PASSWORD + "\n", "");
    }

    void outputData(String headers[], float data[], int dataLength) {
      String strData = "";
      for (int i = 0; i < dataLength; i++) {
        strData += headers[i] + String("=") + String(data[i]);
        if (i < dataLength - 1) {
          strData += "&";
        }
      }
      sendHTTP(CUSTOM_DATA_SERVER_IP, 80, "POST /arduinos/" + ARDUINO_NAME + "/addData", "X-Password: " + CUSTOM_DATA_SERVER_PASSWORD + "\n", strData);
    }
};

