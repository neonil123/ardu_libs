// HRFMessage.h
//
/// \mainpage HopeRF Radio Transceiver Message Library for Arduino
///
/// This is the Arduino HopeRF library for operating with HopeRF HM-TR transceivers
/// (see http://www.hoperf.com)
/// It should also work with other 'transparent' serial transceivers.
/// See http://www.open.com.au/mikem/arduino/HopeRF/HopeRF.pdf
/// for package download details, electrical details and documentation 
/// for connecting to HM-TR transceivers.
///
/// The latest version of this documentation  can be downloaded from 
/// http://www.open.com.au/mikem/arduino/HopeRF
///
/// The version of the package that this documentation refers to can be downloaded 
/// from http://www.open.com.au/mikem/arduino/HopeRF/HopeRF-1.5.zip
///
/// The HopeRF HM-TR transceiver is an inexpensive 433MHz 'transparent' serial transceiver. It handles 
/// internally all the issues of preamble, synchronisation etc (which is the bulk of the work in my other
/// Arduino library, VirtualWire). Although the HM-TR is more expensive than the bare 433 MHz transceivers 
/// supported by VirtualWire, it requires much fewer compute resources from the Arduino (in fact the HM-TR has an 
/// ATMega on it specifically to do the serial-433MHz translation). However the HM-TR does not have any
/// error detection built in. That is provided by this library. 
///
/// This library provides classes for 
/// \li unaddressed, unreliable messages
/// \li addressed, unreliable messages
/// \li addressed, reliable, retransmitted, acknowledged messages. 
///
/// The Library has been tested on Arduino Mega, which has several serial ports besides the USB connection 
/// to the host PC. However, it would be expected to work with any Arduino with a serial port. 
/// The only connection required between the Arduino and the HM-TR module is power, received and transmit.
/// The library is wrtten in C++, and compiles on Linux. With only a little work it could be expected to run 
/// on Linux with a suitable HardwareSerial impoentation, and using HM-TR with RS-232 connections.
///
/// This software is Copyright (C) 2009 Mike McCauley. Use is subject to license
/// conditions. The main licensing options available are GPL V2 or Commercial:
/// 
/// \par Open Source Licensing GPL V2
/// This is the appropriate option if you want to share the source code of your
/// application with everyone you distribute it to, and you also want to give them
/// the right to share who uses it. If you wish to use this software under Open
/// Source Licensing, you must contribute all your source code to the open source
/// community in accordance with the GPL Version 2 when your application is
/// distributed. See http://www.gnu.org/copyleft/gpl.html
/// 
/// \par Commercial Licensing
/// This is the appropriate option if you are creating proprietary applications
/// and you are not prepared to distribute and share the source code of your
/// application. Contact info@open.com.au for details.
///
/// \par Revision History
/// \version 1.4 Compiles on Arduino 1.0
///
/// \version 1.5 HRFReliableDatagram sendToWait() incorrectly could wait forever. 
///   Reported by Nick Maddock.
///
/// \author  Mike McCauley (mikem@open.com.au)
// Copyright (C) 2009 Mike McCauley
// $Id: HRFMessage.h,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

#ifndef HRFMessage_h
#define HRFMessage_h

// When testing on Linux, the def of abs is broken by wiring.h
#ifdef TEST
#undef abs
#endif

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#endif
// These defs cause trouble on some versions of Arduino
#undef abs
#undef double
#undef round

#include <HardwareSerial.h>

/// \def HRF_MAX_MESSAGE_LEN
/// Maximum number of bytes in a message, counting the byte count and FCS
/// Tests show that more than 64 octets in a single message can cause
/// corrupted and missing characters, even though the Serial class has a buffer of 128, 
/// probably due to some undocumented buffer limitation in the HM-TR
#define HRF_MAX_MESSAGE_LEN 64

/// \def HRF_MAX_PAYLOAD
/// The maximum payload length
#define HRF_MAX_PAYLOAD HRF_MAX_MESSAGE_LEN-3

/////////////////////////////////////////////////////////////////////
/// \class HRFMessage HRFMessage.h <HRFMessage.h>
/// \brief Basic message class for HopeRF data transceivers
///
/// Define low level unaddressed, unreliable message.
/// HRFMessage have the format LEN payload FCS-LO FCS-HI.
/// LEN is the total number of octets in the message, including the LEN and FCS octets
/// FCS (16 bits) is the complement of CCITT CRC-16 of all octets in the message, including the LEN
/// HRFMessage is unaddressed, and will be received by all HRFMessage nodes within range.
///
/// This class and others in this library provide easy methods for sending and receiving messages using the HM-TR.
/// Several layers of classes are provided above this
/// to provide a range of features from unacknowledged broadcasts to 
/// acknowledged, reliable addressed messages.
///
/// Although HopeRF HM-TR transceivers have an enable pin, this library make no use of that. Enabling
/// and disabling of the transceiver when it is not required should be done by other code.
///
/// Although HopeRF HM-TR transceivers have a Config pin, this library make no use of that, since it must be
/// enabled when the transceiver powers up. 
/// This librray assumes the transceiver has already been configured, typically using the 
/// HopeRF configuration program.
class HRFMessage
{
private:
    /// This is the instance of HardwareSerial that will be used to communicate with the HM-TR
    HardwareSerial* _serial;

    /// Buffer for incoming messages
    uint8_t _rx_buf[HRF_MAX_MESSAGE_LEN];

    /// How many octets are currently in _rx_buf
    uint8_t _rx_len;

    /// Number of bad messages received and dropped due to bad lengths
    uint8_t _rx_bad;

    /// Number of good messages received
    uint8_t _rx_good;

public:
    /// You can have multiple instances for HM-TR modules on multiple HardwareSerial ports.
    /// \param[in] serial The instance of HardwareSerial to use for IO. Defaults to &Serial, 
    /// the Arduino default serial port.
    HRFMessage(HardwareSerial* serial = &Serial);

    /// Send a message with the given length. Blocks and returns after
    /// the entire message has been sent. Any binary data is permitted.
    /// \param[in] buf Pointer to the binary message to send
    /// \param[in] len Number of octets to send
    /// \return true if the message was transmitted
    /// \return false if the message is too long (>HRF_MAX_PAYLOAD)
    virtual uint8_t send(uint8_t* buf, uint8_t len);

    /// \return message length if a complete unread message is available
    /// \return 0 (false) if no message is available yet)
    virtual uint8_t available();

    /// If a message is available (good checksum or not), copies
    /// up to *len octets to buf.
    /// If a message is copied, *len is set to the length
    /// \return true if there was a message copied _and_ the FCS checksum was good
    /// \param[in] buf Location to copy the received message
    /// \param[in] len Available space in buf. Set to the actual number of octets copied.
    virtual uint8_t recv(uint8_t* buf, uint8_t* len);

    /// The bad message count starts at 0 at instantation time, and is incremented whenever 
    /// a message with a bad length is received. This generally indicates a corrupt messages.
    /// \return the number of corrupt messages received.
    virtual uint8_t rx_bad(void);

    /// The good message count starts at 0 at instantation time, and is incremented whenever
    /// a complete message is received,
    /// \return the number of good messages received.
    virtual uint8_t rx_good(void);

    /// Wait until at least 1 octet is available on the serial device
    /// On Arduino, polls the device. On Linux, blocks
    virtual void waitAvailable(void);
};
#endif
