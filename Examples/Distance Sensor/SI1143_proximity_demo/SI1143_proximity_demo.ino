/* SI1143_proximity_demo.ino
 * http://moderndevice.com/product/si1143-proximity-sensors/
 * Reads the Proximity Sensor and either prints raw LED data or angular data 
 * depending the options chosen in the #define's below
 * Paul Badger, Gavin Atkinson, Jean-Claude Wippler  2013
 */


/*
  For Arduino users use the following pins for various ports
  Connect to pins with 5k series resistors (inputs are 3.3V only)
  Connect Ground.
  Connect 3.3V line to 3.3 volts (PWR line on sensors are not connected).
  On the Arduino Leonardo, "port 0" connects to the SDA/SCL pins on the board-- 2 and 3, respectively.
  
  
  For JeeNode users, just set the port used
  
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
  Tip: SCL is always on the outside edge

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
// #define POWERLINE_SAMPLING     // samples on an integral of a power line period [eg 1/60 sec]
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
    while (!Serial) ;
    digitalWrite(3, HIGH);

    pulse.setReg(PulsePlug::HW_KEY, 0x17);  
    // pulse.setReg(PulsePlug::COMMAND, PulsePlug::RESET_Cmd);

    Serial.print("PART: "); 
    Serial.print(pulse.getReg(PulsePlug::PART_ID)); 
    Serial.print(" REV: "); 
    Serial.print(pulse.getReg(PulsePlug::REV_ID)); 
    Serial.print(" SEQ: "); 
    Serial.println(pulse.getReg(PulsePlug::SEQ_ID)); 

    pulse.setReg(PulsePlug::INT_CFG, 0x03);       // turn on interrupts
    pulse.setReg(PulsePlug::IRQ_ENABLE, 0x10);    // turn on interrupt on PS3
    pulse.setReg(PulsePlug::IRQ_MODE2, 0x01);     // interrupt on ps3 measurement
    pulse.setReg(PulsePlug::MEAS_RATE, 0x84);     // see datasheet
    pulse.setReg(PulsePlug::ALS_RATE, 0x08);      // see datasheet
    pulse.setReg(PulsePlug::PS_RATE, 0x08);       // see datasheet
    pulse.setReg(PulsePlug::PS_LED21, 0x66 );      // LED current for LEDs 1 (red) & 2 (IR1)
    pulse.setReg(PulsePlug::PS_LED3, 0x06);       // LED current for LED 3 (IR2)

    Serial.print( "PS_LED21 = ");                                         
    Serial.println(pulse.getReg(PulsePlug::PS_LED21), BIN);                                          
    Serial.print("CHLIST = ");
    Serial.println(pulse.readParam(0x01), BIN);

    pulse.writeParam(PulsePlug::PARAM_CH_LIST, 0x77);         // all measurements on

    // increasing PARAM_PS_ADC_GAIN will increase the LED on time and ADC window
    // you will see increase in brightness of visible LED's, ADC output, & noise 
    // datasheet warns not to go beyond 4 because chip or LEDs may be damaged
    pulse.writeParam(PulsePlug::PARAM_PS_ADC_GAIN, 0x00);

    pulse.writeParam(PulsePlug::PARAM_PSLED12_SELECT, 0x21);  // select LEDs on for readings see datasheet
    pulse.writeParam(PulsePlug::PARAM_PSLED3_SELECT, 0x04);   //  3 only
    pulse.writeParam(PulsePlug::PARAM_PS1_ADCMUX, 0x03);      // PS1 photodiode select
    pulse.writeParam(PulsePlug::PARAM_PS2_ADCMUX, 0x03);      // PS2 photodiode select
    pulse.writeParam(PulsePlug::PARAM_PS3_ADCMUX, 0x03);      // PS3 photodiode select  

    pulse.writeParam(PulsePlug::PARAM_PS_ADC_COUNTER, B01110000);    // B01110000 is default                                   
    pulse.setReg(PulsePlug::COMMAND, PulsePlug::PSALS_AUTO_Cmd);     // starts an autonomous read loop
    // Serial.println(pulse.getReg(PulsePlug::CHIP_STAT), HEX);  

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

 #ifdef POWERLINE_SAMPLING

    while (millis() - start < 16){   // 60 hz - or use 33 for two cycles
                                     // 50 hz in Europe use 20, or 40

 #else

        while (i < samples){ 

 #endif


 #ifdef PRINT_AMBIENT_LIGHT_SAMPLING   

        pulse.fetchData();

 #else
 
        pulse.fetchLedData();

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
//    Basically a ratio of differences of LED values to each other
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








