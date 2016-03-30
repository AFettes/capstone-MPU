#include<Wire.h>
#include<IRLib.h>
#include<Time.h>


boolean ContTv1 = false;
boolean ContTv2 = false;
boolean ContTv3 = false;

// This variable keeps track of what mode the control is in
// 0 is the default mode (not receiving instructions)
// 1 is active mode (able to input instructions)
// 2 is active and tilted left
// 3 is active and tilted right
// 4 is active and tilted back
// More to be added later
int CONTROL_MODE = 0;
#define DEFAULT_MODE 0
#define ACTIVE 1
#define LEFTACTIVE 2
#define RIGHTACTIVE 3
#define BACKACTIVE 4
#define FORWARDACTIVE 5

// These vectors correspond to various motions
int16_t rightflickup[6] = {-1, -1, -1, 1, 1, 1};
int16_t rightflickdown[6] = {1, 1, 1, -1, -1, -1};
int16_t leftflickup[6] = {1, 1, 1, -1, -1, -1};
int16_t leftflickdown[6] = {-1, -1, -1, 1, 1, 1};
int16_t backflickright[6] = {1, 1, 1, -1, -1, -1};
int16_t backflickleft[6] = {-1, -1, -1, 1, 1, 1};
int16_t forwardflickright[6] = {-1, -1, -1, 1, 1, 1};
int16_t forwardflickleft[6] = {1, 1, 1, -1, -1, -1};


// These numbers keep track of for how many counts the device has been tilted a certain direction.
// This allows us to perform certain actions once the MPU has been tilted for a couple seconds, such
// as change it's mode or send signals
int rightTilt, leftTilt, forwardTilt, backTilt;

// This integer keeps track of how much the user has shaken the MPU over the last 1 second. If the value
// increaseses past a certain amount, action happen
int32_t shake;

// We use a timer to make more complex gestures
int start = 0;

// These variables keep track of the last 10 values read from the gyroscope
// The 0th value is the most recent measurement
int16_t histGyZ[30] = {0};
int16_t histGyX[30] = {0};
int16_t histGyY[30] = {0};

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
  int recentvelocity = 0;

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
//  Serial.print("AcX = "); Serial.print(AcX);
//  Serial.print(" | AcY = "); Serial.print(AcY);
//  Serial.print(" | AcZ = "); Serial.print(AcZ);
//  Serial.print(" | Tmp = "); Serial.print(Tmp/340.00+36.53);  //equation for temperature in degrees C from datasheet
//  Serial.print(" | GyX = "); Serial.print(GyX);
//  Serial.print(" | GyY = "); Serial.print(GyY);
//  Serial.print(" | GyZ = "); Serial.println(GyZ);
  //delay(333);


//  Serial.print("Last 4 values are ");
//  for (int j = 0; j < 4; j++) {
//    Serial.print(histGyZ[j]);
//    Serial.print(" ");
//  }
//  Serial.print("\n");

//  Serial.print("MODE IS ");
//  Serial.print(CONTROL_MODE);
//  Serial.print("\n");

  // update the history variables
  for (int i = 29; i > 0; i--) {
    histGyZ[i] = histGyZ[i-1];
    histGyX[i] = histGyX[i-1];
    histGyY[i] = histGyY[i-1];
    recentvelocity += abs(histGyY[i])/1000 + abs(histGyX[i])/1000 + abs(histGyZ[i])/1000;
  }
  histGyZ[0] = GyZ;
  histGyX[0] = GyX;
  histGyY[0] = GyY;

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
  }
  if (AcX < -12000) {
    backTilt++;
    forwardTilt = 0;
  }
  if (AcX > 12000) {
    forwardTilt++;
    backTilt = 0;
  }
  if (AcX > -8000 && AcX < 8000) {
    forwardTilt = 0;
    backTilt = 0;
  }

  if (abs(GyX) > 12000) {
    shake = shake + 2;
  } else if (shake > 0) {
    shake--;
  }

  if (CONTROL_MODE > 0) {
    
    // Override the current mode if the user hasn't moved the device much in 30 counts

    if (recentvelocity == 0) {
      if (abs(AcZ) > 14000) {
        CONTROL_MODE = 1;
      } else if (abs(AcX) < abs(AcY)){
        if (AcY > 0) {
          CONTROL_MODE = LEFTACTIVE;
        } else {
          CONTROL_MODE = RIGHTACTIVE;
        }
      } else {
        if (AcX > 0) {
          CONTROL_MODE = FORWARDACTIVE;
        } else {
          CONTROL_MODE = BACKACTIVE;
        }
      }
    }
    
    // If device is active
    if (CONTROL_MODE == ACTIVE) {
      if (rightTilt > 10) {
          CONTROL_MODE = RIGHTACTIVE;
      } else if (leftTilt > 10){
          CONTROL_MODE = LEFTACTIVE;
      } else if (backTilt > 10){
          CONTROL_MODE = BACKACTIVE;
      } else if (forwardTilt > 10){
          CONTROL_MODE = FORWARDACTIVE;
      }
    } else if (AcZ > 12000 && abs(AcY) < 3000 && abs(AcX) < 3000) {
      CONTROL_MODE = ACTIVE;
    }

    if (CONTROL_MODE == RIGHTACTIVE && (now()-start)> 1) {
//      Serial.print("UP DOT PRODUCT IS ");
//      Serial.print(dotproduct(histGyZ, rightflickup, 6));
//      Serial.print("\n");
      if (dotproduct(histGyZ, rightflickup, 6) > 120){
        ContTv2 = !ContTv2;
        digitalWrite(12,ContTv2);
        start = now();
      } else if (dotproduct(histGyZ, rightflickdown, 6) > 120){ 
        ContTv1 = !ContTv1;
        digitalWrite(13,ContTv1);
        start = now();
      }
    } else if (CONTROL_MODE == LEFTACTIVE && (now()-start)> 1) {
      if (dotproduct(histGyZ, leftflickup, 6) > 120){
        ContTv2 = !ContTv2;
        digitalWrite(12,ContTv2);
        start = now();
      } else if (dotproduct(histGyZ, leftflickdown, 6) > 120){
        ContTv1 = !ContTv1;
        digitalWrite(13,ContTv1);
        start = now();
      } 
    } else if (CONTROL_MODE == BACKACTIVE && (now()-start)> 1) {
      if (dotproduct(histGyZ, backflickright, 6) > 120){
        ContTv2 = !ContTv2;
        digitalWrite(12,ContTv2);
        start = now();
      } else if (dotproduct(histGyZ, backflickleft, 6) > 120){
        ContTv1 = !ContTv1;
        digitalWrite(13,ContTv1);
        start = now();
      } 
    } else if (CONTROL_MODE == FORWARDACTIVE && (now()-start)> 1) {
      if (dotproduct(histGyZ, forwardflickright, 6) > 120){
        ContTv2 = !ContTv2;
        digitalWrite(12,ContTv2);
        start = now();
      } else if (dotproduct(histGyZ, forwardflickleft, 6) > 120){
        ContTv1 = !ContTv1;
        digitalWrite(13,ContTv1);
        start = now();
      } 
    } 


    if (shake > 80) {
      digitalWrite(13, false);
      digitalWrite(12, false);
      ContTv1 = false;
      ContTv2 = false;
      CONTROL_MODE = DEFAULT_MODE;
      shake = 0;
    }
  }
  
  
  if (leftTilt > 10) {
    CONTROL_MODE = LEFTACTIVE;
  }

//  //Serial.print("Checking conditions for light changing \n");
//  if (CONTROL_MODE == RIGHT && AcX > 7000 && (now()-start)> 1) {
//    //Serial.print("Turning the light on or off \n");
//    digitalWrite(13, !ContTv1);
//    ContTv1 = !ContTv1;
//    start = now();
//  }

//  if (CONTROL_MODE == RIGHT && AcX < -7000 && (now()-start)> 1) {
//    //Serial.print("Turning the light on or off \n");
//    digitalWrite(12, !ContTv2);
//    ContTv2 = !ContTv2;
//    start = now();
//  }
//  
  
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
  if (AcX < -12000 && AcY < 0 && AcZ < 1700) {
  digitalWrite(6,ContTv2);
  ContTv2 = !ContTv2;
  delay(1800);
  } */
}

// Having a function for dot product lets us compare vectors of the last n input movements to 
// examples of inputs that we have for certain motions
// Ex. Flicking the board up produces GyX = [-32000, 0, 32000], we want to find whenever a similar
// motion occurs during our data
// We take the dot product of our ideal motion [-32000, 0, 32000] and the last 3 measurements of GyX from
// the board. 
// Say the last 3 values of GyX were [-10000, -5000, 0] (not very similar to our ideal motion vector, the 
// dot product of this and our sample vector will be (-32000*-10000)+(0*-5000)+(32000*0) = 320 000 000
// Now try it with something more similar to our vector, say [-30000, 5000, 20000]. The dot product for this
// will be (-32000*-30000)+(0*5000)+(32000*20000) = 1 600 000 000. Note that this is much larger then the
// other example vector we tried. This is because the second test vector is more similar to our ideal motion
// vector we are looking for.
int32_t dotproduct (int16_t *input, int16_t *ideal, int length) {
  int32_t output = 0;
  for (int i = 0; i < length; i++) {
    // Here we divide by 1000 to avoid integer overflow
    output += (input[i]/1000 * ideal[i]);
  }
  return output;
}

