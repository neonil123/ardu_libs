// test_hrf.cpp
//
// Run some tests on the HopeRF libraries.

#include <stdio.h>
#include <string.h>
#include <HRFReliableDatagram.h>

int errors = 0;

// Cross linked for testing:
MockHardwareSerial s1;
MockHardwareSerial s2;

HRFMessage dg1(&s1);
HRFMessage dg2(&s2);
HRFDatagram adg1(&s1);
HRFDatagram adg2(&s2);
HRFReliableDatagram rdg1(&s1);
HRFReliableDatagram rdg2(&s2);

void error(const char* msg)
{
    printf("Error: %s\n", msg);
    errors++;
}

// Test HRFMessage
void setup1()
{
    // Cross link the 2 test devices
    s1.setDest(&s2);
    s2.setDest(&s1);
}

void loop1()
{
    uint8_t* msg = (uint8_t*)"test\n";
    uint8_t result = dg1.send(msg, 6);

    if (!result)
	error("test1 send failed");

    if (!dg2.available())
	error("test1 available failed");
	
    uint8_t buf[HRF_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (dg2.recv((uint8_t*)&buf, &len))
    {
	if (len != 6)
	    error("test1 bad length");
	if (memcmp((const char*)buf, "test\n", 6) != 0)
	    error("test1 incorrect content");
    }
    else
	error("test1 recv failed");
}

void test1()
{
    int i;
    setup1();
    for (i = 0; i < 100; i++)
	loop1();
}

////////////////////////////////////////////////////////////////////
// Test HRFDatagram
void setup2()
{
    // Cross link the 2 test devices
    s1.setDest(&s2);
    s2.setDest(&s1);
    adg1.setThisAddress(11);
    adg2.setThisAddress(12);
}

void loop2()
{
    uint8_t* msg = (uint8_t*)"test\n";
    uint8_t result = adg1.sendto(12, msg, 6);

    if (!result)
	error("test2 send failed");

    if (!adg2.available())
	error("test2 available failed");
	
    uint8_t from;
    uint8_t buf[HRF_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (adg2.recvfrom((uint8_t*)&buf, &len, &from))
    {
	if (from != 11)
	    error("test2 bad from");
	if (len != 6)
	    error("test2 bad length");
	if (memcmp((const char*)buf, "test\n", 6) != 0)
	    error("test2 incorrect content");
    }
    else
	error("test2 recv failed");

    // There should not be any more messages for adg2
    if (adg2.available())
	error("test2 available 2 failed");

    result = adg1.sendto(100, msg, 6); // non existent address, should be dropped by adg2
    if (!result)
	error("test2 send 2 failed");
    // There should not be a message waiting at adg2, since the message was not addresed to it
    if (adg2.recvfrom((uint8_t*)&buf, &len, &from))
	error("test2 recv 2 failed");

    result = adg1.sendto(HRF_BROADCAST_ADDRESS, msg, 6); // Broadcast address, should be received by adg2
    if (!result)
	error("test2 send 3 failed");
    if (adg2.recvfrom((uint8_t*)&buf, &len, &from))
    {
	if (from != 11)
	    error("test2 bad from 2");
	if (len != 6)
	    error("test2 bad length 2");
	if (memcmp((const char*)buf, "test\n", 6) != 0)
	    error("test2 incorrect content 2");
    }
    else
	error("test2 recv 2 failed");

    result = adg1.sendto(100, msg, 6); // Should only get this in promiscuous mode
    if (!result)
	error("test2 send 4 failed");
    uint8_t to;
    if (adg2.recvfrom((uint8_t*)&buf, &len, &from, &to, true)) // Promiscuous
    {
	if (from != 11)
	    error("test2 bad from 3");
	if (to != 100)
	    error("test2 bad to");
	if (len != 6)
	    error("test2 bad length 3");
	if (memcmp((const char*)buf, "test\n", 6) != 0)
	    error("test2 incorrect content 3");
    }
    else
	error("test2 recv 3 failed");
}

void test2()
{
    int i;
    setup2();
    for (i = 0; i < 10; i++)
	loop2();
}

////////////////////////////////////////////////////////////////////
// Test HRFReliableDatagram
void setup3()
{
    // Cross link the 2 test devices
    s1.setDest(&s2);
    s2.setDest(&s1);
    rdg1.setThisAddress(11);
    rdg2.setThisAddress(12);
    // Permit a sort of asnychronous testing
    rdg1.setRetries(0);
    rdg2.setRetries(0);
    rdg1.setTimeout(0);
    rdg2.setTimeout(0);
}

void loop3()
{
    uint8_t* msg = (uint8_t*)"test\n";
    // Its hard to test this synchronous code properly. sendToWait should fail since the reciever does
    // not acknowledge. 
    uint8_t result = rdg1.sendtoWait(12, msg, 6);

    if (result)
	error("test3 send failed");

    // Nevertheless there should be a message waiting
    if (!rdg2.available())
	error("test3 available failed");
	
    uint8_t from;
    uint8_t buf[HRF_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rdg2.recvfromAck((uint8_t*)&buf, &len, &from))
    {
	if (from != 11)
	    error("test3 bad from");
	if (len != 6)
	    error("test3 bad length");
	if (memcmp((const char*)buf, "test\n", 6) != 0)
	    error("test3 incorrect content");
    }
    else
	error("test3 recv failed");
    // Expect the ACK to be waiting (uncollected so far) back at rdg1. Absorb it
    // Expect to get only the FLAGS/SQN
    len = sizeof(buf);
    if (rdg1.recvfrom((uint8_t*)&buf, &len, &from))
    {
	if (from != 12)
	    error("test3 bad from 2");
	if (len != 1)
	    error("test3 bad length 2");
    }
    else
	error("test3 ACK recv failed");
	
}

void test3()
{
    int i;
    setup3();
    for (i = 0; i < 10; i++)
	loop3();
}


int main(int argc, char**argv)
{
    test1();
    test2();
    test3();

    printf("Completed with %d errors\n", errors);
    return errors ? 1 : 0;
}
