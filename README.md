# Light-Sensor-Suite
A low cost but very capable WiFi light sensor for daylight harvesting applications

##Overview
Effective daylight harvesting is difficult to implement in industrial environments using commercial lighting controllers because the natural sunlight contribution changes rapidly and varies widely over the course of the day. If the source of the daylight is side lighting, the challenge becomes nearly impossible.

A more practical approach is to use either fluorescent or LED dimmable fixtures over task areas then use a down looking light sensor as feedback to modulate the artificial light and consistently maintain any given light level at the work plane.

Commercial light sensors are expensive and limited. The goal here is to adapt widely available, inexpensive and very accurate smart phone sensor technology for use in an industrial environment.

![LSS Overview](/images/light-suite.png)

##Design Notes
This suite is intended to be mounted in the ceiling at the level of the light fixtures looking down at the task area. The enclosure is a cover that fits a standard 4x4 electrical junction box.

Rather than interface directly, this suite simply logs data to a SCADA system which makes it available to the lighting controllers. The lighting controller's PLC logic should be coded to use the information if available but not depend on it for normal operation. 

The lighting controller PLC supplies 24VDC. A TSR 1-2450 buck converter is used to provde 5VDC to the Feather HUZZAH ESP8266. The Feather's regulator supplies 3.3VDC to the sensor breakout boards.

Both sensor breakout boards communicate with the Feather over I2C. Other I2C sensors can easily be added as needed depending on the application.

A TCS34725 is used for light level and color temperature sensing. Color temperature sensing introduces the opportunity to discriminate between artificial and natural light. A BME280 is used for temperature, humidity and barometric pressure sening. The BME280 is not required for this application but provides useful data for managing destratification, heating and cooling systems.

##MODBUS Holding Register Map (float encoded)
All registers are read only unless otherwise indicated.

###S0 - Feather###
**S0_Available            40001**
Always 1 if the suite is connected and operating. 
**S0_ScanTime             40003**
The number of milliseconds it took to update the S0 data.
**S0_CurrentMillis        40005**
The number of milliseconds since the suite booted. This is natively an unsigned long value and will eventually wrap.
**S0_RSSI                 40007**

###S1 - TCS34725###
**S1_Available            40009**
A value of 1 indicates the sensor is connected and reporting, otherwise the value will be 0.
**S1_ScanTime             40011**
The number of milliseconds it took to update the S1 data.
**S1_Lux                  40013**
The most recent light level in lux.
**S1_CT                   40015**
The most recent color temperature in Kelvin.
**S1_IR                   40017**
The most recent IR correction value.
**S1_AGAINX               40019**
The current gain.
**S1_ATIME_MS             40021**
**S1_ATIME                40023**
The current integration time.
**S1_R_RAW                40025**
**S1_G_RAW                40027**
**S1_B_RAW                40029**
**S1_C_RAW                40031**
The most recent raw sensor data for the R, G, B and C channels.
**S1_R_COMP               40033**
**S1_G_COMP               40035**
**S1_B_COMP               40037**
**S1_C_COMP               40039**
The most recent IR compensated sensor data for the R, G, B and C channels.

###S2 - BME280###
**S2_Available            40041**
A value of 1 indicates the sensor is connected and reporting, otherwise the value will be 0.
**S2_ScanTime             40043**
The number of milliseconds it took to update the S2 data.
**S2_SeaLevelPressureHPA  40045**  
This register is read/write and contains the sea level pressure used to calculate altitude.
**S2_Temperature          40047**
The most recent temperature in Fahrenheit.
**S2_Pressure             40049**
The most recent atmospheric pressure in inches of mercury.
**S2_Altitude             40051**
The most recent calculated altitude in feet.
**s2_Humidity             40053**
The most recent relative humidity.
