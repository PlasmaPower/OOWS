#pragma once

class Sensor {
  public:
    int virtual getNumberOfValues() = 0;
    float virtual getValue(int num) = 0;
    String virtual getValueName(int num) = 0;
};
