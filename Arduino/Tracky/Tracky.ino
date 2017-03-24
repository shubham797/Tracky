#include "CurieIMU.h"

int avgX = 0;
int avgY = 0;
int avgZ = 0;
//int count = 0;

boolean alert = 0;
boolean theft = 0;
// Functions
void setReference();
/*************************************************************
  Setup Function
*************************************************************/
void setup() {
  Serial.begin(9600); // initialize Serial communication
  Serial1.begin(9600); // initialize serial communication for BT
  
  // initialize device
  Serial1.println("Initializing IMU device...");
  CurieIMU.begin();

  // Set the accelerometer range to 3G
  CurieIMU.setAccelerometerRange(3);
  delay(1000);
}
/*************************************************************
  Loop Function
*************************************************************/
void loop() {
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

  while(theft) {
    digitalWrite(13, HIGH);
    if (Serial1.available() > 0 && Serial1.read() == '0') {
      alert = 0;
      theft = 0;
      Serial1.println("Reseting. . .");
    }
  }
  digitalWrite(13, LOW);
  delay(50);
}
/*************************************************************
  setReference Function
*************************************************************/
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

