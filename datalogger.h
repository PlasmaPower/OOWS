#pragma once

#include <SD.h>
#include <RTClib.h>

#include "output.h"

class SDCardOutput : public Output {
  protected:
    File file;
    RTC_DS1307 RTC;
    bool printedHeaders;

  public:
    SDCardOutput() {
      if (!SD.begin(10, 11, 12, 13)) {
       Serial.println("Failed to initialize SD card");
      }
      if (!RTC.begin()) {
        Serial.println("Failed to initialize Real Time Clock");
      }
      if (!RTC.isrunning()) {
        RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
      }
      int filenumber = 0;
      while (true) {
        String filenameStr = "DATA" + String(filenumber) + ".CSV";
        int len = filenameStr.length() + 1;
        char filename[len];
        filenameStr.toCharArray(filename, len);
        if (!SD.exists(filename)) {
          file = SD.open(filename, FILE_WRITE);
          break;
        }
        filenumber++;
      }
      printedHeaders = false;
    }

    void outputData(String headers[], float data[], int dataLength) {
      DateTime now = RTC.now();
      if (!printedHeaders) {
        file.print("unixTime,");
        for (int i = 0; i < dataLength; i++) {
          file.print(String(headers[i]));
          if (i < dataLength - 1) {
            file.print(",");
          }
        }
        file.println();
        printedHeaders = true;
      }
      file.print(now.unixtime());
      file.print(",");
      for (int i = 0; i < dataLength; i++) {
        file.print(String(data[i]));
        if (i < dataLength - 1) {
          file.print(",");
        }
      }
      file.println();
    }
};
