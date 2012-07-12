/* SI114_Pulse_Demo.ino
 * code for the Modern Device SI1143-based pulse sensor
 * code in the public domain
 */

#include <SI114.h>

const int samples = 1;            // samples for smoothing 1 to 10 seem useful
// increase for smoother waveform (with less resolution) 

// #define AMBIENT_LIGHT_SAMPLING // also samples ambient slight (slightly slower)

const int portForSI114 = 1;       // change to the JeeNode port number used

/*
   For Arduino users use the following pins for various ports
 Connect pins with 10k resistors in series
 JeeNode Port  SCL ('duino pin)  SDA ('duino pin)
 1             4             14 (A0)
 2             5             15 (A1)
 3             6             16 (A2)
 4             7             17 (A3)
 */
 
 
/*  LED currents listed in the datasheet
Value   Current in mA
0        0
1        5.6
2        11.2
3        22.4
4        45
5        67
6        90
7        112
8        135
9        157
10       180
11       202
12       224
13       269
14       314
14       359
*/

const int LED1_brightness = 7;    // LED brightnesses from 0 to 15 are valid
const int LED2_brightness = 2; 
const int LED3_brightness = 1; 


unsigned long lastMillis;
volatile unsigned long red, IR1, IR2;       // volatiles for use in Interrupt Service Routine (ISR)
volatile boolean readFlag = false;
volatile int samplesRead;

PortI2C myBus (portForSI114);
PulsePlug pulse (myBus); 

void setup () {
    Serial.begin(57600);
    Serial.println("\n[pulse_demo]");

    if (!pulse.isPresent()) {
        Serial.print("No SI114x found on Port ");
        Serial.println(portForSI114);
    }



    digitalWrite(3, HIGH);                // set pullup on interrupt line
    attachInterrupt(1, readPhotoDiode, FALLING);    // setup interrupt for reading photodectectors 

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
    pulse.setReg(PulsePlug::IRQ_STATUS, 0x10);    // 
    pulse.setReg(PulsePlug::IRQ_MODE1, 0x00);     // interrupt on ps3 measurement
    pulse.setReg(PulsePlug::IRQ_MODE2, 0x00);     // interrupt on ps3 measurement
    pulse.setReg(PulsePlug::MEAS_RATE, 0x88);     // see datasheet 0x84 default
    pulse.setReg(PulsePlug::ALS_RATE, 0x08);      // see datasheet
    pulse.setReg(PulsePlug::PS_RATE, 0x08);       // see datasheet default 0x08
    pulse.setReg(PulsePlug::PS_LED21, (((LED2_brightness & 0x0F) << 4) + (LED1_brightness & 0x0F)));   // LED current for LEDs 1 (red) & 2 (IR1)
    pulse.setReg(PulsePlug::PS_LED3, (LED3_brightness & 0x0F));                                        // LED current for LED 3 (IR2)

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

    unsigned long total=0, start;
    int i=0;
    red = 0;
    IR1 = 0;
    IR2 = 0;
    total = 0;
    start = millis();


    // pulse.setReg(PulsePlug::IRQ_STATUS, 0x10);

    readFlag = true;    // set read via interrupt
    samplesRead = 0;
    pulse.setReg(PulsePlug::IRQ_STATUS, 0x10);  // reset interrupt

    while (readFlag == true){ 
    }

    red = red / samples;    // get averages
    IR1 = IR1 / samples;
    IR2 = IR2 / samples;
    total = red + IR1 + IR2;


#ifdef AMBIENT_LIGHT_SAMPLING

    Serial.print(pulse.resp, HEX);     // resp
    Serial.print("\t");
    Serial.print(pulse.als_vis);       //  ambient visible
    Serial.print("\t");
    Serial.print(pulse.als_ir);        //  ambient IR
    Serial.print("\t");

#endif

//        Serial.print(red);
//        Serial.print("\t");
//        Serial.print(IR1);
//        Serial.print("\t");
//        Serial.print(IR2);
//        Serial.print("\t");
          Serial.println((long)total);  // comment out all the bottom print lines
                                        // except this one for Processing heartbeat monitor


}


// simple smoothing function for future heartbeat detection and processing
float smooth(float data, float filterVal, float smoothedVal){

    if (filterVal > 1){      // check to make sure param's are within range
        filterVal = .99;
    }
    else if (filterVal <= 0.0){
        filterVal = 0.01;
    }

    smoothedVal = (data * (1.0 - filterVal)) + (smoothedVal  *  filterVal);
    return smoothedVal;
}




void readPhotoDiode(){   // ISR
noInterrupts();

if (readFlag == true){
    if (samplesRead < samples){

#ifdef AMBIENT_LIGHT_SAMPLING   

        pulse.fetchData();

#else

        pulse.fetchLedData();

#endif

        red += pulse.ps1;
        IR1 += pulse.ps2;
        IR2 += pulse.ps3;
        samplesRead++;
        
         if (samplesRead >= samples)  readFlag = false;
         else  pulse.setReg(PulsePlug::IRQ_STATUS, 0x10);  // reset interrupt
        
    }
}

interrupts() ;
}









