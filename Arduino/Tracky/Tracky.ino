#include <SoftwareSerial.h>
#include "CurieIMU.h"

SoftwareSerial GPS(2, 3); // Rx, Tx
//SoftwareSerial GSMmodule(4, 5); // Rx, Tx

int Gpsdata;             // for incoming serial data
unsigned int finish = 0; // indicate end of message
unsigned int pos_cnt = 0; // position counter
unsigned int lat_cnt = 0; // latitude data counter
unsigned int log_cnt = 0; // longitude data counter
unsigned int flg = 0;    // GPS flag
unsigned int com_cnt = 0; // comma counter
char lt[20];            // latitude array
char lg[20];             // longitude array

int avgX = 0;
int avgY = 0;
int avgZ = 0;
//int count = 0;

boolean alert = 0;
boolean theft = 0;

// Functions
void setReference();
void GPSData();
//void ring();
/*******************************************************************************************
Function Name: Setup
*******************************************************************************************/
void setup() {
  Serial1.begin(9600); // initialize serial communication for BT
  GPS.begin(9600); // initialize serial communication for GPS
  //GSMmodule.begin(9600); // initialize serial communication for GSM
  
  // initialize device
  Serial1.println("Initializing IMU device...");
  CurieIMU.begin();

  // Set the accelerometer range to 3G
  CurieIMU.setAccelerometerRange(3);
  delay(1000);
}
/*******************************************************************************************
Function Name: Loop
*******************************************************************************************/
void loop() {
  //ring();
  if (Serial1.available() > 0) {
    char ch = Serial1.read();
    if (ch == '1' && alert == 0) {
      // Calculate reference
      setReference();
      alert = 1;
    } else if (ch == '0') {
      alert = 0;
      theft = 0;
      Serial1.println("Reseting. . .");
    } else if (ch == '2') {
      Serial1.println("GPS");
      GPSData();
    }
  }

  if (alert == 1) {
    int x = abs(abs(CurieIMU.readAccelerometer(X_AXIS)) - avgX);
    int y = abs(abs(CurieIMU.readAccelerometer(Y_AXIS)) - avgY);
    int z = abs(abs(CurieIMU.readAccelerometer(Z_AXIS)) - avgZ);
    int s = (x + y + z) / 3;
    Serial1.print("Curret position: ");
    Serial1.print("\t");
    Serial1.println(s);
    if (s > 1000) {
      theft = 1;
      Serial1.println("ALERT");
      Serial1.println();     
    }
  }

  //long long lastMillis = 0;
  while (theft) {
    digitalWrite(13, HIGH);
    if (Serial1.available() > 0 && Serial1.read() == '0') {
      alert = 0;
      theft = 0;
      Serial1.println("Reseting. . .");
    }
  }
  digitalWrite(13, LOW);
  pos_cnt = 0; finish = 0;
  delay(50);
}
/*******************************************************************************************
Function Name: setReference
*******************************************************************************************/
void setReference() {
  long sumX = 0;
  long sumY = 0;
  long sumZ = 0;
  // Calculate reference position
  for (int i = 0; i < 1000; i++) {
    int x = abs(CurieIMU.readAccelerometer(X_AXIS));
    int y = abs(CurieIMU.readAccelerometer(Y_AXIS));
    int z = abs(CurieIMU.readAccelerometer(Z_AXIS));
    sumX += x;
    sumY += y;
    sumZ += z;
  }
  avgX = sumX / 1000;
  avgY = sumY / 1000;
  avgZ = sumZ / 1000;
  Serial1.print("Reference position: ");
  Serial1.print("\t");
  Serial1.print(avgX);
  Serial1.print("\t");
  Serial1.print(avgY);
  Serial1.print("\t");
  Serial1.println(avgZ);
  Serial1.println();
}
/*******************************************************************************************
Function Name: GPSData
*******************************************************************************************/
void GPSData() {
  while (!finish) {
  Serial1.println(GPS.available());
    while (GPS.available() > 0) {       // Check GPS data
      Gpsdata = GPS.read();
      //Serial1.write(Gpsdata);
      //Serial1.println("looping");
      flg = 1;
      if ( Gpsdata == '$' && pos_cnt == 0) // finding GPRMC header
        pos_cnt = 1;
      if ( Gpsdata == 'G' && pos_cnt == 1)
        pos_cnt = 2;
      if ( Gpsdata == 'P' && pos_cnt == 2)
        pos_cnt = 3;
      if ( Gpsdata == 'R' && pos_cnt == 3)
        pos_cnt = 4;
      if ( Gpsdata == 'M' && pos_cnt == 4)
        pos_cnt = 5;
      if ( Gpsdata == 'C' && pos_cnt == 5 )
        pos_cnt = 6;
      if (pos_cnt == 6 &&  Gpsdata == ',') { // count commas in message
        com_cnt++;
        flg = 0;
      } if (com_cnt == 3 && flg == 1) {
        lt[lat_cnt++] =  Gpsdata;         // latitude
        flg = 0;
      } if (com_cnt == 5 && flg == 1) {
        lg[log_cnt++] =  Gpsdata;         // Longitude
        flg = 0;
      } if ( Gpsdata == '*' && com_cnt >= 5) {
        com_cnt = 0;                      // end of GPRMC message
        lat_cnt = 0;
        log_cnt = 0;
        flg     = 0;
        finish  = 1;
      }
    }
  }
}

/*
void ring()
{
  tone(8, 2000);
  delay(100);
  noTone(8);
  delay(100);
  tone(8, 1000);
  delay(100);
  noTone(8);
  delay(100);
}*/

