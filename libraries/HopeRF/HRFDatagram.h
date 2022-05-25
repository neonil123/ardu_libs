// HRFDatagram.h
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HRFDatagram.h,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#ifndef HRFDatagram_h
#define HRFDatagram_h

#include <HRFMessage.h>

// Broadcast address
#define HRF_BROADCAST_ADDRESS 0xff

// The maximum payload length
#define HRF_MAX_DATAGRAM_PAYLOAD HRF_MAX_PAYLOAD-2

/////////////////////////////////////////////////////////////////////
/// \class HRFDatagram HRFDatagram.h <HRFDatagram.h>
/// \brief Class for addressed, unreliable messages
///
/// Extends HRFMessage to define addressed, unreliable datagrams.
/// Every node has an 8 bit address (defaults to 0).
/// Datagrams have format LEN DEST SRC payload FCS-LO FCS-HI.
/// Addresses (DEST and SRC) are 8 bit integers with an address of HRF_BROADCAST_ADDRESS (0xff) 
/// reserved for broadcast.
///
/// Part of the Arduino HopeRF library for operating with HopeRF HM-TR transceivers 
/// (see http://www.hoperf.com).
class HRFDatagram : public HRFMessage
{
protected:
    /// The address of this node. Defaults to 0.
    uint8_t _thisAddress;

public:
    /// \param[in] serial The instance of HardwareSerial to use for IO. Defaults to &Serial, 
    /// \param[in] thisAddress The address to assign to this node. Defaults to 0
    HRFDatagram(HardwareSerial* serial = &Serial, uint8_t thisAddress = 0);

    /// Sets the address of this node. Defaults to 0. If all the nodes leave the address unset (ie 0),
    /// HRFDatagram acts much the same as HRFMessage (with a bit more overhead).
    /// In a conventional multinode system, all nodes will have a unique address (which you could store in EEPROM).
    /// However, if you only have 2 nodes in the network, 
    /// you can leave the addresses of each set to the default of 0.
    /// \param[in] thisAddress The address of this node
    virtual void setThisAddress(uint8_t thisAddress);

    /// Sends a message to the node(s) with the given address
    /// HRF_BROADCAST_ADDRESS is a valid address which will cause the message
    /// to be accepted by all HRFDatagram nodes within range.
    /// \param[in] address The address to send the message to.
    /// \param[in] buf Pointer to the binary message to send
    /// \param[in] len Number of octets to send
    /// \return true if the message was transmitted.
    /// \return false if the message is too long (>HRF_MAX_DATAGRAM_PAYLOAD).
    virtual uint8_t sendto(uint8_t address, uint8_t* buf, uint8_t len);

    /// If there is a message available for this node, copy it to buf and return true
    /// If the message was valid (ie good FCS), return true else return false. 
    /// The SRC address is placed in *from if present and not NULL.
    /// The DEST address is placed in *to if present and not NULL.
    /// If promiscuous is present and true, all messages will be returned, not just those addressed
    /// to this node.
    /// If a message is copied, *len is set to the length.
    /// \param[in] buf Location to copy the received message
    /// \param[in] len Available space in buf. Set to the actual number of octets copied.
    /// \param[in] from If present and not NULL, the referenced uint8_t will be set to the SRC address
    /// \param[in] to If present and not NULL, the referenced uint8_t will be set to the DEST address
    /// \param[in] promiscuous If present and true, return all received messages 
    /// (not just those addressed to this node).
    /// \return true if a valid (good FCS) message was copied to buf
    virtual uint8_t recvfrom(uint8_t* buf, uint8_t* len, uint8_t* from = NULL, uint8_t* to = NULL, uint8_t promiscuous = 0);

    /// If there is a valid message available for this node, copy it to buf and return true
    /// else return false. You can't recover the SRC or DEST address with this call, so you would only use it 
    /// if you do not intend to reply.
    /// If a message is copied, *len is set to the length.
    /// \param[in] buf Location to copy the received message
    /// \param[in] len Available space in buf. Set to the actual number of octets copied.
    /// \return true if a valid message was copied to buf
    virtual uint8_t recv(uint8_t* buf, uint8_t* len);

};

#endif
