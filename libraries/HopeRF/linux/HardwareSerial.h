// HardwareSerial.h
//
// This is an abstract class for Linux that provides a
// similar interface to Arduino HardwareSerial, with special subclasses for testing 
// and access to a real linux serial port
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HRFMessage.h,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#ifndef HardwareSerial_h
#define HardwareSerial_h

#include <stdio.h>
#include <stdlib.h>
#include <wiring.h>

#define HARDWARE_SERIAL_RX_BUFFER_SIZE 256
class HardwareSerial
{
private:
    // Ring buffer
    uint8_t _rx_buf[HARDWARE_SERIAL_RX_BUFFER_SIZE];
    uint8_t _rx_buf_head;
    uint8_t _rx_buf_tail;

protected:
    void store_char(uint8_t ch);

public:
    HardwareSerial(void);
    virtual void begin(long);
    virtual uint8_t available(void);
    virtual int read(void);
    virtual void flush(void);
    virtual void write(uint8_t);
    virtual void waitAvailable(void);
};

// For testing only: allows 2 cross connected test devices
class MockHardwareSerial : public HardwareSerial
{
private:
    MockHardwareSerial* _dest;

public:
    MockHardwareSerial(void);
    void setDest(MockHardwareSerial* dest);
    void write(uint8_t ch);
};


// Provides access to a real Linux serial port
class LinuxHardwareSerial : public HardwareSerial
{
private:
    int _port;
    int baudToFlag(long baud);

public:
    LinuxHardwareSerial(const char* device);
    void begin(long baud);
    void waitAvailable(void);
    void write(uint8_t ch);
};

extern LinuxHardwareSerial Serial;
#endif
