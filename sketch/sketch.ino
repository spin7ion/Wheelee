#include <Wire.h>
#include <L3G.h>
#include <LSM303.h>
#include <DualVNH5019MotorShield.h>
#include <SoftwareSerial.h>// import the serial library

SoftwareSerial btSerial(0, 1); // RX, TX

#define fullturnTicks 2240
#define lengthOfWheel 0.2826

#define ToPhiRad(x) ((x)*0.00140249672)//0.00140249672

#define ToRad(x) ((x)*0.01745329252)  // *pi/180
#define ToDeg(x) ((x)*57.2957795131)  // *180/pi

// L3G4200D gyro: 2000 dps full scale
// 70 mdps/digit; 1 dps = 0.07
#define Gyro_Gain_X 0.07 //X axis Gyro gain
#define Gyro_Gain_Y 0.07 //Y axis Gyro gain
#define Gyro_Gain_Z 0.07 //Z axis Gyro gain
#define Gyro_Scaled_X(x) ((x)*ToRad(Gyro_Gain_X)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Y(x) ((x)*ToRad(Gyro_Gain_Y)) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Z(x) ((x)*ToRad(Gyro_Gain_Z)) //Return the scaled ADC raw data of the gyro in radians for second

#define coordX 0
#define coordY 1
#define coordZ 2

L3G gyro;
LSM303 compass;
DualVNH5019MotorShield md;

float angle=1.585;
float balanceAt=1.585;//radians

int encoder0PinA = 3;
int encoder0PinB = 11;

long spd=400;

long accelerometerValues[3]={0,0,0};
long magnetometerValues[3]={0,0,0};
float gyroRate[3]={0,0,0};

volatile int encoder0Pos = 0;


long timer=0;   //general purpuse timer
long timer_old;

void setup() {
  pinMode(encoder0PinA, INPUT);
  digitalWrite(encoder0PinA, HIGH);
  pinMode(encoder0PinB, INPUT);
  digitalWrite(encoder0PinB, HIGH);
  Serial.begin(9600);
  Serial.println("Wheelee");
//  btSerial.begin(9600);
//  btSerial.println("Wheelee");
  I2C_Init();
  delay(1500);
  Accel_Init();
  Gyro_Init();
  md.init();
  attachInterrupt(1, doEncoder, CHANGE);  // encoder pin on interrupt 1 - pin 3
  timer=millis();
}


//float K1=0.05,K2=0.08,K3=6.37,K4=1.1;
float K1=0.1,K2=0.29,K3=6.5,K4=1.12;
//float K1=0.05,K2=0.18,K3=6.5,K4=1.12;
long getLQRSpeed(float phi,float dphi,float angle,float dangle){
  return constrain((phi*K1+dphi*K2+K3*angle+dangle*K4)*285,-400,400);
}
float lastPhi=0,dPhi=0;
float G_Dt=0.02;



float getPhiAdding(float dif){

  if(dif<200 && dif>-200){return 0.f;}
  float ret = dif*0.08;

  return ret;
}

float getFactorAdding(float dif){
  if(dif<200 && dif>-200){return 0.f;}
  float ret = dif/500*20;
  return ret;
}

char BluetoothData; // the data given from Computer

float phiDif=0.f;
float factorDif=0.f;

// the loop routine runs over and over again forever:
void loop() {
  if((millis()-timer)>=20)  // Main loop runs at 50Hz
  {
    if (Serial.available()){
    BluetoothData=Serial.read();
     if(BluetoothData=='w'){   
      phiDif=200;//constrain(phiDif+10,-200,200);
     } else if(BluetoothData=='s'){ 
      phiDif=-200;//constrain(phiDif-10,-200,200);      
     } else if(BluetoothData=='a'){   
      factorDif=200;//constrain(factorDif+10,-200,200);
     } else if(BluetoothData=='d'){   
      factorDif=-200;//constrain(factorDif-10,-200,200);      
     } else if(BluetoothData=='c'){   
      factorDif=0;//constrain(factorDif-10,-200,200);      
      phiDif=0;
     }
    }
    
   
    timer_old = timer;
    timer=millis();
    if (timer>timer_old){
      G_Dt = (timer-timer_old)/1000.0;    // Real time of loop run. We use this on the DCM algorithm (gyro integration time)
    }
    else{
      G_Dt = 0;
    }
    
    Read_Accel();
    Read_Gyro();
    float accAng=getAccAngle();
    
    angle=comp_filter(accAng,-gyroRate[coordY]);
    if(accAng>1 && accAng<2){
    dPhi=(ToPhiRad(encoder0Pos)-lastPhi)/G_Dt;
    
    encoder0Pos+=getPhiAdding(phiDif);
    
    lastPhi=ToPhiRad(encoder0Pos);
    
    spd=getLQRSpeed(ToPhiRad(encoder0Pos),dPhi,balanceAt-angle,gyroRate[coordY]);
     float factorL=getFactorAdding(factorDif);
    md.setSpeeds(spd-factorL,spd+factorL);
    }else{
      md.setBrakes(400,400);
    }
    if(millis()%50==0){
      Serial.println(angle);
    }
    /*btSerial.print('a');
    btSerial.print(accAng);  
    btSerial.print('g');
    btSerial.print(gyroRate[coordY]);  
    btSerial.print('r');
    btSerial.print(angle);
    btSerial.print('p');
    btSerial.print(dPhi);
    btSerial.print('r');
    btSerial.print(encoder0Pos);
    btSerial.println('e');*/
  }
}

void doEncoder() {
  if (digitalRead(encoder0PinA) == HIGH) {
    if (digitalRead(encoder0PinB) == LOW) {  // check channel B to see which way
                                             // encoder is turning
      encoder0Pos = encoder0Pos - 1;         // CCW
    } 
    else {
      encoder0Pos = encoder0Pos + 1;         // CW
    }
  }
  else                                        // found a high-to-low on channel A
  { 
    if (digitalRead(encoder0PinB) == LOW) {   // check channel B to see which way
                                              // encoder is turning  
      encoder0Pos = encoder0Pos + 1;          // CW
    } 
    else {
      encoder0Pos = encoder0Pos - 1;          // CCW
    }
  }
  if(encoder0Pos>fullturnTicks*3 || encoder0Pos<-fullturnTicks*3){
    encoder0Pos=0;
  }
}

void stopIfFault()
{
  if (md.getM1Fault())
  {
    Serial.println("M1 fault");
    while(1);
  }
  if (md.getM2Fault())
  {
    Serial.println("M2 fault");
    while(1);
  }
}
void I2C_Init()
{
  Wire.begin();
}

void Gyro_Init()
{
  gyro.init();
  gyro.writeReg(L3G_CTRL_REG1, 0x0F); // normal power mode, all axes enabled, 100 Hz
  gyro.writeReg(L3G_CTRL_REG4, 0x20); // 2000 dps full scale
}
void Accel_Init()
{
  compass.init();
  compass.enableDefault();
  switch (compass.getDeviceType())
  {
    case LSM303::device_D:
      compass.writeReg(LSM303::CTRL2, 0x18); // 8 g full scale: AFS = 011
      break;
    case LSM303::device_DLHC:
      compass.writeReg(LSM303::CTRL_REG4_A, 0x28); // 8 g full scale: FS = 10; high resolution output mode
      break;
    default: // DLM, DLH
      compass.writeReg(LSM303::CTRL_REG4_A, 0x30); // 8 g full scale: FS = 11
  }
}


void Read_Gyro()
{
  gyro.read();
 
  gyroRate[0]=-Gyro_Scaled_X(gyro.g.x);  
  gyroRate[1]=-Gyro_Scaled_Y(gyro.g.y);
  gyroRate[2]=-Gyro_Scaled_Z(gyro.g.z);
}



// Reads x,y and z accelerometer registers
void Read_Accel()
{
  compass.readAcc();
  
  accelerometerValues[0] = compass.a.x;
  accelerometerValues[1] = compass.a.y;
  accelerometerValues[2] = compass.a.z;
}

float getAccAngle() {
  return atan2(-(float)accelerometerValues[coordX], (float)accelerometerValues[coordZ])+3.14;    // in Quid: 1024/(2*PI))
}

void Read_Compass()
{
  compass.readMag();
  
  magnetometerValues[0] = compass.m.x;
  magnetometerValues[1] = compass.m.y;
  magnetometerValues[2] = compass.m.z;
}

/********************************************************************
* Complimentary Filter
********************************************************************/

float lastCompTime=0;
float filterAngle=1.50;
float dt=0.005;//0.002 //0.005

float comp_filter(float newAngle, float newRate) {

dt=(millis()-lastCompTime)/1000.0;
float filterTerm0;
float filterTerm1;
float filterTerm2;
float timeConstant;

timeConstant=0.5; // default 1.0

filterTerm0 = (newAngle - filterAngle) * timeConstant * timeConstant;
filterTerm2 += filterTerm0 * dt;
filterTerm1 = filterTerm2 + ((newAngle - filterAngle) * 2 * timeConstant) + newRate;
filterAngle = (filterTerm1 * dt) + filterAngle;
lastCompTime=millis();
return filterAngle; // This is actually the current angle, but is stored for the next iteration
}
