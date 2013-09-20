SI1143_Pulse-Prox_Sensors
=========================

Arduino Library for using the Modern Device Pulse Sensor and Proximity Sensors
http://moderndevice.com/product/pulse-heartbeat-sensor-pulse-sensor-1x/
http://moderndevice.com/product/si1143-proximity-sensors/

Installation instructions:
Download the library, unzip it and place it in the libraries folder 
inside wherever you are saving your Arduino Sketches. 
Restart Arduino if it's open.
Then look for the examples in File->Examples->SI1143_Pulse&Prox_Sensors->Examples
Load the example for your particular sensor.

Hardware setup:
Read the chart at the top of the sketch for the correct pins to use for I2C communication.
Then set the portForSI114 constant to the port number corresponding to the pins you
wired up.
For JeeNodes the sensor is plug and play.

Using the sketch:
See the #defines at the top of the sketch for various printing modes and give them a try.
Have fun!

