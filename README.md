# Light-Sensor-Suite
A low cost but very capable WiFi light sensor for daylight harvesting applications

##Overview
Effective daylight harvesting is difficult to implement in industrial environments using commercial lighting controllers because the natural sunlight contribution changes rapidly and varies widely over the course of the day. If the source of the daylight is side lighting, the challenge becomes almost impossible.

A more practical approach is to use either fluorescent or LED dimmable fixtures over task areas then use a down looking light sensor as feedback to modulate the artificial light and consistently maintain any given light level at the work plane.

Commercial light sensors are expensive and limited. The goal here is to adapt widely available, inexpensive and very accurate smart phone sensor technology for use in an industrial environment.

![LSS Overview](/images/light-suite.png)

##Design Notes
This suite is intended to be mounted in the ceiling at the level of the light fixtures looking down at the task area. The enclosure is a simple cover for a standard 4x4 electrical box.

Rather than interface directly, this suite simply logs data to a SCADA system which makes it available to the lighting controllers. The lighting controller's PLC logic should be coded to use the information if available but not depend on it for normal operation. 

The lighting controller PLC supplies 24VDC. A TSR 1-2450 buck converter is used to provde 5VDC to the Feather HUZZAH ESP8266. The Feather's regulator supplies 3.3VDC to the sensor breakout boards.

Both sensor breakout boards communicate with the Feather over I2C. Other I2C sensors can easily be added as needed depending on the application.

A TCS34725 is used for light level and color temperature sensing. Color temperature sensing introduces the opportunity to discriminate between artificial and natural light. A BME280 is used for temperature, humidity and barometric pressure sening. The BME280 is not required for this application but provides useful data for managing destratification, heating and cooling systems.
