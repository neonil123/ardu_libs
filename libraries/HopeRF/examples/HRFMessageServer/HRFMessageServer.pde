// HRFMessageServer
//
// Example program shows how to received HRFMessages and reply to them
// See also HRFMessageClient
// This version runs on ArduinoMega, as it suports multiple HardwareSerial ports.
// With modification could run on a single Serial port (ie without USB monitoring)

#include <HRFMessage.h>

// Declare the HRFMessage to use Serial1 for IO with the HM-TR module
HRFMessage server(&Serial1);

void setup()
{
    Serial.begin(9600);  // Monitoring via USB
    Serial1.begin(9600); // The HM-TR transceiver defaults to 9600 unless you change it with the HopeRF setup program
}

void loop()
{
    uint8_t buf[HRF_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (server.recv((uint8_t*)&buf, &len))
    {
      // Got a message from the client.
      // Send a reply back
      server.send((uint8_t*)"reply\n", 7); // Includes the NUL at the end of the string
      Serial.print("got: ");
      Serial.print((const char*)buf);
    }
}
