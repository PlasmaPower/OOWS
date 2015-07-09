#include <SPI.h>
#include <DHT.h>
#include <WiFi.h>

WiFiClient client;

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
  DHT22Sensor(int pin) : dht(DHT22, pin) {
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

/*
WIP
class ThermistorSensor : public Sensor {
public:
  TermistorSensor(int pin) {}

  int getNumberOfValues() {
    return 1;
  }
};
*/

class Output {
public:
  void virtual outputData(int data[], int dataLength) = 0;
};

class ThingspeakOutput : public Output {
public:
  ThingspeakOutput() {}

  void outputData(int data[], int dataLength) {
    String dataStr = "";
    for (int i = 0; i < dataLength; i++) {
      dataStr += String("field") + String(i) + String("=") + String(data[i]);
      if (i < dataLength - 1) {
        dataStr += "&";
      }
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

void setup() {
  while (WiFi.begin("Bronco-Guest") != WL_CONNECTED);
  Sensor* sensors[] = {new DHT22Sensor(2)};
  int sensorsLength = 1;
  Output* outputs[] = {new ThingspeakOutput()};
  int outputsLength = 1;
  for (int i = 0; i < sensorsLength; i++) {
    int num = sensors[i]->getNumberOfValues();
    int vals[num];
    for (int n = 0; n < num; n++) {
      vals[n] = sensors[i]->getValue(n);
    }
    for (int n = 0; n < outputsLength; n++) {
      outputs[n]->outputData(vals, num);
    }
  }
}

void loop() {
  
}

