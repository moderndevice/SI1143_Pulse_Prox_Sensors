/* SI1143_proximity_demo.ino
 * http://moderndevice.com/product/si1143-proximity-sensors/
 * Reads the Proximity Sensor and either prints raw LED data or angular data 
 * depending the options chosen in the #define's below
 * Paul Badger, Gavin Atkinson, Jean-Claude Wippler  2013
 */


/* Hardware setup - please consult the pin chart below for your choice of SDA/SCL pins to use
 for the sensor. Note that this I2C version is strictly bit-banged which makes it a 
 tad slower than the arduino version.
 However you can use it on any pins you choose, adding a lot of flexibility.
 The SI114 is not applicable to I2C daisy chaining on the same I2C line so use different ports
 for two sensors.
 
 Note that no series resistors are required using this library.
 If you choose to use series resistors on the data lines, use 4.7 or 5k.
 Connect Ground.
 Connect 3.3V line to 3.3 volts (PWR line on sensors are not connected).
 
 
 For JeeNode / Arduino / ATmega328 users, just set the port used
 
 JeeNode Port  SDA ('duino pin)  SCL ('duino pin)
 0             A4            A5    // Leonardo: SDA is pin 2, SCL is pin 3.
 1             4             14 (A0)
 2             5             15 (A1)
 3             6             16 (A2)
 4             7             17 (A3)
 
 On the ATMega2560:
 JeeNode Port  SDA ('duino pin)  SCL ('duino pin)
 0             18            19
 1             30            31
 2             32            33
 3             14            15
 4             28            29
 
 
 On the MD BBLeo / Leonardo / ATMega32u4:
 JeeNode Port  SDA ('duino pin)  SCL ('duino pin)
 0             2            3
 1             4            18 (A0)
 2             5            19 (A1)
 3             6            20 (A2)
 4             7            21 (A3)
 
 */

const int PORT_SI114_LEFT = 1;       // change to the JeeNode port number used, see the pin chart above
const int PORT_SI114_RIGHT = 2;       // change to the JeeNode port number used, see the pin chart above

#include "SI114.h"

const int samples = 4;            // samples for smoothing 1 to 10 seem useful

#define PRINT_RAW_LED_VALUES      // prints Raw LED values 

unsigned long lastMillis, IR1, IR2, IR3;

PortI2C rightBus (PORT_SI114_RIGHT);
PulsePlug SI114Right (rightBus); 

PortI2C LeftBus (PORT_SI114_LEFT);
PulsePlug SI114Left (LeftBus); 

void setup () {
    Serial.begin(9600);
    Serial.println("\n[pulse_demo]");

    if (!SI114Right.isPresent()) {
        Serial.print("No SI114x found on Port ");
        Serial.println(PORT_SI114_RIGHT);
    }
    Serial.begin(9600);

    SI114Right.initPulsePlug(SI114Left);
    SI114Right.initPulsePlug(SI114Right);

}


void loop(){

    delay(100);
    unsigned long leftSI114Total, rightSI114Total;

    SI114Left.fetchLedData();
    SI114Right.fetchLedData();

    SI114Left.ps1;
    SI114Left.ps2;
    SI114Left.ps3;
    leftSI114Total = SI114Left.ps1 + SI114Left.ps2 + SI114Left.ps3;


    SI114Right.ps1;
    SI114Right.ps2;
    SI114Right.ps3;
    rightSI114Total = SI114Right.ps1 + SI114Right.ps2 + SI114Right.ps3;


#ifdef PRINT_AMBIENT_LIGHT_SAMPLING 


    SI114Left.fetchData();
    SI114Right.fetchData();

    Serial.print(" left   ");
    Serial.print(SI114Left.als_vis);        //  left ambient visible
    Serial.print("\t"); 
    Serial.print(SI114Left.als_ir);         //  ambient IR
    Serial.print("    right ");
    Serial.print(SI114Right.als_vis);       //  right ambient visible
    Serial.print("\t");
    Serial.print(SI114Right.als_ir);        //  ambient IR
    Serial.print("\t");

#endif


    Serial.print(" left   ");
    Serial.print(SI114Left.ps1);
    Serial.print("\t");
    Serial.print(SI114Left.ps2);
    Serial.print("\t");
    Serial.print(SI114Left.ps3);
    Serial.print("\t");
    Serial.print(leftSI114Total);
    Serial.print("    right ");
    Serial.print(SI114Right.ps1);
    Serial.print("\t");
    Serial.print(SI114Right.ps2);
    Serial.print("\t");
    Serial.print(SI114Right.ps3);
    Serial.print("\t"); 
    Serial.println(rightSI114Total);   
}












