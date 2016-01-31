#include <ESP8266WiFi.h>

#include <SPI.h>
#include "ModbusTCP.h"

// Register map for this example
#define S1_FloatConstant 40001 // modpoll -m tcp -t 4:float -r 40001 [ip_addr] 
#define S1_CurrentMillis 40003 // modpoll -m tcp -t 4:int -r 40003 [ip_addr]
#define S1_FeetPerMile   40005 // modpoll -m tcp -t 4 -r 40005 [ip_addr]

ModbusTCP m;

void setup(void)
{
  Serial.begin(115200);
  Serial.println(); Serial.println("Hello");
  // set some initial values
  m.setFloat(S1_FloatConstant, PI);
  m.setFloat(S1_CurrentMillis, millis());
  m.setFloat(S1_FeetPerMile, 5280);

  m.begin("yourSSID", "yourPassword");
}

void loop(void)
{
  // Process MODBUS requests on every scan
  m.run();
  delay(10);
  // Update the MODBUS registers
  m.setFloat(S1_CurrentMillis, millis());  
}




