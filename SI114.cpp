// SI114.h
// Code for the Modern Device Pulse Sensor
// Based on the SI1143 chip
// Heavily updated by Toby Corkindale to use the Wire library,
// amongst other changes. February 2016.
// https://github.com/TJC
// Original version by:
// paul@moderndevice.com 6-27-2012

#include "SI114.h"
#include <Wire.h>
// See https://www.arduino.cc/en/Reference/Wire

PulsePlug::PulsePlug()
{
  Wire.begin();
}

bool PulsePlug::isPresent() {
  beginTransmission();
  byte result = endTransmission();
  if (result == 0) {
    return true;
  }
  else {
    Serial.print("isPresent() error code = ");
    Serial.println(result);
    return false;
  }
}

byte PulsePlug::readParam (byte addr) {
    // read from parameter ram
    beginTransmission();
    Wire.write(PulsePlug::COMMAND);
    Wire.write(0x80 | addr); // PARAM_QUERY
    endTransmission();
    delay(10); // XXX Nothing in datasheet indicates this is required; was in original code.
    return getReg(PulsePlug::PARAM_RD);
}

byte PulsePlug::getReg (byte reg) {
    // get a register
    beginTransmission();
    Wire.write(reg);
    endTransmission();
    requestData(1);
    byte result = Wire.read();
    delay(10); // XXX Nothing in datasheet indicates this is required; was in original code.
    return result;
}

void PulsePlug::setReg (byte reg, byte val) {
    // set a register
    beginTransmission();
    Wire.write(reg);
    Wire.write(val);
    endTransmission();
    delay(10); // XXX Nothing in datasheet indicates this is required; was in original code.
}

void PulsePlug::id()
{
    Serial.print("PART: ");
    Serial.print(PulsePlug::getReg(PulsePlug::PART_ID));
    Serial.print(" REV: ");
    Serial.print(PulsePlug::getReg(PulsePlug::REV_ID));
    Serial.print(" SEQ: ");
    Serial.println(PulsePlug::getReg(PulsePlug::SEQ_ID));
}

void PulsePlug::initSensor()
{
    PulsePlug::setReg(PulsePlug::HW_KEY, 0x17);
    // pulsePlug.setReg(PulsePlug::COMMAND, PulsePlug::RESET_Cmd);
    //
    setReg(PulsePlug::INT_CFG, 0x03);       // turn on interrupts
    setReg(PulsePlug::IRQ_ENABLE, 0x10);    // turn on interrupt on PS3
    setReg(PulsePlug::IRQ_MODE2, 0x01);     // interrupt on ps3 measurement
    setReg(PulsePlug::MEAS_RATE, 0x84);     // 10ms measurement rate
    setReg(PulsePlug::ALS_RATE, 0x08);      // ALS 1:1 with MEAS
    setReg(PulsePlug::PS_RATE, 0x08);       // PS 1:1 with MEAS

    // Current setting for LEDs pulsed while taking readings
    // PS_LED21  Setting for LEDs 1 & 2. LED 2 is high nibble
    // each LED has 16 possible (0-F in hex) possible settings
    // see the SI114x datasheet.

    // These settings should really be automated with feedback from output
    // On my todo list but your patch is appreciated :)
    // support at moderndevice dot com.
    setReg(PulsePlug::PS_LED21, 0x39);      // LED current for 2 (IR1 - high nibble) & LEDs 1 (red - low nibble)
    setReg(PulsePlug::PS_LED3, 0x02);       // LED current for LED 3 (IR2)

    writeParam(PulsePlug::PARAM_CH_LIST, 0x77);         // all measurements on

    // increasing PARAM_PS_ADC_GAIN will increase the LED on time and ADC window
    // you will see increase in brightness of visible LED's, ADC output, & noise
    // datasheet warns not to go beyond 4 because chip or LEDs may be damaged
    writeParam(PulsePlug::PARAM_PS_ADC_GAIN, 0x00);

    // You can select which LEDs are energized for each reading.
    // The settings below (in the comments)
    // turn on only the LED that "normally" would be read
    // ie LED1 is pulsed and read first, then LED2 & LED3.
    writeParam(PulsePlug::PARAM_PSLED12_SELECT, 0x21);  // 21 select LEDs 2 & 1 (red) only
    writeParam(PulsePlug::PARAM_PSLED3_SELECT, 0x04);   // 4 = LED 3 only

    // Sensors for reading the three LEDs
    // 0x03: Large IR Photodiode
    // 0x02: Visible Photodiode - cannot be read with LEDs on - just for ambient measurement
    // 0x00: Small IR Photodiode
    writeParam(PulsePlug::PARAM_PS1_ADCMUX, 0x03);      // PS1 photodiode select
    writeParam(PulsePlug::PARAM_PS2_ADCMUX, 0x03);      // PS2 photodiode select
    writeParam(PulsePlug::PARAM_PS3_ADCMUX, 0x03);      // PS3 photodiode select 


    writeParam(PulsePlug::PARAM_PS_ADC_COUNTER, B01110000);    // B01110000 is default
    setReg(PulsePlug::COMMAND, PulsePlug::PSALS_AUTO_Cmd);     // starts an autonomous read loop
}

void PulsePlug::setLEDcurrents(byte LED1, byte LED2, byte LED3){
/* VLEDn = 1 V, PS_LEDn = 0001	5.6
VLEDn = 1 V, PS_LEDn = 0010	11.2
VLEDn = 1 V, PS_LEDn = 0011	22.4
VLEDn = 1 V, PS_LEDn = 0100	45
VLEDn = 1 V, PS_LEDn = 0101	67
VLEDn = 1 V, PS_LEDn = 0110	90
VLEDn = 1 V, PS_LEDn = 0111	112
VLEDn = 1 V, PS_LEDn = 1000	135
VLEDn = 1 V, PS_LEDn = 1001	157
VLEDn = 1 V, PS_LEDn = 1010	180
VLEDn = 1 V, PS_LEDn = 1011	202
VLEDn = 1 V, PS_LEDn = 1100	224
VLEDn = 1 V, PS_LEDn = 1101	269
VLEDn = 1 V, PS_LEDn = 1110	314
VLEDn = 1 V, PS_LEDn = 1111	359   */

LED1 = constrain(LED1, 0, 15);
LED2 = constrain(LED2, 0, 15);
LED3 = constrain(LED3, 0, 15);

PulsePlug::setReg(PulsePlug::PS_LED21, (LED2 << 4) | LED1 );
PulsePlug::setReg(PulsePlug::PS_LED3, LED3);

}

void PulsePlug::setLEDdrive(byte LED1pulse, byte LED2pulse, byte LED3pulse){
 // this sets which LEDs are active on which pulses
 // any or none of the LEDs may be active on each PulsePlug
 //000: NO LED DRIVE
 //xx1: LED1 Drive Enabled
 //x1x: LED2 Drive Enabled (Si1142 and Si1143 only. Clear for Si1141)
 //1xx: LED3 Drive Enabled (Si1143 only. Clear for Si1141 and Si1142)
 // example setLEDdrive(1, 2, 5); sets LED1 on pulse 1, LED2 on pulse 2, LED3, LED1 on pulse 3

PulsePlug::writeParam(PulsePlug::PARAM_PSLED12_SELECT, (LED1pulse << 4) | LED2pulse );  // select LEDs on for readings see datasheet
PulsePlug::writeParam(PulsePlug::PARAM_PSLED3_SELECT, LED3pulse);

}

// Returns ambient light values as an array
// First item is visual light, second is IR light.
uint16_t* PulsePlug::fetchALSData () {
    static uint16_t als_data[2];
    static uint16_t tmp;
    // read out all result registers as lsb-msb pairs of bytes
    beginTransmission();
    Wire.write(ALS_VIS_DATA0);
    endTransmission();
    requestData(4);

    for (int i=0; i<=1; i++) {
        als_data[i] = Wire.read();

        tmp = Wire.read();
        als_data[i] += (tmp << 8);
    }

    return als_data;
}

// Fetch data from the PS1, PS2 and PS3 registers.
// They are stored as LSB-MSB pairs of bytes there; convert them to 16bit ints here.
uint16_t* PulsePlug::fetchLedData() {
    static uint16_t ps[3];
    static uint16_t tmp;

    beginTransmission();
    Wire.write(PulsePlug::PS1_DATA0);
    endTransmission();
    requestData(6);

    for (int i=0; i<=2; i++) {
        ps[i] = Wire.read();

        tmp = Wire.read();
        ps[i] += (tmp << 8);
    }

    return ps;
}


void PulsePlug::writeParam (byte addr, byte val) {
    // write to parameter ram
    beginTransmission();
    Wire.write(PulsePlug::PARAM_WR);
    Wire.write(val);
    // auto-increments into PulsePlug::COMMAND
    Wire.write(0xA0 | addr); // PARAM_SET
    endTransmission();
    delay(10); // XXX Nothing in datasheet indicates this is required; was in original code.
}
