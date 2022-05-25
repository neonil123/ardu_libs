// Prevent some headers being loaded
#define _UTIL_CRC16_H_
#include <wiring.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

extern "C"
{
//    extern void exit(int status);
    extern uint16_t _crc_ccitt_update (uint16_t crc, uint8_t data);
}
#define F_CPU 16000000


time_t start_time = 0;

unsigned long millis()
{
    if (start_time == 0)
	start_time = time(NULL);
    return (time(NULL) - start_time) * 1000;
}

// CRC-CCITT calculation from http://snowcat.de/sd2iec/crcgen-new.c
#define lo8(x) (x & 0xFF)
#define hi8(x) ((x >> 8) & 0xFF)
uint16_t _crc_ccitt_update (uint16_t crc, uint8_t data)
{
//    printf("CCITT %04x, %02x\n", crc, data);

    data ^= lo8(crc);
    data ^= data << 4;
    return ((((uint16_t)data << 8) | hi8(crc)) ^ (uint8_t)(data >> 4) ^ ((uint16_t)data << 3));
}	

#include "../HRFMessage.cpp"
#include "../HRFDatagram.cpp"
#include "../HRFReliableDatagram.cpp"


