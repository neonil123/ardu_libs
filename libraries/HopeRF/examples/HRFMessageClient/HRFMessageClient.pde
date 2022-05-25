// HRFMessageClient
//
// example program shows how to use HRFMessage to send messages
// to a HRFMessage server.
// HRFMessage is unaddressed: all HRFMessage nodes within range will 
// receive messages
// Sends a HRFMessage once a second, and hopes to get a reply from the server. Prints it if it does
// This version runs on ArduinoMega, as it suports multiple HardwareSerial ports.
// With modification could run on a single Serial port (ie without USB monitoring)

#include <HRFMessage.h>

// Declare the HRFMessage to use Serial1 for IO with the HM-TR module
HRFMessage client(&Serial1);
long      lastSendTime = 0;

void setup()
{
    Serial.begin(9600);  // Monitoring via USB
    Serial1.begin(9600); // The HM-TR transceiver defaults to 9600 unless you change it with the HopeRF setup program
}

void loop()
{
  // Send a message to the server at most once a second
  long thisTime = millis();
  if (thisTime > lastSendTime + 1000)
  {
    client.send((uint8_t*)"test\n", 6); // Includes the NUL at the end of the string
    Serial.print("sending\n");
    lastSendTime = thisTime;
  }
  
  // But always look for a reply
  uint8_t buf[HRF_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if (client.recv((uint8_t*)&buf, &len))
  {
      // Got a reply from the server
    Serial.print("got: ");
    Serial.print((const char*)buf);
  }
}
