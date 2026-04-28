#include <SoftwareSerial.h>
#define XBEE_RX 2
#define XBEE_TX 3
SoftwareSerial xbee(XBEE_RX, XBEE_TX);
//Send APIFRAME helper function, sends the data that is placed in the function place holer
void sendAPIFrame(String data) {
  uint8_t payloadLen = data.length(); // payload is the size of the data string
  uint8_t frameLen = payloadLen + 14; // necessary padding for the packet
  // These are the first 5 bytes of every xbee API frame (constant)
  xbee.write((uint8_t)0x7E);
  xbee.write((uint8_t)0x00);
  xbee.write((uint8_t)frameLen);
  xbee.write((uint8_t)0x10);
  xbee.write((uint8_t)0x01);
  // Mac Adress of wolf.py
  xbee.write((uint8_t)0x00); xbee.write((uint8_t)0x13); xbee.write((uint8_t)0xA2); xbee.write((uint8_t)0x00);
  xbee.write((uint8_t)0x42); xbee.write((uint8_t)0x01); xbee.write((uint8_t)0x09); xbee.write((uint8_t)0x05);
  // Last four bytes of the header before the payload.
  xbee.write((uint8_t)0xFF);
  xbee.write((uint8_t)0xFE);
  xbee.write((uint8_t)0x00);
  xbee.write((uint8_t)0x00);
  // write the payload
  for (int i = 0; i < payloadLen; i++) {
    xbee.write((uint8_t)data[i]);
  }
  // end packing for the packet. The Xbee system verifies packets because they all must be 0xFF
  uint8_t checksum = 0x10 + 0x01 + 0x00 + 0x13 + 0xA2 + 0x00 + 0x42 + 0x01 + 0x09 + 0x05 + 0xFF + 0xFE + 0x00 + 0x00;
  for (int i = 0; i < payloadLen; i++) {
    checksum += (uint8_t)data[i];
  }
  xbee.write((uint8_t)(0xFF - checksum));
}
// Helper function that will allow the sheep to declare which state it is in.
void requestRSSI() {
  uint8_t frame[] = {
    0x7E, 0x00, 0x04,
    0x08, 0x01,
    0x44, 0x42,
    0x70
  };
  for (int i = 0; i < 8; i++) xbee.write(frame[i]);
}
// This actually reads the RSSI value and prints it
uint8_t readRSSI() {
  delay(500);
  uint8_t buf[32];
  int bufLen = 0;
  while (xbee.available() && bufLen < 32) {
    buf[bufLen++] = xbee.read();
  }

  Serial.print("RSSI raw response: ");
  for (int i = 0; i < bufLen; i++) {
    if (buf[i] < 0x10) Serial.print("0");
    Serial.print(buf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  for (int i = 0; i < bufLen - 9; i++) {
    if (buf[i] == 0x7E && buf[i+3] == 0x88) {
      return buf[i+8];
    }
  }
  return 0;
}
// THis function will return the alert level given the RSSI
String getAlertLevel(uint8_t rssi) {
  if (rssi < 50)       return "CLOSE";    // -50 dBm or stronger
  else if (rssi < 70)  return "MEDIUM";   // -50 to -70 dBm
  else                 return "FAR";      // weaker than -70 dBm
}
// set up the xbee on a 9600 baud rate
void setup() {
  Serial.begin(9600);
  xbee.begin(9600);
  while (xbee.available()) xbee.read(); // flush boot garbage
  Serial.println("Transmitter Ready");
}

void loop() {
  // --- Packet 1 ---
  // This packet will just send it's indentifying info to the wolf
  // The wolf will send an ack bit back that can be used to calculate RSSI
  // The wolf will use this packet to determine the alertness of each of the sheep.
  sendAPIFrame("ping");
  Serial.println("Sent: Sheep1");
  delay(200);
  requestRSSI();
  uint8_t rssi = readRSSI();
  Serial.print("RSSI: -"); Serial.print(rssi); Serial.println(" dBm");
  // sends the alert level to the host.
  String alert = getAlertLevel(rssi);
  sendAPIFrame("Sheep1: " + alert);
  Serial.println("Sent alert: " + alert);

  // Add the state handling here, alert level saved in string ALERT
  // CLOSE, MEDIUM, and FAR are 3 options

  delay(800);
}