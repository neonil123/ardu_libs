# Makefile
#
# Makefile for the Arduino HopeRF project
#
# Author: Mike McCauley (mikem@open.com.au)
# Copyright (C) 2009 Mike McCauley
# $Id: HRFMessage.h,v 1.1 2009/08/15 05:32:58 mikem Exp mikem $

DISTFILE = HopeRF-1.5.zip

all:	doxygen dist upload

doxygen: 
	doxygen project.cfg


ci:
	ci -l `cat MANIFEST`

dist:	
	(cd ..; zip HopeRF/$(DISTFILE) `cat HopeRF/MANIFEST`)

upload:
	scp $(DISTFILE) doc/*.html doc/*.gif doc/*.png doc/*.css doc/*.pdf server2:/var/www/html/mikem/arduino/HopeRF
