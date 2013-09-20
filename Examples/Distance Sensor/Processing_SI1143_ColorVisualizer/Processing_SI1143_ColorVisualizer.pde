/* Processing code for the Modern Device SI1143 based Proximity Sensor
 * http://moderndevice.com/product/si1143-proximity-sensors/
 * The sketch receives three tab-separated values;
 * An angle, a vector size, and a total light amount quantity and translates
 * these values into a color in HSB colorspace. The vector is also imaged as a black line
 * radiating from the center of the screen.
 * Paul Badger, Gavin Atkinson, Jean-Claude Wippler  2013
 */


import processing.serial.*;

color c;

int cx, cy;
Serial myPort;
float radius;
int  totalDist;
float angle;


void setup() {
    size(700, 700);

    // List all the available serial ports
    println(Serial.list());
    // I know that the first port in the serial list on my mac
    // is always my  Arduino, so I open Serial.list()[0].
    // Open whatever port is the one you're using.
    
    // Open the port you are using at the rate you want:
      myPort = new Serial(this, Serial.list()[0], 57600);

    // don't generate a serialEvent() unless you get a newline character:
    myPort.bufferUntil('\n');

    radius = ((min(width, height) / 2) - 30);
    cx = width / 2;
    cy = height / 2;
}

void draw() {
}

void serialEvent(Serial myPort) { 
    // get the ASCII string:
    String inString = myPort.readStringUntil('\n');

    if (inString != null) {
        // trim off any whitespace:
        

        inString = trim(inString);
        // split the string on the tabs and convert the 
        // resulting substrings into an integer array:
       
        float dataArray[] = float(split(inString, "\t"));
        // if the array has at least three elements, you know
        // you got the whole thing.  Put the numbers in the
        // color variables:
        if (dataArray.length >=3) { // make sure we have good data

            print (dataArray [0]);
            print(",  ");
            print (dataArray [1]);
            print (",  ");     
            println (dataArray [2]);

            colorMode(HSB, 360);  // Use HSB with scale of 0-100
            // map our angle to the hue and our size vector to brightness
            // did we scale the brightness to anything that makes sense?

            dataArray[1] = map(dataArray[1], 0, 4000, 0, 360);
            totalDist = (int)map(dataArray[2], 0, 10000, 100, 360);
            constrain(dataArray[1], 0, 360);
            constrain( totalDist, 0, 360);
            c = color(dataArray[0], dataArray[1], totalDist );  // c is the background color
            angle = dataArray[0];

            // set the background color with the color values:
            background(c);
            //  dataArray[1] = map(dataArray[1], 0, 14000, 0, 360);
            strokeWeight(6);

            angle = ( angle + 210 ) % 360;   // change some mapping for LED position
            angle = radians(angle);

            // draw one line all the way to the outside circle to show direction
            float radius2 = ((min(width, height) / 2) - 30);
            float xt = cx + cos(angle) * radius2;
            float yt = cy + sin(angle) * radius2;
            stroke(360 - dataArray[0], dataArray[1], 180);
            //line(cx, cy, xt, yt);
            stroke(0, 0, 0);
            strokeWeight(12);


            radius = map(dataArray[1], 0, 360, 0, (min(width, height) / 2));
            float x = cx + cos(angle) * radius;
            float y = cy + sin(angle) * radius;
            line(cx, cy, x, y);
            //  text("The value of variable v is "+ dataArray[1], 20 , 680);
        }
    }
}



