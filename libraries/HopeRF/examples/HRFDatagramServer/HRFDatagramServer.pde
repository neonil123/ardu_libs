// HRFDatagramServer
//
// Example program shows how to received HRFDatagrams and reply to them
// See also HRFDatagramClient
// This version runs on ArduinoMega, as it supports multiple HardwareSerial ports.
// With modification could run on a single Serial port (ie without USB monitoring)

#include <HRFDatagram.h>

// Declare the HRFMessage to use Serial1 for IO with the HM-TR module
// The address of this node is 11.
HRFDatagram server(&Serial1, 11);

void setup()
{
    Serial.begin(9600);  // Monitoring via USB
    Serial1.begin(9600); // The HM-TR transceiver defaults to 9600 unless you change it with the HopeRF setup program
}

void loop()
{
    uint8_t from;
    uint8_t buf[HRF_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (server.recvfrom((uint8_t*)&buf, &len, &from))
    {
      // Got a message from the client.
      // Send a reply back to whomever sent it
      server.sendto(from, (uint8_t*)"reply\n", 7); // Includes the NUL at the end of the string
      Serial.print("got from node: ");
      Serial.print(from, DEC);
      Serial.print(": ");
      Serial.print((const char*)buf);
    }
}
