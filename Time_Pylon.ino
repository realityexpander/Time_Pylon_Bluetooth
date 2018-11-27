
// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

#define DEBUG false

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;

/*
  Bluetooth Serial & Accelerometer interface for Time Pylon for Time Mate project

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 https://www.teachmemicro.com/arduino-bluetooth/
 https://maker.pro/arduino/tutorial/how-to-interface-arduino-and-the-mpu-6050-sensor
 https://hackernoon.com/create-your-first-arduino-node-js-iot-visualization-app-in-under-15-minutes-619f8e6f7181

 The circuit:
 * RX is digital pin 9 (connect to TX of other device)
 * TX is digital pin 10 (connect to RX of other device)

 based on Mikal Hart's example

 */
#include <SoftwareSerial.h>

SoftwareSerial blueSerial(9, 10); // RX, TX
int c=0;
int side = -1;
int newSide = -1;
int newSideCount = 0;
int prevNewSide = -1;
float ax_f, ay_f, az_f = 0;
float oldax_f, olday_f, oldaz_f = 0;
  
void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif

  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  // set the data rate for the SoftwareSerial port
  blueSerial.begin(9600);

}

void loop() { 

  // Local echo
  if (blueSerial.available()) {
    Serial.write(blueSerial.read());
  }
  if (Serial.available()) {
    blueSerial.write(Serial.read());
  }

  c++;
  if(c>20000) {
    /// blueSerial.println(random(0,100));

    oldax_f = ax_f; 
    olday_f = ay_f;
    oldaz_f = az_f;
    
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    c=0;
    ax_f = ax/16383.0;
    ay_f = ay/16383.0;
    az_f = az/16383.0;
    
    Serial.print("{\"side\":");
    Serial.print(side);
    Serial.print(", \"ax\":");
    Serial.print(ax_f);
    Serial.print(", \"ay\":");
    Serial.print(ay_f);
    Serial.print(", \"az\":");
    Serial.print(az_f);
    Serial.println("}");

#if DEBUG
    blueSerial.print("{\"side\":");
    blueSerial.print(side);
    blueSerial.print(", \"ax\":");
    blueSerial.print(ax_f);
    blueSerial.print(", \"ay\":");
    blueSerial.print(ay_f);
    blueSerial.print(", \"az\":");
    blueSerial.print(az_f);
    blueSerial.println("}");
#endif

    newSide = -1; // Default to uninitalized
    if( ax_f > 0.00 && ax_f < 0.13 && ay_f > -0.05 && ay_f < 0.25 && az_f > 0.70 && az_f < 0.90 ) {
      newSide = 1;
    }
    if( ax_f > -0.90 && ax_f < -0.70 && ay_f > -0.20 && ay_f < 0.07 && az_f > 0.20 && az_f < 0.36 ) {
      newSide = 2;
    }
    if( ax_f > -0.90 && ax_f < -0.70 && ay_f > -0.15 && ay_f < 0.10 && az_f > -0.80 && az_f < -0.65 ) {
      newSide = 3;
    }
    if( ax_f > 0.02 && ax_f < 0.14 && ay_f > -0.25 && ay_f < 0.10 && az_f > -1.25 && az_f < -1.10 ) {
      newSide = 4;
    }
    if( ax_f > 0.85 && ax_f < 1.00 && ay_f > -0.13 && ay_f < 0.00 && az_f > -0.80 && az_f < -0.60 ) {
      newSide = 5;
    }
    if( ax_f > 0.80 && ax_f < 0.99 && ay_f > -0.07 && ay_f < 0.18 && az_f > 0.24 && az_f < 0.38 ) {
      newSide = 6;
    }
    if( ax_f > -0.05 && ax_f < 0.18 && ay_f > -1.08 && ay_f < -.90 && az_f > -0.25 && az_f < 0.05 ) {
      newSide = 7;
    }

    if (newSide != -1 && newSide != side) {
      if(side == -1) { // initial state
        side = newSide;
        outputSide();
      }
      
      if (newSide == prevNewSide) {
        newSideCount++;
      } else {
        newSideCount = 0;
        prevNewSide = newSide;
      }
    }
    if(newSide != -1 && newSideCount > 3 && newSide != side) {
      side = newSide;
      newSideCount = 0;
      outputSide();
    }
    
    
  }

}

void outputSide() {
  blueSerial.print("{\"side\":");
  blueSerial.print(side);
  blueSerial.println("}");
  
  Serial.print("{\"side\":");
  Serial.print(side);
  Serial.println("}");
}
