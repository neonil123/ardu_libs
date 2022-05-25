// HRFDatagram.cpp
//
// Define addressed datagram
// 
// Part of the Arduino HopeRF library for operating with HopeRF HM-TR transceivers 
// (see http://www.hoperf.com)
// HRFDatagram will be received only by the addressed node or all nodes within range if the 
// to address is HRF_BROADCAST_ADDRESS
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HRFDatagram.cpp,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#if (ARDUINO < 100)
#include "WProgram.h"
#endif
#include "HRFDatagram.h"

////////////////////////////////////////////////////////////////////
// Constructors
HRFDatagram::HRFDatagram(HardwareSerial* serial, uint8_t thisAddress) 
  : HRFMessage(serial)
{
    setThisAddress(thisAddress);
}

////////////////////////////////////////////////////////////////////
// Public methods
void HRFDatagram::setThisAddress(uint8_t thisAddress)
{
    _thisAddress = thisAddress;
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFDatagram::sendto(uint8_t address, uint8_t* buf, uint8_t len)
{
    uint8_t tx_buf[HRF_MAX_MESSAGE_LEN];

    if (len > (HRF_MAX_PAYLOAD-2))
	return false;

    tx_buf[0] = address;
    tx_buf[1] = _thisAddress;
    memcpy(tx_buf+2, buf, len);
    return send(tx_buf, len+2);
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFDatagram::recvfrom(uint8_t* buf, uint8_t* len, uint8_t* from, uint8_t* to, uint8_t promiscuous)
{
    uint8_t rx_buf[HRF_MAX_MESSAGE_LEN];
    uint8_t rx_buf_len = sizeof(rx_buf);

    while (HRFMessage::recv(rx_buf, &rx_buf_len))
    {
	if (promiscuous || rx_buf[0] == _thisAddress || rx_buf[0] == HRF_BROADCAST_ADDRESS)
	{
	    // Its for this node
	    if (*len > (rx_buf_len - 2))
		*len = rx_buf_len - 2;
	    memcpy(buf, rx_buf+2, *len);
	    if (from)
		*from = rx_buf[1];
	    if (to)
		*to = rx_buf[0];
	    return true;
	}
	rx_buf_len = sizeof(rx_buf);
    }

    // Nothing found
    return false;
}

/////////////////////////////////////////////////////////////////////
uint8_t HRFDatagram::recv(uint8_t* buf, uint8_t* len)
{
    return recvfrom(buf, len);
}
