// HRFReliableDatagramClient
//
// example program shows how to use HRFReliableDatagram to send messages
// addressed to a HRFDatagram server.
// HRFReliableDatagram is addressed: only the addressed HRFDatagram nodes(s) within range will 
// receive messages and acknowledge
// Sends a HRFReliableDatagram once a second to a nominated address, and hopes to get a reply from the server. Prints it if it does
// This version runs on ArduinoMega, as it supports multiple HardwareSerial ports.
// With modification could run on a single Serial port (ie without USB monitoring)

#include <HRFReliableDatagram.h>

// Declare the HRFDatagram to use Serial1 for IO with the HM-TR module
// The address of this node is 10.
HRFReliableDatagram client(&Serial1, 10);
long      lastSendTime = 0;

void setup()
{
    Serial.begin(9600);  // Monitoring via USB
    Serial1.begin(9600); // The HM-TR transceiver defaults to 9600 unless you change it with the HopeRF setup program
    client.setTimeout(500); 
}

void loop()
{
  // Send a message to the server at most once a second
  long thisTime = millis();
  if (thisTime > lastSendTime + 1000)
  {
    // Send a message to node 11 (the server)
    Serial.print("sending\n");
    if (!client.sendtoWait(11, (uint8_t*)"test\n", 6)) // Includes the NUL at the end of the string
      Serial.print("sendtoWait failed\n");

    lastSendTime = thisTime;
  }
  
  // But always look for a reply
  uint8_t from;
  uint8_t buf[HRF_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if (client.recvfromAck((uint8_t*)&buf, &len, &from))
  {
     // Got a reply from the server
    Serial.print("got from node: ");
    Serial.print(from, DEC);
    Serial.print(": ");
    Serial.print((const char*)buf);
  }
}
