// HRFReliableDatagram.h
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HRFReliableDatagram.h,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#ifndef HRFReliableDatagram_h
#define HRFReliableDatagram_h

#include <HRFDatagram.h>

// Mask for the Sequence Number in FLAG/SQN
#define HRF_SQN_MASK 0x7f

// The acknowledgement bit in the FLAG/SQN
#define HRF_FLAG_ACK 0x80

// The maximum payload length
#define HRF_MAX_RELIABLE_DATAGRAM_PAYLOAD HRF_MAX_DATAGRAM_PAYLOAD-1

/////////////////////////////////////////////////////////////////////
/// \class HRFReliableDatagram HRFReliableDatagram.h <HRFReliableDatagram.h>
/// \brief Class for sending addressed, acknowledged, retransmitted datagrams.
///
/// Extends HRFDatagram to define addressed, reliable datagrams with acknowledgement and retransmission.
/// Based on HRFDatagram, adds flags and sequence numbers. HRFReliableDatagram is reliable in the sense
/// that messages are acknowledged, and unacknowledged messages are retransmitted until acknowledged or the
/// retries are exhausted.
/// When addressed messages are collected by the application (recvfromAck), an acknowledgement is automatically sent.
/// When addressed messages are sent (by sendtoWait), it will wait for an ack, and retransmit
/// after timeout until an ack is received or retries are exhausted
/// ReliableDatagrams have format LEN DEST SRC FLAG/SQN payload FCS-LO FCS-HI
/// Low 7 bits of FLAG/SQN is the sequence number. Most significant bit is the ACK flag.
///
/// Part of the Arduino HopeRF library for operating with HopeRF HM-TR transceivers 
/// (see http://www.hoperf.com)
class HRFReliableDatagram : public HRFDatagram
{
private:
    /// The last sequence number to be used
    /// Defaults to 0
    uint8_t _lastSequenceNumber;

    // Retransmit timeout (milliseconds)
    /// Defaults to 1000
    long _timeout;

    // Retries (0 means one try only)
    /// Defaults to 3
    uint8_t _retries;

    /// Array of the last seen sequence number indexed by node address that sent it
    /// It is used for duplicate detection. Duplicated messages are re-acknowledged when received 
    /// (this is generally due to lost ACKs, causing the sender to retransmit, even though we have already
    /// received that message)
    uint8_t _seenSequenceNumbers[256];

public:
    /// \param[in] serial The instance of HardwareSerial to use for IO. Defaults to &Serial, 
    /// \param[in] thisAddress The address to assign to this node. Defaults to 0
    HRFReliableDatagram(HardwareSerial* serial = &Serial, uint8_t thisAddress = 0);

    /// Sets the retransmit timeout. If sendtoWait is waiting for an ack longer than this time (in milliseconds), 
    /// it will retransmit the message. Defaults to 1000ms. The timeout is measured from the beginning of
    /// transmission of the message. It must be longer than the transmit time of the request plus the transmit 
    /// time of the acknowledgement (6 octets) plus the latency of the receiver.
    /// \param[in] timeout The new timeout period in milliseconds
    virtual void setTimeout(long timeout);

    /// Sets the max number of retries. Defaults to 3. If set to 0, the message will only be sent once.
    /// sendtoWait will give up if there is no ack received after all transmissions time out.
    /// param[in] retries The maximum number a retries.
    virtual void setRetries(uint8_t retries);

    /// Send the message and waits for an ack. Returns true if an acknowledgement is received.
    /// Synchronous: any message other than the desired ACK received while waiting is discarded.
    /// Blocks until an ACK is received or all retries are exhausted (ie up to retries*timeout milliseconds).
    /// \param[in] address The address to send the message to.
    /// \param[in] buf Pointer to the binary message to send
    /// \param[in] len Number of octets to send
    /// \return true if the message was transmitted and an acknowledgement is received.
    /// \return false if the message is too long (>HRF_MAX_DATAGRAM_PAYLOAD).
    virtual uint8_t sendtoWait(uint8_t address, uint8_t* buf, uint8_t len);

    /// If there is a valid message available for this node, send an acknowledgement to the SRC
    /// address (blocking until this is complete), then copy the message to buf and return true
    /// else return false. 
    /// If a message is copied, *len is set to the length..
    /// If from is not NULL, the SRC address is placed in *from.
    /// If to is not NULL, the DEST address is placed in *to.
    /// This is the preferred function for getting messages addressed to this node.
    /// If the message is not a broadcast, acknowledge to the sender before returning.
    /// \param[in] buf Location to copy the received message
    /// \param[in] len Available space in buf. Set to the actual number of octets copied.
    /// \param[in] from If present and not NULL, the referenced uint8_t will be set to the SRC address
    /// \param[in] to If present and not NULL, the referenced uint8_t will be set to the DEST address
    virtual uint8_t recvfromAck(uint8_t* buf, uint8_t* len, uint8_t* from = NULL, uint8_t* to = NULL);
};

#endif

