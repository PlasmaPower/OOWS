#include <SPI.h>
#include <DHT.h>
#include <WiFi.h>

#define THERM_A_COEFFICIENT 0.003354015
#define THERM_B_COEFFICIENT 0.000256277
#define THERM_C_COEFFICIENT 0.000002082921
#define THERM_D_COEFFICIENT 0.000000073003206
#define THERM_PIN 2
#define AREF_VOLTAGE 5
#define THERM_RESISTOR 10000
#define network "Bronco-Guest"

WiFiClient client;

float readVoltage(int pin) {
  /*
  analogRead reads a value from the sensor, but scales it from 0-1023
  It instead should be from 0 volts to the constant AREF_VOLTAGE (maximum number of volts)
  To undo the scaling, we use a ratio
  */
  return analogRead(pin) * AREF_VOLTAGE / 1023.0;
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
public:
  DHT22Sensor(int pin) : dht(pin, DHT22) {
    dht.begin();
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
    switch(num) {
    case 0:
      return "Temperature";
    case 1:
      return "Humidity";
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
  
  float getValue(int num){
    float Vout = readVoltage(pin);
    float rt = (THERM_RESISTOR*(Vout/AREF_VOLTAGE))/(1-(Vout/AREF_VOLTAGE));
    float ln = log(rt/THERM_RESISTOR);
    float tempK = pow(THERM_A_COEFFICIENT+THERM_B_COEFFICIENT*ln+THERM_C_COEFFICIENT*(pow(ln, 2))+THERM_D_COEFFICIENT*(pow(ln,3)),(-1));
    float thermTemperature = tempK-273.5;
    return thermTemperature;
  }
  
  
  String getValueName(int num) {
    return "Thermistor Temperature";
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
    String dataStr = "";
    for (int i = 0; i < dataLength; i++) {
      dataStr += String("field") + String(i) + String("=") + String(data[i]);
      if (i < dataLength - 1) {
        dataStr += "&";
      }
    }
    if (WiFi.status() != WL_CONNECTED) {
      while (WiFi.begin(network) != WL_CONNECTED);
    }
    if (client.connect("api.thingspeak.com", 80)) {
      client.print("POST /update HTTP/1.1\n");
      client.print("Host: api.thingspeak.com\n");
      client.print("Connection: close\n");
      client.print("X-THINGSPEAKAPIKEY: USV4OP96Q9662WRD\n");
      client.print("Content-Type: application/x-www-form-urlencoded\n");
      client.print("Content-Length: ");
      client.print(dataStr.length());
      client.print("\n\n");
    }
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

void setup() {
  Serial.begin(9600);
  while (WiFi.begin(network) != WL_CONNECTED);
  Sensor* sensors[] = {new DHT22Sensor(2), new ThermistorSensor(1)};
  int sensorsLength = 2;
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

void loop() {}

