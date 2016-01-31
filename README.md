# Light-Sensor-Suite
A low cost but capable WiFi ambient light sensor for advanced daylight harvesting applications.

The hardware was prototyped and refined using  [Adafruit IO](https://io.adafruit.com), the target SCADA environment is [Inductive Automation's Ignition](https://inductiveautomation.com/scada-software/) product.

##Overview
Effective daylight harvesting is difficult to implement in industrial environments using commercial lighting controllers because the natural sunlight contribution changes rapidly and varies widely over the course of the day. If the source of the daylight is side lighting, the challenge becomes nearly impossible.

A more practical approach is to use either fluorescent or LED dimmable fixtures over task areas then use a down looking light sensor as feedback to modulate the artificial light and consistently maintain any given light level at the work plane.

Commercial light sensors are expensive and limited. The goal here is to adapt widely available, inexpensive and very accurate smart phone sensor technology for use in an industrial environment.

![LSS Overview](/images/light-suite.png)

##Design Notes
This suite is intended to be mounted in the ceiling at the level of the light fixtures looking down at the task area. The enclosure is a cover that fits a standard 4x4 electrical junction box.

Rather than interface directly, this suite simply logs data to a SCADA system which makes it available to the lighting controllers. The lighting controller's PLC logic should be coded to use the information if available but not depend on it for normal operation. 

The lighting controller PLC supplies 24VDC. A [TSR 1-2450] (https://www.adafruit.com/products/1065) buck converter (supply 6.5 to 36VDC) is used to provde 5VDC to the [Adafruit Feather HUZZAH ESP8266] (https://www.adafruit.com/products/2821). The ESP8266's regulator supplies 3.3VDC to the sensor breakout boards.

Both sensor breakout boards communicate with the ESP8266 over I2C. Other I2C sensors can easily be added as needed depending on the application.

A [TCS34725] (https://www.adafruit.com/products/1334) is used for light level and color temperature sensing. Color temperature sensing introduces the opportunity to discriminate between artificial and natural light. A [BME280] (https://www.adafruit.com/products/2652) is used for temperature, humidity and barometric pressure sening. The BME280 is not required for this application but provides useful data for managing destratification, heating and cooling systems.

##MODBUS Holding Register Map (Float encoding, reversed word order)
All registers are read only unless otherwise indicated.

###S0 - ESP8266###
**40001 S0_Available**

Always 1 if the suite is connected and operating. 

**40003 S0_ScanTime**

The number of milliseconds it took to update the S0 data.

**40005 S0_CurrentMillis**

The number of milliseconds since the suite booted. This is natively an unsigned long value and will eventually wrap.

**40007 S0_RSSI**

The WiFi signal strength.

###S1 - TCS34725###

**40009 S1_Available**

A value of 1 indicates the sensor is connected and reporting, otherwise the value will be 0.

**40011 S1_ScanTime**

The number of milliseconds it took to update the S1 data.

**40013 S1_Lux**

The most recent light level in lux.

**40015 S1_CT**

The most recent color temperature in Kelvin.

**40017 S1_IR**

The most recent IR correction value.

**40019 S1_AGAINX**

The current gain.

**40021 S1_ATIME_MS, 40023 S1_ATIME**

The current integration time.

**40025 S1_R_RAW, 40027 S1_G_RAW, 40029 S1_B_RAW, 40031 S1_C_RAW**

The most recent raw sensor data for the R, G, B and C channels.

**40033 S1_R_COMP, 40035 S1_G_COMP, 40037 S1_B_COMP, 40039 S1_C_COMP**

The most recent IR compensated sensor data for the R, G, B and C channels.

###S2 - BME280###

**40041 S2_Available**

A value of 1 indicates the sensor is connected and reporting, otherwise the value will be 0.

**40043 S2_ScanTime**

The number of milliseconds it took to update the S2 data.

**40045 S2_SeaLevelPressureHPA**  

This register is read/write and contains the sea level pressure used to calculate altitude.

**40047 S2_Temperature**

The most recent temperature in Fahrenheit.

**40049 S2_Pressure**

The most recent atmospheric pressure in inches of mercury.

**40051 S2_Altitude**

The most recent calculated altitude in feet.

**40053 S2_Humidity**

The most recent relative humidity.
