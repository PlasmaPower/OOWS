#include <SPI.h>
#include <DHT.h>

#include "config.h"

#define THERM_A_COEFFICIENT 0.003354015
#define THERM_B_COEFFICIENT 0.000256277
#define THERM_C_COEFFICIENT 0.000002082921
#define THERM_D_COEFFICIENT 0.000000073003206
#define THERM_PIN 2
#define AREF_VOLTAGE 5
#define THERM_RESISTOR 10000

#define WIFI_CONNECT_ATTEMPTS 4
#define CC3000_SHIELD

#ifdef WIFI_SHIELD
  #include <WiFi.h>
#endif

#ifdef CC3000_SHIELD
  #define CC3000_TINY_DRIVER
  #include <Adafruit_CC3000.h>
#endif

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

#ifdef WIFI_SHIELD
  WiFiClient client;
#endif

#ifdef CC3000_SHIELD
  // These are the interrupt and control pins
  #define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
  // These can be any two pins
  #define ADAFRUIT_CC3000_VBAT  5
  #define ADAFRUIT_CC3000_CS    10
  // Use hardware SPI for the remaining pins
  // On an UNO, SCK = 13, MISO = 12, and MOSI = 11
  Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);
#endif

int failedCounter = 0;

void initShields() {
  #ifdef CC3000_SHIELD
    if (!cc3000.begin()) {
      Serial.println("Could not connect to cc3000, please check your wiring");
    }
  #endif
}

void startWiFi() {
  #ifdef WIFI_SHIELD
    Serial.println("Connecting to " + String(NETWORK));
    if (client.connected()) {
      Serial.println("Currently connected, stopping");
      client.stop();
    }
    delay(1000);
    int error = WL_CONNECTED;
    for (int i = 0; i < WIFI_CONNECT_ATTEMPTS; i++) {
      error = WiFi.begin(NETWORK);
      if (error == WL_CONNECTED) {
        Serial.println("Connected to " + String(NETWORK));
        break;
      }
      if (i < WIFI_CONNECT_ATTEMPTS - 1) {
        Serial.println("Failed to connect to " + String(NETWORK) + " with error code " + String(error) + ", trying again...");
      } else {
        Serial.println("!!ERROR!! Failed to connect to " + String(NETWORK) + " " + String(WIFI_CONNECT_ATTEMPTS) + " times ending with error code " + String(error) +
          ", will NOT try again until wifi is used again");
      }
      if (error == WL_CONNECTED) {
        delay(1000);
      }
    }
  #endif

  #ifdef CC3000_SHIELD
    Serial.println("Connecting to " + String(NETWORK));
    if (cc3000.connectToAP(NETWORK, "", WLAN_SEC_UNSEC, WIFI_CONNECT_ATTEMPTS)) {
      Serial.println("Connected to " + String(NETWORK));
    } else {
      Serial.println("Failed to connect to " + String(NETWORK));
    }
    delay(5000);
  #endif
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
  #ifdef WIFI_SHIELD
    Serial.println("Sending HTTP message...");
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
        Serial.println("Successfully sent message!");

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
  #endif
  if (failedCounter >= 20) {
    startWiFi();
    failedCounter = 0;
    return false;
  }
}

boolean sendHTTP(uint32_t ip, int port, String request, String string) {
  #ifdef CC3000_SHIELD
    Serial.println("Sending HTTP message...");
    Adafruit_CC3000_Client socket = cc3000.connectTCP(ip, port);
    if (socket.connected()) {
      socket.print(request + " HTTP/1.0\n");
      socket.print("Connection: close\n");
      socket.print("Content-Type: application/x-www-form-urlencoded\n");
      if (string.length() > 0) {
        socket.print("Content-Length: ");
        socket.print(string.length());
      }
      socket.print("\n\n");
      socket.print(string);

      delay(1000);

      if (socket.connected()) {
        Serial.println("Successfully sent message!");

        failedCounter = 0;
        socket.close();
        return true;
      } else {
        failedCounter++;

        Serial.println("Connection to host failed (" + String(failedCounter, DEC) + ")");
        Serial.println();
      }
    } else {
      failedCounter++;

      Serial.println("Connection to host Failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  #endif
  if (failedCounter >= 20) {
    startWiFi();
    failedCounter = 0;
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
      sendHTTP(CUSTOM_DATA_SERVER_IP, 80, "GET /arduino/" + ARDUINO_NAME + "/init", "");
    }

    void outputData(String headers[], float data[], int dataLength) {
      String strData = "";
      for (int i = 0; i < dataLength; i++) {
        strData += headers[i] + String("=") + String(data[i]);
        if (i < dataLength - 1) {
          strData += "&";
        }
      }
      sendHTTP(CUSTOM_DATA_SERVER_IP, 80, "POST /arduino/" + ARDUINO_NAME + "/addData", strData);
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
  while(!Serial);
  Serial.println("Starting up");
  initShields();
  startWiFi();
  Sensor* sensors[] = SENSORS;
  int sensorsLength = 2;
  Output* outputs[] = OUTPUTS;
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
        String tmp = sensors[i]->getValueName(n);
        headers[currHeader++] = tmp;
      }
    }
    for (int n = 0; n < outputsLength; n++) {
      outputs[n]->outputData(headers, vals, valsLength);
    }
    delay(5000);
  }
}
