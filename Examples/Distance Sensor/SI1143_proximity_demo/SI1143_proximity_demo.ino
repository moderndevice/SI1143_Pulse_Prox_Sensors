/* SI1143_proximity_demo.ino
 * http://moderndevice.com/product/si1143-proximity-sensors/
 * Reads the Proximity Sensor and either prints raw LED data or angular data 
 * depending the options chosen in the #define's below
 * Paul Badger, Gavin Atkinson, Jean-Claude Wippler  2013
 */


/* Hardware setup - please consult the pin chart below for your choice of SDA/SCL pins to use
 for the sensor. The "Port" settings set both the SDA and SCL pins at one time
 Note that this I2C version is strictly bit-banged which makes it a 
 tad slower than the arduino version.
 However you can use it on any pins you choose, adding a lot of flexibility.
 The SI114 is not applicable to I2C daisy chaining on the same I2C line so use different ports
 for two sensors.
 
 Note that no series resistors are required using this library.
 If you choose to use series resistors on the data lines, use 4.7 or 5k.
 Connect Ground.
 Connect 3.3V line to 3.3 volts (PWR line on sensors are not connected).
  
  
  For JeeNode / Arduino / ATmega328 users, just set the port used
  
 Port  SDA ('duino pin)  SCL ('duino pin)
       0             A4            A5    // Leonardo: SDA is pin 2, SCL is pin 3.
       1             4             14 (A0)
       2             5             15 (A1)
       3             6             16 (A2)
       4             7             17 (A3)

  On the ATMega2560 / Arduino Mega etc:
  Port  SDA ('duino pin)  SCL ('duino pin)
  0             18            19
  1             30            31
  2             32            33
  3             14            15
  4             28            29
  
  On the MD BBLeo / Leonardo / ATMega32u4:
  Port  SDA ('duino pin)  SCL ('duino pin)
  0             2            3
  1             4            18 (A0)
  2             5            19 (A1)
  3             6            20 (A2)
  4             7            21 (A3)

*/

const int PORT_FOR_SI114 = 2;       // change to the JeeNode port number used, see the pin chart above

#include <SI114.h>

const int samples = 4;            // samples for smoothing 1 to 10 seem useful
                                  // increase for smoother waveform (with less resolution) 
float Avect = 0.0; 
float Bvect = 0.0;
float Cvect = 0.0;
float Tvect, x, y, angle = 0;


// some printing options for experimentation (sketch is about the same)
//#define SEND_TO_PROCESSING_SKETCH
#define PRINT_RAW_LED_VALUES   // prints Raw LED values for debug or experimenting
// #define PRINT_AMBIENT_LIGHT_SAMPLING   // also samples ambient slight (slightly slower)
                                          // good for ambient light experiments, comparing output with ambient
 
unsigned long lastMillis, red, IR1, IR2;

PortI2C myBus (PORT_FOR_SI114);
PulsePlug pulse (myBus); 

void setup () {
    Serial.begin(57600);
    Serial.println("\n[pulse_demo]");

    if (!pulse.isPresent()) {
        Serial.print("No SI114x found on Port ");
        Serial.println(PORT_FOR_SI114);
    }
    Serial.begin(57600);
    
    pulse.initPulsePlug();
    

    pulse.setLEDcurrents(6, 6, 6);
    // void PulsePlug.setLEDCurrent( byte LED1, byte LED2, byte LED3)
    // 0 to 15 are valid
    // 1 = 5.6mA, 2 = 11.2mA, 3 = 22.4mA, 4 = 45mA; 5 = 67mA, 6 =90mA, 7 = 112mA, 8 = 135mA, 9 = 157mA,
    // 10 = 180mA, 11 = 202mA, 12 = 224mA, 13 = 269mA, 14 = 314mA, 15 = 359mA 
    // It may be possible to damage the chip and or LEDs with some current / time settings - check that datasheet!
    
    pulse.setLEDdrive(1, 2, 4);
    //void PulsePlug::setLEDdrive(byte LED1pulse, byte LED2pulse, byte LED3pulse){
     // this sets which LEDs are active on which pulses 
     // any or none of the LEDs may be active on each PulsePlug
     //000: NO LED DRIVE
     //xx1: LED1 Drive Enabled
     //x1x: LED2 Drive Enabled (Si1142 and Si1143 only. Clear for Si1141)
     //1xx: LED3 Drive Enabled (Si1143 only. Clear for Si1141 and Si1142)
     // example setLEDdrive(1, 2, 5); sets LED1 on pulse 1, LED2 on pulse 2, LED3, LED1 on pulse 3
     
    
}


void loop(){
  delay(100);

    unsigned long total=0, start;
    int i=0;
    red = 0;
    IR1 = 0;
    IR2 = 0;
    total = 0;
    start = millis();



 while (i < samples){ 


 #ifdef PRINT_AMBIENT_LIGHT_SAMPLING   

        pulse.fetchData();    // gets ambient readings and LED (pulsed) readings

 #else
 
        pulse.fetchLedData();    // gets just LED (pulsed) readings - bit faster

 #endif
        red += pulse.ps1;
        IR1 += pulse.ps2;
        IR2 += pulse.ps3;
        i++;

    }

    red = red / i;  // get averages
    IR1 = IR1 / i;
    IR2 = IR2 / i;
    total = red + IR1 + IR2;

 #ifdef PRINT_AMBIENT_LIGHT_SAMPLING

    Serial.print(pulse.resp, HEX);     // resp
    Serial.print("\t");
    Serial.print(pulse.als_vis);       //  ambient visible
    Serial.print("\t");
    Serial.print(pulse.als_ir);        //  ambient IR
    Serial.print("\t");
   
 #endif
 
#ifdef PRINT_RAW_LED_VALUES

    Serial.print(red);
    Serial.print("\t");
    Serial.print(IR1);
    Serial.print("\t");
    Serial.print(IR2);
    Serial.print("\t");
    Serial.println((long)total);    
    Serial.print("\t");      

#endif                                
                                    

#ifdef SEND_TO_PROCESSING_SKETCH

	/* Add LED values as vectors - treat each vector as a force vector at
	 * 0 deg, 120 deg, 240 deg respectively
	 * parse out x and y components of each vector
	 * y = sin(angle) * Vect , x = cos(angle) * Vect
	 * add vectors then use atan2() to get angle
	 * vector quantity from pythagorean theorem
	 */

Avect = IR1;
Bvect = red;
Cvect = IR2;

// cut off reporting if total reported from LED pulses is less than the ambient measurement
// eliminates noise when no signal is present
if (total > 900){    //determined empirically, you may need to adjust for your sensor or lighting conditions

	x = (float)Avect -  (.5 * (float)Bvect ) - ( .5 * (float)Cvect); // integer math
	y = (.866 * (float)Bvect) - (.866 * (float)Cvect);

	angle = atan2((float)y, (float)x) * 57.296 + 180 ; // convert to degrees and lose the neg values
	Tvect = (long)sqrt( x * x + y * y); 
}
else{  // report 0's if no signal (object) present above sensor
 angle = 0;
Tvect = 0; 
total =  900;
  
}

// angle is the resolved angle from vector addition of the three LED values
// Tvect is the vector amount from vector addition of the three LED values
// Basically a ratio of differences of LED values to each other
// total is just the total of raw LED amounts returned, proportional to the distance of objects from the sensor.



Serial.print(angle);
Serial.print("\t");
Serial.print(Tvect);
Serial.print("\t");
Serial.println(total);

#endif

delay(10);                               
                                                                  
}


// simple smoothing function for future heartbeat detection and processing
float smooth(float data, float filterVal, float smoothedVal){

    if (filterVal > 1){      // check to make sure param's are within range
        filterVal = .999;
    }
    else if (filterVal <= 0.0){
        filterVal = 0.001;
    }

    smoothedVal = (data * (1.0 - filterVal)) + (smoothedVal  *  filterVal);
    return smoothedVal;
}








