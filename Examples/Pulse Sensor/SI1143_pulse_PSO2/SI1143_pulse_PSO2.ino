/* SI114_pulse_PSO2.ino 
 * A version of SI114_Pulse_Demo.ino with PSO2 sensing added
 * demo code for the Modern Device SI1143-based pulse sensor
 * http://moderndevice.com/product/pulse-heartbeat-sensor-pulse-sensor-1x/
 * Paul Badger 2013 with plenty of coding help from Jean-Claude Wippler
 * Hardware setup - please read the chart below and set the appropriate options
 * See the #defines for various printing options
 * This is experimental software (and pre-beta) at that, it is not suitable 
 * for any particular purpose. No life-critical devices should
 * be based on this software.
 * Code in the public domain but credit for the software is nice :)
 */
 
 #include <SI114.h>
 
 const int portForSI114 = 1;        // change to the JeeNode port number used

/*
 For Arduino users just use the following pins for various port settings
 Or use port 0 for traditional SDA (A4) and SCL (A5)
 Connect pins with 10k resistors in series
 
 JeeNode users just set the appropriate port
 
 JeeNode Port  SCL ('duino pin)  SDA ('duino pin)
 0             18 (A5)       19 (A4)
 1             4             14 (A0)
 2             5             15 (A1)
 3             6             16 (A2)
 4             7             17 (A3)
 */


const int SAMPLES_TO_AVERAGE = 5;             // samples for smoothing 1 to 10 seem useful 5 is default
// increase for smoother waveform (with less resolution - slower!) 


//#define SEND_TOTAL_TO_PROCESSING   // Use this option exclusive of other options
                                      // for sending data to HeartbeatGraph in Processing
// #define POWERLINE_SAMPLING         // samples on an integral of a power line period [eg 1/60 sec]
// #define AMBIENT_LIGHT_SAMPLING     // also samples ambient slight (slightly slower)
 // #define PRINT_LED_VALS             // print LED raw values
 #define GET_PULSE_READING            // prints HB, signal size, PSO2 ratio


int binOut;     // 1 or 0 depending on state of heartbeat
int BPM;
unsigned long red;        // read value from visible red LED
unsigned long IR1;        // read value from infrared LED1
unsigned long IR2;       // read value from infrared LED2
unsigned long IR_total;     // IR LED reads added together



PortI2C myBus (portForSI114);
PulsePlug pulse (myBus); 

void setup () {
    Serial.begin(57600);
    Serial.println("\n Pulse_demo ");

    if (pulse.isPresent()) {
        Serial.print("SI114x Pulse Sensor found on Port ");
        Serial.println(portForSI114);
    }
        else{
        Serial.print("No SI114x found on Port ");
        Serial.println(portForSI114);
    }
    Serial.begin(57600);
    digitalWrite(3, HIGH);

    initPulseSensor();
}


void loop(){
    readPulseSensor();
}



// simple smoothing function for  heartbeat detection and processing
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


void initPulseSensor(){

    pulse.setReg(PulsePlug::HW_KEY, 0x17);  
    // pulse.setReg(PulsePlug::COMMAND, PulsePlug::RESET_Cmd);

    Serial.print("PART: "); 
    Serial.print(pulse.getReg(PulsePlug::PART_ID)); 
    Serial.print(" REV: "); 
    Serial.println(pulse.getReg(PulsePlug::REV_ID)); 
    Serial.print(" SEQ: "); 
    Serial.println(pulse.getReg(PulsePlug::SEQ_ID)); 

    pulse.setReg(PulsePlug::INT_CFG, 0x03);       // turn on interrupts
    pulse.setReg(PulsePlug::IRQ_ENABLE, 0x10);    // turn on interrupt on PS3
    pulse.setReg(PulsePlug::IRQ_MODE2, 0x01);     // interrupt on ps3 measurement
    pulse.setReg(PulsePlug::MEAS_RATE, 0x84);     // see datasheet
    pulse.setReg(PulsePlug::ALS_RATE, 0x08);      // see datasheet
    pulse.setReg(PulsePlug::PS_RATE, 0x08);       // see datasheet

    // Current setting for LEDs pulsed while taking readings
    // PS_LED21  Setting for LEDs 1 & 2. LED 2 is high nibble
    // each LED has 16 possible (0-F in hex) possible settings
    // see the SI114x datasheet.
    
    // These settings should really be autimated with feedback from output
    // On my todo list but your patch is appreciated :)
    // support at moderndevice dot com.
    pulse.setReg(PulsePlug::PS_LED21, 0x39);      // LED current for 2 (IR1 - high nibble) & LEDs 1 (red - low nibble) 
    pulse.setReg(PulsePlug::PS_LED3, 0x02);       // LED current for LED 3 (IR2)

/*  debug infor for the led currents
    Serial.print( "PS_LED21 = ");                                         
    Serial.println(pulse.getReg(PulsePlug::PS_LED21), BIN);                                          
    Serial.print("CHLIST = ");
    Serial.println(pulse.readParam(0x01), BIN);
*/
    pulse.writeParam(PulsePlug::PARAM_CH_LIST, 0x77);         // all measurements on

    // increasing PARAM_PS_ADC_GAIN will increase the LED on time and ADC window
    // you will see increase in brightness of visible LED's, ADC output, & noise 
    // datasheet warns not to go beyond 4 because chip or LEDs may be damaged
    pulse.writeParam(PulsePlug::PARAM_PS_ADC_GAIN, 0x00);


    // You can select which LEDs are energized for each reading.
    // The settings below (in the comments)
    // turn on only the LED that "normally" would be read
    // ie LED1 is pulsed and read first, then LED2 & LED3.
    pulse.writeParam(PulsePlug::PARAM_PSLED12_SELECT, 0x21);  // 21 select LEDs 2 & 1 (red) only                                                               
    pulse.writeParam(PulsePlug::PARAM_PSLED3_SELECT, 0x04);   // 4 = LED 3 only

    // Sensors for reading the three LEDs
    // 0x03: Large IR Photodiode
    // 0x02: Visible Photodiode - cannot be read with LEDs on - just for ambient measurement
    // 0x00: Small IR Photodiode
    pulse.writeParam(PulsePlug::PARAM_PS1_ADCMUX, 0x03);      // PS1 photodiode select 
    pulse.writeParam(PulsePlug::PARAM_PS2_ADCMUX, 0x03);      // PS2 photodiode select 
    pulse.writeParam(PulsePlug::PARAM_PS3_ADCMUX, 0x03);      // PS3 photodiode select  

    pulse.writeParam(PulsePlug::PARAM_PS_ADC_COUNTER, B01110000);    // B01110000 is default                                   
    pulse.setReg(PulsePlug::COMMAND, PulsePlug::PSALS_AUTO_Cmd);     // starts an autonomous read loop
    // Serial.println(pulse.getReg(PulsePlug::CHIP_STAT), HEX);  
    Serial.print("end init");
}

void readPulseSensor(){

    static int foundNewFinger, red_signalSize, red_smoothValley;
    static long red_valley, red_Peak, red_smoothRedPeak, red_smoothRedValley, 
               red_HFoutput, red_smoothPeak; // for PSO2 calc
    static  int IR_valley=0, IR_peak=0, IR_smoothPeak, IR_smoothValley, binOut, lastBinOut, BPM;
    static unsigned long lastTotal, lastMillis, IRtotal, valleyTime = millis(), lastValleyTime = millis(), peakTime = millis(), lastPeakTime=millis(), lastBeat, beat;
    static float IR_baseline, red_baseline, IR_HFoutput, IR_HFoutput2, shiftedOutput, LFoutput, hysterisis;

    unsigned long total=0, start;
    int i=0;
    int IR_signalSize;
    red = 0;
    IR1 = 0;
    IR2 = 0;
    total = 0;
    start = millis();

         
    
     #ifdef POWERLINE_SAMPLING
     
     while (millis() - start < 16){   // 60 hz - or use 33 for two cycles
     // 50 hz in Europe use 20, or 40
       Serial.print("sample");
     #else     
     while (i < SAMPLES_TO_AVERAGE){      
     #endif
     
     
     #ifdef AMBIENT_LIGHT_SAMPLING   
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
    total =  IR1 + IR2 + red;  // red excluded
    IRtotal = IR1 + IR2;
    
   

#ifdef AMBIENT_LIGHT_SAMPLING

    Serial.print(pulse.resp, HEX);     // resp
    Serial.print("\t");
    Serial.print(pulse.als_vis);       //  ambient visible
    Serial.print("\t");
    Serial.print(pulse.als_ir);        //  ambient IR
    Serial.print("\t");

#endif


#ifdef PRINT_LED_VALS

    Serial.print(red);
    Serial.print("\t");
    Serial.print(IR1);
    Serial.print("\t");
    Serial.print(IR2);
    Serial.print("\t");
    Serial.println((long)total);   

#endif

 #ifdef SEND_TOTAL_TO_PROCESSING
    Serial.println(total);
 #endif

 #ifdef GET_PULSE_READING

    // except this one for Processing heartbeat monitor
    // comment out all the bottom print lines

    if (lastTotal < 20000L && total > 20000L) foundNewFinger = 1;  // found new finger!

    lastTotal = total;
     
    // if found a new finger prime filters first 20 times through the loop
    if (++foundNewFinger > 25) foundNewFinger = 25;   // prevent rollover 

    if ( foundNewFinger < 20){
        IR_baseline = total - 200;   // take a guess at the baseline to prime smooth filter
   Serial.println("found new finger");     
    }
    
    else if(total > 20000L) {    // main running function
    
    
        // baseline is the moving average of the signal - the middle of the waveform
        // the idea here is to keep track of a high frequency signal, HFoutput and a 
        // low frequency signal, LFoutput
        // The LF signal is shifted downward slightly downward (heartbeats are negative peaks)
        // The high freq signal has some hysterisis added. 
        // When the HF signal crosses the shifted LF signal (on a downward slope), 
        // we have found a heartbeat.
        IR_baseline = smooth(IRtotal, 0.99, IR_baseline);   // 
        IR_HFoutput = smooth((IRtotal - IR_baseline), 0.2, IR_HFoutput);    // recycling output - filter to slow down response
        
        red_baseline = smooth(red, 0.99, red_baseline); 
        red_HFoutput = smooth((red - red_HFoutput), 0.2, red_HFoutput);
        
        // beat detection is performed only on the IR channel so 
        // fewer red variables are needed
        
        IR_HFoutput2 = IR_HFoutput + hysterisis;     
        LFoutput = smooth((IRtotal - IR_baseline), 0.95, LFoutput);
        // heartbeat signal is inverted - we are looking for negative peaks
        shiftedOutput = LFoutput - (IR_signalSize * .05);

        if (IR_HFoutput  > IR_peak) IR_peak = IR_HFoutput; 
        if (red_HFoutput  > red_Peak) red_Peak = red_HFoutput;
        
        // default reset - only if reset fails to occur for 1800 ms
        if (millis() - lastPeakTime > 1800){  // reset peak detector slower than lowest human HB
            IR_smoothPeak =  smooth((float)IR_peak, 0.6, (float)IR_smoothPeak);  // smooth peaks
            IR_peak = 0;
            
            red_smoothPeak =  smooth((float)red_Peak, 0.6, (float)red_smoothPeak);  // smooth peaks
            red_Peak = 0;
            
            lastPeakTime = millis();
        }

        if (IR_HFoutput  < IR_valley)   IR_valley = IR_HFoutput;
        if (red_HFoutput  < red_valley)   red_valley = red_HFoutput;
        
  /*      if (IR_valley < -1500){
            IR_valley = -1500;  // ditto above
            Serial.println("-1500");
        } 
        if (red_valley < -1500) red_valley = -1500;  // ditto above  */



        if (millis() - lastValleyTime > 1800){  // insure reset slower than lowest human HB
            IR_smoothValley =  smooth((float)IR_valley, 0.6, (float)IR_smoothValley);  // smooth valleys
            IR_valley = 0;
            lastValleyTime = millis();           
        }

   //     IR_signalSize = IR_smoothPeak - IR_smoothValley;  // this the size of the smoothed HF heartbeat signal
        hysterisis = constrain((IR_signalSize / 15), 35, 120) ;  // you might want to divide by smaller number
                                                                // if you start getting "double bumps"
            
        // Serial.print(" T  ");
        // Serial.print(IR_signalSize); 

        if  (IR_HFoutput2 < shiftedOutput){
            // found a beat - pulses are valleys
            lastBinOut = binOut;
            binOut = 1;
         //   Serial.println("\t1");
            hysterisis = -hysterisis;
            IR_smoothValley =  smooth((float)IR_valley, 0.99, (float)IR_smoothValley);  // smooth valleys
            IR_signalSize = IR_smoothPeak - IR_smoothValley;
            IR_valley = 0x7FFF;
            
            red_smoothValley =  smooth((float)red_valley, 0.99, (float)red_smoothValley);  // smooth valleys
            red_signalSize = red_smoothPeak - red_smoothValley;
            red_valley = 0x7FFF;
            
            lastValleyTime = millis();
             
        } 
        else{
         //   Serial.println("\t0");
            lastBinOut = binOut;
            binOut = 0;
            IR_smoothPeak =  smooth((float)IR_peak, 0.99, (float)IR_smoothPeak);  // smooth peaks
            IR_peak = 0;
            
            red_smoothPeak =  smooth((float)red_Peak, 0.99, (float)red_smoothPeak);  // smooth peaks
            red_Peak = 0;
            lastPeakTime = millis();      
            } 

  if (lastBinOut == 1 && binOut == 0){
      Serial.println(binOut);
  }

        if (lastBinOut == 0 && binOut == 1){
            lastBeat = beat;
            beat = millis();
            BPM = 60000 / (beat - lastBeat);
            Serial.print(binOut);
            Serial.print("\t BPM ");
            Serial.print(BPM);  
            Serial.print("\t IR ");
            Serial.print(IR_signalSize);
            Serial.print("\t PSO2 ");         
            Serial.println(((float)red_baseline / (float)(IR_baseline/2)), 3);                     
        }

    }
 #endif
}



