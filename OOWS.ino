#include <SPI.h>
#include <DHT.h>
#include <WiFi.h>

#include "config.h"

#define THERM_A_COEFFICIENT 0.003354015
#define THERM_B_COEFFICIENT 0.000256277
#define THERM_C_COEFFICIENT 0.000002082921
#define THERM_D_COEFFICIENT 0.000000073003206
#define THERM_PIN 2
#define AREF_VOLTAGE 5
#define THERM_RESISTOR 10000

//Thermocouple Constants http://www.mosaic-industries.com/embedded-systems/microcontroller-projects/temperature-measurement/thermocouple/type-t-calibration-table
#define T0 1.3500000*100
#define V0 5.9588600
#define P1 2.0325591*10
#define P2 3.3013079
#define P3 1.2638462*0.1
#define P4 -8.2883695*0.0001
#define Q1 1.7595577*0.1
#define Q2 7.9740521*0.001
#define Q3 0

WiFiClient client;
int failedCounter = 0;

void startWiFi() {
  client.stop();
  delay(1000);
  while (WiFi.begin(NETWORK) != WL_CONNECTED) {
    Serial.println("Failed to connect to network, trying again...");
  }
  Serial.println("Connected to network");
}

float readVoltage(int pin) {
  /*
  analogRead reads a value from the sensor, but scales it from 0-1023
  It instead should be from 0 volts to the constant AREF_VOLTAGE (maximum number of volts)
  To undo the scaling, we use a ratio
  */
  return analogRead(pin) * AREF_VOLTAGE / 1023.0;
}

boolean sendHTTP(String host, int port, String request, String string) {
  client.stop();
  if (client.connect(host.c_str(), port)) {
    client.print(request + " HTTP/1.1\n");
    client.print("Host: " + host + "\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    if (string.length() > 0) {
      client.print("Content-Length: ");
      client.print(string.length());
    }
    client.print("\n\n");
  
    client.print(string);
  
    if (client.connected()) {
      Serial.println("Connecting to host...");
      Serial.println();
  
      failedCounter = 0;
      return true;
    }
    else {
      failedCounter++;
  
      Serial.println("Connection to host failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  
  } else {
    failedCounter++;
  
    Serial.println("Connection to host Failed (" + String(failedCounter, DEC) + ")");
    Serial.println();
  }
  if (failedCounter > 3) {
    startWiFi();
    return false;
  }
}

class Sensor {
  public:
    int virtual getNumberOfValues() = 0;
    float virtual getValue(int num) = 0;
    String virtual getValueName(int num) = 0;
};

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

class ThermocoupleSensor : public Sensor {
  protected:
    int pin;

  public:
    ThermocoupleSensor(int pin) {
      this->pin = pin;
    }

    int getNumberOfValues() {
      return 1;
    }

    float getValue(int num) {
      float V = readVoltage(pin);
      float thermoTemperature = T0 + ((V - V0) * (P1 + (V - V0) * (P2 + (V - V0) * (P3 + P4 * (V - V0))))) / (1 + (V - V0) * (Q1 + (V - V0) * (Q2 + Q3 * (V - V0))));

      return thermoTemperature;
    }

    String getValueName(int num) {
      return "thermocouple_pin_" + String(pin) + "_temperature";
    }
};

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

class Output {
  public:
    void virtual outputData(float data[], int dataLength) {};
    void virtual outputData(String headers[], float data[], int dataLength) {
      this->outputData(data, dataLength);
    }
};

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
      sendHTTP("api.thingspeak.com", 80, "POST /update", tsData);
    }
};

class CustomDataServerOutput : public Output {
  public:
    CustomDataServerOutput() {
      sendHTTP(CUSTOM_DATA_SERVER_HOST, 3000, "GET /arduino/" + ARDUINO_NAME + "/init", ""); // TODO: Host on AWS and get actual IP
    }

    void outputData(String headers[], float data[], int dataLength) {
      String strData = "";
      for (int i = 0; i < dataLength; i++) {
        strData += headers[i] + String("=") + String(data[i]);
        if (i < dataLength - 1) {
          strData += "&";
        }
      }
      sendHTTP(CUSTOM_DATA_SERVER_HOST, 3000, "POST /arduino/" + ARDUINO_NAME + "/addData", strData);
    }
};

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

void setup() {}

void loop() {
  Serial.begin(9600);
  Sensor* sensors[] = {new DHT22Sensor(2), new ThermistorSensor(1), new ThermocoupleSensor(2), new TippingBucket(7)};
  int sensorsLength = 4;
  Output* outputs[] = {new SerialOutput(), new ThingspeakOutput()};
  int outputsLength = 2;
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
        headers[currHeader++] = sensors[i]->getValueName(n);
      }
    }
    for (int n = 0; n < outputsLength; n++) {
      outputs[n]->outputData(headers, vals, valsLength);
    }
    delay(5000);
  }
}
