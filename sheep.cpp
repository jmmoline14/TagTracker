#include <SoftwareSerial.h>

#define XBEE_RX 2
#define XBEE_TX 3
SoftwareSerial xbee(XBEE_RX, XBEE_TX);

void setup() {
  Serial.begin(9600);
  xbee.begin(9600);

  // Enter command mode
  delay(1000);
  xbee.print("+++");
  delay(1000); // Wait for "OK"

  xbee.println("ATID1234");         // Set PAN ID
  delay(200);
  xbee.println("ATDH0013A200");     // High 32 bits of receiver address
  delay(200);
  xbee.println("ATDL420108FE");     // Low 32 bits of receiver address
  delay(200);
  xbee.println("ATWR");             // Save to flash
  delay(200);
  xbee.println("ATCN");             // Exit command mode
  delay(200);
}

void loop() {
  int sensorValue = analogRead(A0);
  xbee.print("Sensor Value: ");
  xbee.println(sensorValue);
  delay(1000);
}