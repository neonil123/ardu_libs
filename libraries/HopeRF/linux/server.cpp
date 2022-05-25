// server.cpp
//
// Linux program to implement a HRFreliableDatagram server using the
// HopeRF library.
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: server.cpp,v 1.2 2009/08/17 06:35:13 mikem Exp mikem $

#include "HardwareSerial.h"
#include "HRFReliableDatagram.h"
#include <stdio.h>

HRFReliableDatagram server(&Serial, 11);

// this has been structured a bit like an Arduino sketch, but it
// doesnt have to be that way
void setup()
{
    Serial.begin(9600);
    server.setTimeout(500);
}

void loop()
{
    uint8_t from;
    uint8_t buf[HRF_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    server.waitAvailable();
    if (server.recvfromAck((uint8_t*)&buf, &len, &from))
    {
	printf("Got from %d: %s\n", from, buf);
	if (!server.sendtoWait(from, (uint8_t*)"reply\n", 7))
	    printf("sendtoWait failed\n");
    }
}

int main(int argc, char**argv)
{
    setup();

    // This is a bit ugly: in a real Linux program, you would not hard loop
    // like this
    while (1)
	loop();
}
