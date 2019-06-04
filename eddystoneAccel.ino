
/* 
Copyright 2017 Roguish, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
 
#include <CurieBLE.h>
#include "CurieIMU.h"

// Eddystone Protocol Specification
// https://github.com/google/eddystone/blob/master/protocol-specification.md

// Eddystone uses UUID FEAA
BLEService service = BLEService("FEAA");

// UUID FEAA also used for Characteristic
// Only BLEBroadcast is required
// Set the third param to the length of your ServiceData string (or longer). 
// If it's not long enough the Service Data will be truncated
BLECharacteristic characteristic( "FEAA", BLEBroadcast, 50 );

// 4 Frame Types EID/URL/TLM/UID
const uint8_t FRAME_TYPE_EDDYSTONE_UID = 0x00;
const uint8_t FRAME_TYPE_EDDYSTONE_URL = 0x10;
const uint8_t FRAME_TYPE_EDDYSTONE_TLM = 0x20;
const uint8_t FRAME_TYPE_EDDYSTONE_EID = 0x40;


// Transmission Power
// Beacon Transmission Power is calibrated by measuring the rssi value at 1 meter from the beacon
// and adding 41
// https://altbeacon.github.io/android-beacon-library/eddystone-support.html
const int8_t TX_POWER_DBM = -29; // (-70 + 41); 

float ax, ay, az;   //scaled accelerometer values

void setup() {
  // enable if you want to log values to Serial Monitor
  Serial.begin(9600);

  // begin initialization
  BLE.begin();

  // No not set local name 
  // BLE.setLocalName("DO_NOT_SET");

  // set service
  BLE.setAdvertisedService(service);
  
  // add the characteristic to the service
  service.addCharacteristic( characteristic );

  // add service
  BLE.addService( service );

  // call broadcast otherwise Service Data is not included in advertisement value
  characteristic.broadcast();

  // create the variables to be send
  byte * x = (byte *) &ax;
  byte * y = (byte *) &ay;
  byte * z = (byte *) &az;
  uint8_t advdatacopy[] =
  {
    FRAME_TYPE_EDDYSTONE_TLM,
    (uint8_t) TX_POWER_DBM,
    x[3],x[2],x[1],x[0],
    y[3],y[2],y[1],y[0],
    z[3],z[2],z[1],z[0],
    
  };
  characteristic.writeValue( advdatacopy, sizeof(advdatacopy) );

  // start advertising
  BLE.advertise();

  //Setup the accelerometer and calibrate
  CurieIMU.begin();
  delay(500);
  CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
  CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
  CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 1);
  // Set the accelerometer range to 2G
  CurieIMU.setAccelerometerRange(2);
}

void loop() {
  // read accelerometer measurements from device, scaled to the configured range
  CurieIMU.readAccelerometerScaled(ax, ay, az);

  // Send reading in case of larger movement detected
  if(abs(ax) > 0.5 || abs(ay) > 0.5 ){
    sendReading();
  }
  
}

void sendReading(){
  //Print in serial monitor the readed values
  Serial.print("a:\t");
  Serial.print(ax);
  Serial.print("\t");
  Serial.print(ay);
  Serial.print("\t");
  Serial.print(az);
  Serial.println();
  
  serialFloatPrint(ax);
  serialFloatPrint(ay);
  serialFloatPrint(az);
  //Set values
  byte * x = (byte *) &ax;
  byte * y = (byte *) &ay;
  byte * z = (byte *) &az;
  uint8_t advdatacopy[] =
  {
    FRAME_TYPE_EDDYSTONE_TLM,
    (uint8_t) TX_POWER_DBM,
    x[0],x[1],x[2],x[3],
    y[0],y[1],y[2],y[3],
    z[0],z[1],z[2],z[3],
    
  };
    
  //Advertise packet
  characteristic.writeValue( advdatacopy, sizeof(advdatacopy) );
  delay(1000);
}
void serialFloatPrint(float f) {
  byte * b = (byte *) &f;
  /* DEBUG */
  Serial.println();
  Serial.print(b[0],HEX);
  Serial.print(b[1], HEX);
  Serial.print(b[2], HEX);
  Serial.println(b[3], HEX);
  //*/
}
