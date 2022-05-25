// Linux version of HardwareSerial, with test class and 
// access to a real Linux serial port
// Can be used with HopeRF on Linux
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HardwareSerial.cpp,v 1.2 2009/08/17 06:35:13 mikem Exp mikem $

#include "HardwareSerial.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

// Default device
LinuxHardwareSerial Serial("/dev/ttyUSB0");

///////////////////////////////////////////////////////////////
HardwareSerial::HardwareSerial(void)
{
    _rx_buf_head = 0;
    _rx_buf_tail = 0;
}

void HardwareSerial::begin(long)
{
}

uint8_t HardwareSerial::available(void)
{
    return (HARDWARE_SERIAL_RX_BUFFER_SIZE + _rx_buf_head - _rx_buf_tail) % HARDWARE_SERIAL_RX_BUFFER_SIZE;
}

void HardwareSerial::store_char(uint8_t ch)
{
    uint8_t i = (_rx_buf_head + 1) % HARDWARE_SERIAL_RX_BUFFER_SIZE;
    if (i != _rx_buf_tail) 
    {
	_rx_buf[_rx_buf_head] = ch;
	_rx_buf_head = i;
    }
}

int HardwareSerial::read(void)
{
    // if the head isn't ahead of the tail, we don't have any characters
    if (_rx_buf_head == _rx_buf_tail) 
	return -1;
    else 
    {
	uint8_t ch = _rx_buf[_rx_buf_tail];
	_rx_buf_tail = (_rx_buf_tail + 1) % HARDWARE_SERIAL_RX_BUFFER_SIZE;
	return ch;
    }
}

void HardwareSerial::flush(void)
{
    _rx_buf_head = _rx_buf_tail;
}

void HardwareSerial::write(uint8_t)
{
}

void HardwareSerial::waitAvailable(void)
{
}

///////////////////////////////////////////////////////////////
MockHardwareSerial::MockHardwareSerial(void)
{
    _dest = NULL;
}

void MockHardwareSerial::setDest(MockHardwareSerial* dest)
{
    _dest = dest;
}

void MockHardwareSerial::write(uint8_t ch)
{
    if (_dest)
	_dest->store_char(ch);
}

///////////////////////////////////////////////////////////////
LinuxHardwareSerial::LinuxHardwareSerial(const char* device)
{
    int port = open(device, O_RDWR);
    if (port < 0)
    {
	perror("failed to open device");
	return;
    }
    _port = port;
}

// Given a baud rate, convert it to the flag required by tcsetattr
int LinuxHardwareSerial::baudToFlag(long baud)
{
    int flag;

    // This is very ugly
    switch (baud)
    {
	// Do POSIX-specified rates first.
	case 0: flag = B0; break;
	case 50: flag = B50; break;
	case 75: flag = B75; break;
	case 110: flag = B110; break;
	case 134: flag = B134; break;
	case 150: flag = B150; break;
	case 200: flag = B200; break;
	case 300: flag = B300; break;
	case 600: flag = B600; break;
	case 1200: flag = B1200; break;
	case 1800: flag = B1800; break;
	case 2400: flag = B2400; break;
	case 4800: flag = B4800; break;
	case 9600: flag = B9600; break;
	case 19200: flag = B19200; break;
	case 38400: flag = B38400; break;
	    // And now the extended ones conditionally.
# ifdef B7200
	case 7200: flag = B7200; break;
# endif
# ifdef B14400
	case 14400: flag = B14400; break;
# endif
# ifdef B57600
	case 57600: flag = B57600; break;
# endif
# ifdef B115200
	case 115200: flag = B115200; break;
# endif
# ifdef B230400
	case 230400: flag = B230400; break;
# endif
# ifdef B460800
	case 460800: flag = B460800; break;
# endif
# ifdef B500000
	case 500000: flag = B500000; break;
# endif
# ifdef B576000
	case 576000: flag = B576000; break;
# endif
# ifdef B921600
	case 921600: flag = B921600; break;
# endif
# ifdef B1000000
	case 1000000: flag = B1000000; break;
# endif
# ifdef B1152000
	case 1152000: flag = B1152000; break;
# endif
# ifdef B2000000
	case 2000000: flag = B2000000; break;
# endif
# ifdef B3000000
	case 3000000: flag = B3000000; break;
# endif
# ifdef B3500000
	case 3500000: flag = B3500000; break;
# endif
# ifdef B4000000
	case 4000000: flag = B4000000; break;
# endif
	default:
	    flag = B0;
    }
  return flag;
}

void LinuxHardwareSerial::begin(long baud)
{
    // Set blocking IO
    int flags = 0;
    int curmode;
    flags = fcntl(_port, F_GETFL, 0); // Read current settings
    flags &= ~O_NDELAY;
    fcntl(_port, F_SETFL, flags);

    struct termios t;
    tcgetattr(_port, &t);
    // Only block for a second at a time
    t.c_cc[VTIME] = 10;
    // Set 9600 baud
    t.c_cflag &= ~CBAUD;
    t.c_cflag |= baudToFlag(baud);
    // No parity
    t.c_cflag &= ~PARENB;
    // 1 stop bit
    t.c_cflag &= ~CSTOPB;
    tcsetattr(_port, TCSANOW, &t);
}


///////////////////////////////////////////////////////////////
// Block until something is available to read
void LinuxHardwareSerial::waitAvailable(void)
{
    while (!available())
    {
	// Nothing in the ring buffer, have to get something from the port, which is blocking
	uint8_t ch;
	ssize_t result = ::read(_port, &ch, 1);
	if (result == 1)
	    store_char(ch);
	else if (result < 0)
	    perror("read failed");
    }
}

void LinuxHardwareSerial::write(uint8_t ch)
{
    if (::write(_port, &ch, 1) < 0)
	perror("write failed");
}

