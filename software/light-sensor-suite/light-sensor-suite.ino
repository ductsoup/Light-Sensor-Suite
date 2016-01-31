/***************************************************************************
  light-sensor-suite firmware

  This sketch makes I2C sensor data available to a ModbusTCP protocol master over WiFi.

  https://github.com/ductsoup/Light-Sensor-Suite

  MIT License
  Copyright (c) 2016 Ductsoup
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
   
  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE. 
  ***************************************************************************/

#define WLAN_SSID  "yourSSID"
#define WLAN_PASS  "yourPassword"

/***************************************************************************
  This is a library for the BME280 humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME280 Breakout
  ----> http://www.adafruit.com/products/2650

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit andopen-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

#include <Wire.h>
#include "Adafruit_TCS34725.h"
//
// An experimental wrapper class that implements the improved lux and color temperature from
// TAOS and a basic autorange mechanism.
//
// Written by ductsoup, public domain
//

// RGB Color Sensor with IR filter and White LED - TCS34725
// I2C 7-bit address 0x29, 8-bit address 0x52
//
// http://www.adafruit.com/product/1334
// http://learn.adafruit.com/adafruit-color-sensors/overview
// http://www.adafruit.com/datasheets/TCS34725.pdf
// http://www.ams.com/eng/Products/Light-Sensors/Color-Sensor/TCS34725
// http://www.ams.com/eng/content/view/download/265215 <- DN40, calculations
// http://www.ams.com/eng/content/view/download/181895 <- DN39, some thoughts on autogain
// http://www.ams.com/eng/content/view/download/145158 <- DN25 (original Adafruit calculations)
//
// connect LED to digital 4 or GROUND for ambient light sensing
// connect SCL to analog 5
// connect SDA to analog 4
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

// some magic numbers for this device from the DN40 application note
#define TCS34725_R_Coef 0.136
#define TCS34725_G_Coef 1.000
#define TCS34725_B_Coef -0.444
#define TCS34725_GA 1.0
#define TCS34725_DF 310.0
#define TCS34725_CT_Coef 3810.0
#define TCS34725_CT_Offset 1391.0

// Autorange class for TCS34725
class tcs34725 {
  public:
    tcs34725(void);

    boolean begin(void);
    void getData(void);

    boolean isAvailable, isSaturated;
    uint16_t againx, atime, atime_ms;
    uint16_t r, g, b, c;
    uint16_t ir;
    uint16_t r_comp, g_comp, b_comp, c_comp;
    uint16_t saturation, saturation75;
    float cratio, cpl, ct, lux, maxlux;

  private:
    struct tcs_agc {
      tcs34725Gain_t ag;
      tcs34725IntegrationTime_t at;
      uint16_t mincnt;
      uint16_t maxcnt;
    };
    static const tcs_agc agc_lst[];
    uint16_t agc_cur;

    void setGainTime(void);
    Adafruit_TCS34725 tcs;
};
//
// Gain/time combinations to use and the min/max limits for hysteresis
// that avoid saturation. They should be in order from dim to bright.
//
// Also set the first min count and the last max count to 0 to indicate
// the start and end of the list.
//
const tcs34725::tcs_agc tcs34725::agc_lst[] = {
  { TCS34725_GAIN_60X, TCS34725_INTEGRATIONTIME_700MS,     0, 47566 },
  { TCS34725_GAIN_16X, TCS34725_INTEGRATIONTIME_154MS,  3171, 63422 },
  { TCS34725_GAIN_4X,  TCS34725_INTEGRATIONTIME_154MS, 15855, 63422 },
  { TCS34725_GAIN_1X,  TCS34725_INTEGRATIONTIME_2_4MS,   248,     0 }
};
tcs34725::tcs34725() : agc_cur(0), isAvailable(0), isSaturated(0) {
}

// initialize the sensor
boolean tcs34725::begin(void) {
  tcs = Adafruit_TCS34725(agc_lst[agc_cur].at, agc_lst[agc_cur].ag);
  if ((isAvailable = tcs.begin()))
    setGainTime();
  return (isAvailable);
}

// Set the gain and integration time
void tcs34725::setGainTime(void) {
  tcs.setGain(agc_lst[agc_cur].ag);
  tcs.setIntegrationTime(agc_lst[agc_cur].at);
  atime = int(agc_lst[agc_cur].at);
  atime_ms = ((256 - atime) * 2.4);
  switch (agc_lst[agc_cur].ag) {
    case TCS34725_GAIN_1X:
      againx = 1;
      break;
    case TCS34725_GAIN_4X:
      againx = 4;
      break;
    case TCS34725_GAIN_16X:
      againx = 16;
      break;
    case TCS34725_GAIN_60X:
      againx = 60;
      break;
  }
}

// Retrieve data from the sensor and do the calculations
void tcs34725::getData(void) {
  // read the sensor and autorange if necessary
  tcs.getRawData(&r, &g, &b, &c);
  while (1) {
    if (agc_lst[agc_cur].maxcnt && c > agc_lst[agc_cur].maxcnt)
      agc_cur++;
    else if (agc_lst[agc_cur].mincnt && c < agc_lst[agc_cur].mincnt)
      agc_cur--;
    else break;

    setGainTime();
    delay((256 - atime) * 2.4 * 4); // shock absorber
    //delay(2000);
    tcs.getRawData(&r, &g, &b, &c);
    break;
  }

  // DN40 calculations
  ir = (r + g + b > c) ? (r + g + b - c) / 2 : 0;
  r_comp = r - ir;
  g_comp = g - ir;
  b_comp = b - ir;
  c_comp = c - ir;
  cratio = float(ir) / float(c);

  saturation = ((256 - atime) > 63) ? 65535 : 1024 * (256 - atime);
  saturation75 = (atime_ms < 150) ? saturation75 = saturation - saturation / 4 : saturation;
  isSaturated = (atime_ms < 150 && c > saturation75) ? 1 : 0;
  cpl = (atime_ms * againx) / (TCS34725_GA * TCS34725_DF);
  maxlux = 65535 / (cpl * 3);

  lux = (TCS34725_R_Coef * float(r_comp) + TCS34725_G_Coef * float(g_comp) + TCS34725_B_Coef * float(b_comp)) / cpl;
  ct = TCS34725_CT_Coef * float(b_comp) / float(r_comp) + TCS34725_CT_Offset;
}

tcs34725 tcs;

#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define SEALEVELPRESSURE_HPA (1015.03)

Adafruit_BME280 bme; // I2C

// Configure MODBUS over WiFi
#include <ESP8266WiFi.h>
#include "ModbusTCP.h"

// MODBUS holding register map (float)

#define S0_Available            40001
#define S0_ScanTime             40003
#define S0_CurrentMillis        40005
#define S0_RSSI                 40007

#define S1_Available            40009
#define S1_ScanTime             40011
#define S1_Lux                  40013
#define S1_CT                   40015
#define S1_IR                   40017
#define S1_AGAINX               40019
#define S1_ATIME_MS             40021
#define S1_ATIME                40023
#define S1_R_RAW                40025
#define S1_G_RAW                40027
#define S1_B_RAW                40029 
#define S1_C_RAW                40031
#define S1_R_COMP               40033
#define S1_G_COMP               40035
#define S1_B_COMP               40037
#define S1_C_COMP               40039

#define S2_Available            40041
#define S2_ScanTime             40043
#define S2_SeaLevelPressureHPA  40045  
#define S2_Temperature          40047
#define S2_Pressure             40049
#define S2_Altitude             40051
#define S2_Humidity             40053

bool s1, s2; // flags to indicate which sensors are available

ModbusTCP m;

void setup() {
  // Setup serial port access.
  Serial.begin(115200);
  delay(10);
  Serial.println(); Serial.println();
  Serial.println("2016-01-30 light-sensor-suite");

  // Connect to WiFi access point
  m.begin(WLAN_SSID, WLAN_PASS);
  m.setFloat(S0_Available, 1);

  // Initialize S1
  Serial.print("S1 - TCS34725 ");
  Serial.println((s1 = tcs.begin()) ? "initialized" : "error");
  m.setFloat(S1_Available, (float) s1);

  // Initialize S2
  Serial.print("S2 - BME280 ");
  Serial.println((s2 = bme.begin()) ? "initialized" : "error");
  m.setFloat(S2_Available, (float) s2);
  m.setFloat(S2_SeaLevelPressureHPA, SEALEVELPRESSURE_HPA);

  pinMode(2, OUTPUT); // If all went well turn the white LED on the TCS off
  if (s1 && s2)
    digitalWrite(2, LOW);
}

#define MAX_SCAN  1000
unsigned long S0StartTime = 0, S1StartTime = MAX_SCAN, S2StartTime = 2 * MAX_SCAN;

void loop() {

  unsigned long curMillis = millis();

  // update S0 registers
  if (curMillis - S0StartTime > 1000) {
    m.setFloat(S0_CurrentMillis, curMillis);
    m.setFloat(S0_RSSI, m.RSSI());
    m.setFloat(S0_ScanTime, millis() - curMillis);
    S0StartTime = curMillis;
  }

  // update S1 registers
  while (s1 && (curMillis - S1StartTime > 5000)) {
    tcs.getData();
    m.setFloat(S1_Lux, tcs.lux);
    m.setFloat(S1_CT, tcs.ct);
    m.setFloat(S1_IR, tcs.ir);
    m.setFloat(S1_AGAINX, tcs.againx);
    m.setFloat(S1_ATIME_MS, tcs.atime_ms);
    m.setFloat(S1_AGAINX, tcs.atime);
    m.setFloat(S1_R_RAW, tcs.r);
    m.setFloat(S1_G_RAW, tcs.g);
    m.setFloat(S1_B_RAW, tcs.b);
    m.setFloat(S1_C_RAW, tcs.c);
    m.setFloat(S1_R_COMP, tcs.r_comp);
    m.setFloat(S1_G_COMP, tcs.g_comp);
    m.setFloat(S1_B_COMP, tcs.b_comp);
    m.setFloat(S1_C_COMP, tcs.c_comp);
    m.setFloat(S1_ScanTime, millis() - curMillis);
    S1StartTime = curMillis;
  }

  // update S2 registers
  while (s2 && (curMillis - S2StartTime > 10000)) {
    m.setFloat(S2_Temperature, (9.0 / 5.0) * bme.readTemperature() + 32.0); // in F
    m.setFloat(S2_Pressure, 0.02953 * bme.readPressure() / 100.0F); // in InHG
    m.setFloat(S2_Altitude, 3.28084 * bme.readAltitude(m.getFloat(S2_SeaLevelPressureHPA))); // in feet
    m.setFloat(S2_Humidity, bme.readHumidity());
    m.setFloat(S2_ScanTime, millis() - curMillis);
    S2StartTime = curMillis;
  }

  // process MODBUS requests
  m.run();
  delay(10);
}
