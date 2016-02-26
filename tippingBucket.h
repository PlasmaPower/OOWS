#pragma once

#include "sensor.h"

volatile int tippingBucketCount;

void incrementTippingBucketCount() {
  tippingBucketCount++;
}

class TippingBucket : public Sensor {
  protected:
    bool interruptAttached = false;
    int pin;
  public:
    TippingBucket(int pin) {
      if (!interruptAttached) {
        attachInterrupt(digitalPinToInterrupt(pin), incrementTippingBucketCount, RISING);
        interruptAttached = true;
      }
      this->pin = pin;
    }

    int getNumberOfValues() {
      return 1;
    }

    float getValue(int num) {
      int tmp = tippingBucketCount;
      tippingBucketCount = 0;
      return tmp;
    }

    String getValueName(int num) {
      return "tipping_bucket_pin_" + String(pin) + "_Pulses";
    }
};
