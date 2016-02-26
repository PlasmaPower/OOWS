#pragma once

class Output {
  public:
    void virtual outputData(float data[], int dataLength) {};
    void virtual outputData(String headers[], float data[], int dataLength) {
      this->outputData(data, dataLength);
    }
};
