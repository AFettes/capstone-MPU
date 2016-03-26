#include<Wire.h>
#include<IRLib.h>
#include<Time.h>


boolean ContTv1 = true;
boolean ContTv2 = true;
boolean ContTv3 = true;

// This variable keeps track of what mode the control is in
// 0 is the default mode
// 1 is tilted to the right
// 2 is tilted to the left
// More to be added later
int CONTROL_MODE = 0;
#define DEFAULT_MODE 0
#define RIGHT 1
#define LEFT 2

// These numbers keep track of for how many counts the device has been tilted a certain direction.
// This allows us to perform certain actions once the MPU has been tilted for a couple seconds, such
// as change it's mode or send signals
int rightTilt, leftTilt, forwardTilt, backTilt;

// This integer keeps track of how much the user has shaken the MPU over the last 1 second. If the value
// increaseses past a certain amount, action happen
int32_t shake;

// We use a timer to make more complex gestures
int start;



IRsend My_Sender;

const int MPU=0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  
  pinMode(13,OUTPUT);
  pinMode(12,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(10,OUTPUT);

}

void loop(){
  int GyXshake;

  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);  // request a total of 14 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L) // temperature 
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
  //delay(333);

  // We check if the device is tilted in various directions, and modify the tilt variables accordingly
  if (AcY < -14000) {
    rightTilt++;
    leftTilt = 0;
  }
  if (AcY > 14000) {
    leftTilt++;
    rightTilt = 0;
  }
  if (AcY > -8000 && AcY < 8000) {
    rightTilt = 0;
    leftTilt = 0;
    CONTROL_MODE = DEFAULT_MODE;
//    digitalWrite(13, false);
//    ContTv1 = false;
//    digitalWrite(12, false);
//    ContTv2 = false;
  }
  if (AcX < -14000) {
    backTilt++;
    forwardTilt = 0;
  }
  if (AcX > 14000) {
    forwardTilt++;
    backTilt = 0;
  }
  if (AcX > -8000 && AcX < 8000) {
    forwardTilt = 0;
    backTilt = 0;
    // CONTROL_MODE = 0; probably dont have this here
  }

  // Increment the shake variable
  //if (GyX > 
  if (shake > 0) {
    shake = shake+abs(GyX)-10000;
  } else {
    shake = shake+abs(GyX);
  }

  if (rightTilt > 10) {
    CONTROL_MODE = RIGHT;
  }
  if (leftTilt > 10) {
    CONTROL_MODE = LEFT;
//    digitalWrite(12, true);
//    ContTv2 = true;
  }
  //Serial.print("Checking conditions for light changing \n");
  if (CONTROL_MODE == RIGHT && AcX > 7000 && (now()-start)> 1) {
    //Serial.print("Turning the light on or off \n");
    digitalWrite(13, !ContTv1);
    ContTv1 = !ContTv1;
    start = now();
  }

  if (CONTROL_MODE == RIGHT && AcX < -7000 && (now()-start)> 1) {
    //Serial.print("Turning the light on or off \n");
    digitalWrite(12, !ContTv2);
    ContTv2 = !ContTv2;
    start = now();
  }
  
  if (shake > 100000) {
    digitalWrite(13, !ContTv1);
    ContTv1 = !ContTv1;
    digitalWrite(12, !ContTv2);
    ContTv2 = !ContTv2;
    shake = 0;
  }
  // Tap Down
//  if (AcY < -6000) {
//    digitalWrite(13,ContTv1);
//    ContTv1 = !ContTv1;
//
//    for (int i=0; i<20; i++)
//    {
//      My_Sender.send(NECX, 0xE0E040BF, 32);
//    // Send IR signal
//    }
//  //delay(1800); 
//  }
  
//  //Tap Up
//  if (AcY > 8000) {
//    digitalWrite(12,ContTv2);
//    ContTv2 = !ContTv2;
//    delay(1800);
//  }
  
/*
  if (AcX < -15000 && AcY < 0 && AcZ < 1700) {
  digitalWrite(6,ContTv2);
  ContTv2 = !ContTv2;
  delay(1800);
  } */
}
