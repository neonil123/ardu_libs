# Makefile
# for Linux version of HopeRF and example programs

CFLAGS = -g -I . -I .. -DTEST 

CPPFLAGS = $(CFLAGS)

PROGS = server test_hrf

all:	$(PROGS)

hrf.o:	hrf.cpp ../HRFMessage.cpp ../HRFMessage.h ../HRFDatagram.cpp ../HRFDatagram.h ../HRFReliableDatagram.cpp ../HRFReliableDatagram.h WProgram.h

test_hrf:	hrf.o test_hrf.o HardwareSerial.o
	cc -g -o $@ $^ -lstdc++

server:	server.o HardwareSerial.o hrf.o
	gcc -g -o $@ $^ -lstdc++

clean:
	rm -f *~ *.o $(PROGS)