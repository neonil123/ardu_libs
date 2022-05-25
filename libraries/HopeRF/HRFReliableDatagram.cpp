// HRFReliableDatagram.cpp
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
// $Id: HRFReliableDatagram.cpp,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#if (ARDUINO < 100)
#include "WProgram.h"
#endif
#include "HRFReliableDatagram.h"

////////////////////////////////////////////////////////////////////
// Constructors
HRFReliableDatagram::HRFReliableDatagram(HardwareSerial* serial, uint8_t thisAddress) 
    : HRFDatagram(serial, thisAddress)
{
    _lastSequenceNumber = 0;
    _timeout = 1000;
    _retries = 3;
}

////////////////////////////////////////////////////////////////////
// Public methods
void HRFReliableDatagram::setTimeout(long timeout)
{
    _timeout = timeout;
}

////////////////////////////////////////////////////////////////////
void HRFReliableDatagram::setRetries(uint8_t retries)
{
    _retries = retries;
}

////////////////////////////////////////////////////////////////////
uint8_t HRFReliableDatagram::sendtoWait(uint8_t address, uint8_t* buf, uint8_t len)
{
    uint8_t tx_buf[HRF_MAX_MESSAGE_LEN];

    if (len > HRF_MAX_RELIABLE_DATAGRAM_PAYLOAD)
	return false;

    // Assemble the message
    _lastSequenceNumber = (_lastSequenceNumber + 1) & HRF_SQN_MASK; 
    uint8_t thisSequenceNumber = _lastSequenceNumber;
    tx_buf[0] = thisSequenceNumber;
    memcpy(tx_buf+1, buf, len);

    uint8_t retries = 0;
    while (retries++ <= _retries)
    {
	long thisSendTime = millis();
	if (!sendto(address, tx_buf, len+1))
	    return false; // Transmit failed

	// Never wait for ACKS to broadcasts:
	if (address == HRF_BROADCAST_ADDRESS)
	    return true;

	while (millis() < thisSendTime + _timeout)
	{
	    uint8_t rx_buf[HRF_MAX_MESSAGE_LEN];
	    uint8_t rx_buf_len = sizeof(rx_buf);
	    uint8_t from;
	    uint8_t to;
	    if (recvfrom(rx_buf, &rx_buf_len, &from, &to))
	    {
		// Now have a message: is it our ACK?
		if (   from == address 
		    && to == _thisAddress 
		    && (rx_buf[0] & HRF_FLAG_ACK) 
		    && (rx_buf[0] & HRF_SQN_MASK) == thisSequenceNumber)
		{
		    // Its the ACK we are waiting for
		    return true;
		}
	    }
	    // Not the one we are waiting for, maybe keep waiting
	}
	// Timeout exhausted, maybe retry
    }
    return false;
}

////////////////////////////////////////////////////////////////////
uint8_t HRFReliableDatagram::recvfromAck(uint8_t* buf, uint8_t* len, uint8_t* from, uint8_t* to)
{
    uint8_t rx_buf[HRF_MAX_MESSAGE_LEN];
    uint8_t rx_buf_len = sizeof(rx_buf);
    uint8_t _from;
    uint8_t _to;
    while (recvfrom(rx_buf, &rx_buf_len, &_from, &_to))
    {
	// Never ACK an ACK
	if (!(rx_buf[0] & HRF_FLAG_ACK))
	{
	    uint8_t sequenceNumber = rx_buf[0] & HRF_SQN_MASK;
	    // Its a normal message for this node, not an ACK
	    if (_to != HRF_BROADCAST_ADDRESS)
	    {
		// Acknowledge
		uint8_t tx_buf = sequenceNumber | HRF_FLAG_ACK;
		if (!sendto(_from, &tx_buf, 1))
		    return false; // Failed
	    }
	    if (sequenceNumber != _seenSequenceNumbers[_from])
	    {
		if (*len > (rx_buf_len - 1))
		    *len = rx_buf_len - 1;
		memcpy(buf, rx_buf+1, *len);
		if (from)
		    *from = _from;
		if (to)
		    *to = _to;
		// record the fact that we have seen this message
		_seenSequenceNumbers[_from] = sequenceNumber;
		return true;
	    }
	}
	// Go round again
	rx_buf_len = sizeof(rx_buf);
    }
    // No message for us available
    return false;
}
