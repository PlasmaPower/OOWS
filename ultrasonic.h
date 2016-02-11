#pragma once

#ifdef ULTRASONIC

//#define USE_TIMER2

#ifdef USE_TIMER2
  #include <eRCaGuy_Timer2_Counter.h>
#endif

#include "sensor.h"

/* ==========PIN MAPPING==========
 *
 *        ANALOG
 *
 *  A0    ========== Thermistor 0 (Top)
 *  A1    ========== Thermistor 1 (Mid)
 *  A2    ========== Thermistor 2 (Bot)
 *
 *       DIGITAL
 *       
 *  2   ==========  Xbee
 *  3   ==========  Xbee
 *  4   ==========  Digital Temp Sensors
 *  5   ==========  Sonar Power       
 *  6   ==========  Sonar Echo
 *  7   ==========  Sonar Init
 *  8   ==========  Future use (Software Serial RX)
 *  9   ==========  Future use (Software Serial TX)
 *  10  ==========  SD Card 
 *  11  ==========  SD Card
 *  12  ==========  SD Card
 *  13  ==========  SD Card
 *
 *
 * ===============================
 */

/* ====Campbell Handshakes====
 *
 *    "Distance\n"      
 *  "TempTop\n"
 *  "TempMid\n"
 *  "TempBot\n"
 *
 * =====SensComp Connections=====
 * 
 * 
 *   Pin 1 - VDD
 *   Pin 2 - GND
 *   Pin 3 - Echo
 *   Pin 4 - not used
 *   Pin 5 - Init
 *   Pin 6 - not used
 *
 * ===========================
 */



// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 3
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor as measured by multimeter
#define SERIESRESISTOR 1586 

unsigned long getTime() {
  #ifdef USE_TIMER2
    return timer2.get_count();
  #else
    return millis();
  #endif
}

int tempSamples[NUMSAMPLES];

float thermistor_temp(int option){

  uint8_t i;
  float average;

  for (i=0; i< NUMSAMPLES; i++) {
    tempSamples[i] = analogRead(option);
    delay(10);
  }

  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
    average += tempSamples[i];
  }
  average /= NUMSAMPLES;

  // convert the value to resistance
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;

  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C

  return steinhart;

}

float microsecondsToCentimeters(long microseconds) {
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29.0 / 2.0;
}

float tempCorrection(int pin, float microseconds) {

  float temp = thermistor_temp(pin);
  float usPerMeter = (1.0/340.0)/100.0;
  float sound = 331.4 + 0.607 * temp;

  return (sound * microseconds)/1000000.0;
}

float sensor_distance(int thermPin, int SonarPower, int SonarEcho, int SonarInit){

  unsigned long duration0, duration1;
  float duration, cm;
  // The PING))) is triggered by a HIGH pulse of 2 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  digitalWrite(SonarInit, LOW);
  digitalWrite(SonarPower, LOW); // currently does nothing
  delay(5);
  digitalWrite(SonarPower, HIGH);
  delay(20);

  // int i;
  //  for (i=0; i< NUMSAMPLES; i++) {

  digitalWrite(SonarInit, HIGH);

  // The same pin is used to read the signal from the PING))): a HIGH
  // pulse whose duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.

  duration0 = getTime();

  while(digitalRead(SonarEcho) == 0 && (getTime() - duration0) < 1000000.0 && getTime() > duration0){
    ;
  }
  duration1 = getTime();
  digitalWrite(SonarInit, LOW);

  duration = (duration1 - duration0)/2.0;

  if(duration1 < duration0 || duration >= 1000000.0){
    digitalWrite(SonarPower, LOW);
    return 0;
  }else if(duration <= 0){
    digitalWrite(SonarPower, LOW);
    return 0;
  }


  //  }
  // convert the time into a distance
  //cm = microsecondsToCentimeters(duration);
  digitalWrite(SonarPower, LOW);
  cm = tempCorrection(thermPin, duration);

  //Serial.print(cm);
  //Serial.print(" cm");
  //Serial.println();
  return cm; 

}

class UltrasonicSensor : public Sensor {
  protected:
  int thermPin, sonarPower, sonarEcho, sonarInit;

  public:
  UltrasonicSensor(int thermPin, int sonarPower, int sonarEcho, int sonarInit) {
    this->thermPin = thermPin;
    this->sonarPower = sonarPower;
    this->sonarEcho = sonarEcho;
    this->sonarInit = sonarInit;
    #ifdef USE_TIMER2
      timer2.setup();
    #endif
  }

  int getNumberOfValues() {
    return 1;
  }

  float getValue(int number) {
    return sensor_distance(thermPin, sonarPower, sonarEcho, sonarInit);
  }

  String getValueName(int num) {
    return "Ultrasonic Distance";
  }
};

#endif

