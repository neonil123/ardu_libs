// HRFMessage.cpp
//
// Define low level unaddressed datagram
// Part of the Arduino HopeRF library for operating with HopeRF HM-TR transceivers 
// (see http://www.hoperf.com)
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HRFMessage.cpp,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#if (ARDUINO < 100)
#include "WProgram.h"
#endif
#include "HRFMessage.h"
#include <util/crc16.h>

// Compute CRC over count bytes.
// This should only be ever called at user level, not interrupt level
uint16_t hrf_crc(uint8_t *ptr, uint8_t count)
{
    uint16_t crc = 0xffff;

    while (count-- > 0) 
	crc = _crc_ccitt_update(crc, *ptr++);
    return crc;
}

/////////////////////////////////////////////////////////////////////
// Constructors
HRFMessage::HRFMessage(HardwareSerial* serial)
{
    _serial = serial;
    _rx_len = 0;
    _rx_bad = 0;
    _rx_good = 0;
}


/////////////////////////////////////////////////////////////////////
// Public methods
uint8_t HRFMessage::send(uint8_t* buf, uint8_t len)
{
    uint8_t i;
    uint16_t crc = 0xffff;
    uint8_t  count = len + 3; // Added byte count and FCS to get total number of bytes

    if (len > HRF_MAX_PAYLOAD)
	return false;

    // Encode the message length
    crc = _crc_ccitt_update(crc, count);
    _serial->write(count);

    for (i = 0; i < len; i++)
    {
	crc = _crc_ccitt_update(crc, buf[i]);
	_serial->write(buf[i]);
    }

    // Append the fcs, 16 bits before encoding (4 6-bit symbols after encoding)
    // Caution: VW expects the _ones_complement_ of the CCITT CRC-16 as the FCS
    // VW sends FCS as low byte then hi byte
    crc = ~crc;
    _serial->write(crc & 0xff);
    _serial->write(crc >> 8);

    return true;
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFMessage::available()
{
    // Get any pending chars (but do not overflow the rx buffer)
    // Need at least one to discover the expected mesage length
    while (_rx_len == 0 || _rx_len < _rx_buf[0])
    {
	if (_serial->available())
	{
	    uint8_t ch = _serial->read();
	    // The first byte is the byte count
	    // Check it for sensibility. It cant be less than 4, since it
	    // includes the bytes count itself and the 2 byte FCS
	    // Nor can it be greater than HRF_MAX_MESSAGE_LEN
	    // Caution: It is possible for a corrupted length within these limits 
	    // to cause messages to be lost
	    // until the the corrupted length number of octets have been received
	    if (_rx_len == 0 && (ch < 4 || ch > HRF_MAX_MESSAGE_LEN))
	    {
		_rx_bad++; // Corrupt length?
		// Throw everything pending away, else way may take a long time to recover:
		_serial->flush(); 
	    }
	    else
		_rx_buf[_rx_len++] = ch;
	    // Go around again and check for complete message
	}
	else
	    return 0; // No complete message available yet
    }

    return _rx_len;
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFMessage::recv(uint8_t* buf, uint8_t* len)
{
    if (!available())
	return false;

    if (*len > (_rx_len - 3))
	*len = _rx_len - 3;
    memcpy(buf, _rx_buf + 1, *len);
    uint8_t result = (hrf_crc(_rx_buf, _rx_len) == 0xf0b8); // FCS OK?
    _rx_len = 0; // done with this message
    return result;
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFMessage::rx_bad(void)
{
    return _rx_bad;
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFMessage::rx_good(void)
{
    return _rx_good;
}

/////////////////////////////////////////////////////////////////////
void HRFMessage::waitAvailable(void)
{
#ifdef ARDUINO
    // On Arduino we poll
    while (!_serial->available())
	;
#else
    // On Linux, we use a special method in our own version of HardwareSerial
    // which blocks until a character is avilable
    _serial->waitAvailable();
#endif
}
