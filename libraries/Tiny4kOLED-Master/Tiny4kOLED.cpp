/*
 * SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays
 *
 * @created: 2014-08-12
 * @author: Neven Boyanov
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 * @ modified to include 16x32 fonts: 2017-01-12 
 * @author: M.V.Predoi 
 * Fits into Digispark ATtiny85 (6012 bytes)
 * Distributed as open source software under MIT License, see LICENSE.txt file.
 * Please, as a favor, retain the link https://sites.google.com/site/pre2arduino/
-----------------------------------------------------------------------------------
 SSD1306xLED is a C library for the SSD1306 display driver to control dot matrix OLED/PLED 128x64 displays. 
 It is working with ATtiny85 or similar microcontroller.
 The SSD1306xLED library is written in plain C and does not require any additional libraries to function except those that come with the SDK.
 This library has support for and tested work with the 8 MHz CPU clock mode.
*/


#include <stdlib.h>
#include <avr/io.h>

#include <avr/pgmspace.h>
#include "Tiny4kOLED.h"


#ifdef FONT6X8			// In case font6x8 is defined: load it
	#include "font6x8.h"
#endif

#ifdef FONT8X16			     // In case font8x16 is defined: load it
	#include "font8x16.h"
#endif

#ifdef FONT16X32			     // In case font16x32 is defined: load it
	#include "font16x32.h"
#endif



#define SSD1306_PAGES 4
#define SSD1306_MAX_PAGE 3
#define SSD1306_MAX_PAGE_8x16 2

#define SSD1306_COMMAND 0x00
#define SSD1306_DATA 0x40

// ----------------------------------------------------------------------------

// Some code based on "IIC_without_ACK" by http://www.14blog.com/archives/1358

const uint8_t ssd1306_init_sequence [] PROGMEM = {	// Initialization Sequence
	0xAE,			// Display OFF (sleep mode)
	0x20, 0b00,		// Set Memory Addressing Mode
					// 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;
					// 10=Page Addressing Mode (RESET); 11=Invalid
	0xB0,			// Set Page Start Address for Page Addressing Mode, 0-7
	0xC8,			// Set COM Output Scan Direction
	0x00,			// ---set low column address
	0x10,			// ---set high column address
	0x40,			// --set start line address
	0x81, 0x8F,		// Set contrast control register
	0xA1,			// Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
	0xA6,			// Set display mode. A6=Normal; A7=Inverse
	0xA8, 0x1F,		// Set multiplex ratio(1 to 64)
	0xA4,			// Output RAM to Display
					// 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
	0xD3, 0x00,		// Set display offset. 00 = no offset
	0xD5, 0x80,		// --set display clock divide ratio/oscillator frequency
	0xD9, 0x22,		// Set pre-charge period
	0xDA, 0x02,		// Set com pins hardware configuration
	0xDB,			// --set vcomh
	0x20,			// 0x20,0.77xVcc
	0x8D, 0x14		// Set DC-DC enable
};

uint8_t oledFont, ci, oledX, oledY = 0;
uint8_t renderingFrame, drawingFrame = 0;

SSD1306Device::SSD1306Device(void){}

void SSD1306Device::begin(void) {
	TinyWireM.begin();

	ssd1306_send_start(SSD1306_COMMAND);
	for (uint8_t i = 0; i < sizeof(ssd1306_init_sequence); i++) {
		ssd1306_send_byte(SSD1306_COMMAND, pgm_read_byte(&ssd1306_init_sequence[i]));
	}
	ssd1306_send_stop();
}



void SSD1306Device::setFont(uint8_t font) {
	oledFont = font;
}






void SSD1306Device::ssd1306_send_start(uint8_t transmission_type) {
	TinyWireM.beginTransmission(SSD1306);
	TinyWireM.write(transmission_type);
}

void SSD1306Device::ssd1306_send_stop(void) {
	TinyWireM.endTransmission();
}

void SSD1306Device::ssd1306_send_byte(uint8_t transmission_type, uint8_t byte) {
	if (TinyWireM.write(byte) == 0) {
		ssd1306_send_stop();
		ssd1306_send_start(transmission_type);
		TinyWireM.write(byte);
	}
}

void SSD1306Device::ssd1306_send_command(uint8_t command) {
	ssd1306_send_start(SSD1306_COMMAND);
	TinyWireM.write(command);
	ssd1306_send_stop();
}

void SSD1306Device::setCursor(uint8_t x, uint8_t y) {
	ssd1306_send_start(SSD1306_COMMAND);
	if (renderingFrame == 1) {
		TinyWireM.write(0xb0 | ((y + 4) & 0x07));			
	}
	else {
		TinyWireM.write(0xb0 | (y & 0x07));
	}
	TinyWireM.write(((x & 0xf0) >> 4) | 0x10);
	TinyWireM.write(x & 0x0f);
	ssd1306_send_stop();
	oledX = x;
	oledY = y;
}

void SSD1306Device::clear(void) {
	fill(0x00);
}

void SSD1306Device::fill(uint8_t fill) {
	for (uint8_t m = 0; m < SSD1306_PAGES; m++) {
		setCursor(0, m);
		ssd1306_send_start(SSD1306_DATA);
		for (uint8_t n = 0; n < 128; n++) {
			ssd1306_send_byte(SSD1306_DATA, fill);
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}




size_t SSD1306Device::write(byte c) {
// This function is modified to work also with 16x32 fonts for digits
//  Useful to display only numerical results in Largest Size!
//=================================================================
#ifdef FONT16X32	
 if(oledFont==2) {  
	if (c == '\r')
		return 1;
	if (c == '\n') {
		setCursor(0, oledY);
		return 1;
	}
	// If "font16x32.h" contains only 10 symbols (0..9) rotated 90deg. right, then:
  	ci = c - 44;  	// Only symbols ,-./0...9:  are available in 13kB file [0=char(16)]
      oledY = 0;			   	// Can only write from the top position..
      setCursor(oledX, oledY);   //  then set oledY=0.
	for (uint8_t j = 3; j > 1 ; j--) { 		// Send from top to bottom 2*16 bytes
		ssd1306_send_start(SSD1306_DATA);
		for (uint8_t i = 0; i < 16; i++) { 	// Send from top to bottom 16 vertical bytes
			TinyWireM.write(pgm_read_byte(&ssd1306xled_font16x32[ci * 64 + 4*i +j]));
		}
		ssd1306_send_stop();
		setCursor(oledX, oledY + 4-j);   		// Prepare position of next row of 16 bytes
	}
       setCursor(oledX, oledY-1);   			// Return a row higher
	for (uint8_t j = 3; j > 1 ; j--) { 		// Send from the lower half 2*16 bytes
		ssd1306_send_start(SSD1306_DATA);
		for (uint8_t i = 0; i < 16; i++) { 	// Send from top to bottom 2*16 bytes
			TinyWireM.write(pgm_read_byte(&ssd1306xled_font16x32[ci * 64 + 4*i +j -2]));
		}
		ssd1306_send_stop();
		setCursor(oledX, 3);   			// Prepare position of last row of 16 bytes
	}
     setCursor(oledX+16, oledY);				// Move cursor for next digit
  }
#endif
//==================================================================
#ifdef FONT8X16
if(oledFont==1) { 
	if (c == '\n') {
		oledY++;
  		if (oledY > SSD1306_MAX_PAGE) {
			oledY = SSD1306_MAX_PAGE;
  		}
  		setCursor(0, oledY);
			return 1;
      }

	ci = c - 32;

	if (oledX > 120) {
		oledY+=2;
		if (oledY > SSD1306_MAX_PAGE_8x16) {
			oledY = SSD1306_MAX_PAGE_8x16;
		}
			setCursor(0, oledY);
	}
	ssd1306_send_start(SSD1306_DATA);
	for (uint8_t i = 0; i < 8; i++) {
		TinyWireM.write(pgm_read_byte(&ssd1306xled_font8x16[ci * 16 + i]));
	}
	ssd1306_send_stop();
	setCursor(oledX, oledY + 1);
	ssd1306_send_start(SSD1306_DATA);
	for (uint8_t i = 0; i < 8; i++) {
		TinyWireM.write(pgm_read_byte(&ssd1306xled_font8x16[ci * 16 + i + 8]));
	}
	ssd1306_send_stop();
	setCursor(oledX + 8, oledY - 1);
 }
#endif
//==================================================================
#ifdef FONT6X8
 if(oledFont== 0) { 
	if (c == '\n') {
		if (oledFont == FONT6X8) {
			oledY++;
			if (oledY > SSD1306_MAX_PAGE) {
				oledY = SSD1306_MAX_PAGE;
			}
			setCursor(0, oledY);
			return 1;
		}
	}
	ci = c - 32;
	if (oledX > 122) {
		oledY++;
		if (oledY > SSD1306_MAX_PAGE) {
			oledY = SSD1306_MAX_PAGE;
		}
		setCursor(0, oledY);
	}

	ssd1306_send_start(SSD1306_DATA);
	for (uint8_t i= 0; i < 6; i++) {
		ssd1306_send_byte(SSD1306_DATA, pgm_read_byte(&ssd1306xled_font6x8[ci * 6 + i]));
	}
	ssd1306_send_stop();
	oledX+=6; // we don't need to call setCursor for every character.
 }
#endif
//==================================================================
return 1;
}



void SSD1306Device::bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]) {
	uint16_t j = 0;
 	for (uint8_t y = y0; y < y1; y++) {
		setCursor(x0,y);
		ssd1306_send_start(SSD1306_DATA);
		for (uint8_t x = x0; x < x1; x++) {
			ssd1306_send_byte(SSD1306_DATA, pgm_read_byte(&bitmap[j++]));
		}
		ssd1306_send_stop();
	}
	setCursor(0, 0);
}

void SSD1306Device::on(void) {
	ssd1306_send_command(0xAF);
}

void SSD1306Device::off(void) {
	ssd1306_send_command(0xAE);
}

void SSD1306Device::clearToEOL(void) {
	fillToEOL(0x00);
}

void SSD1306Device::fillToEOL(uint8_t fill) {
	fillLength(fill, 128 - oledX);
}

void SSD1306Device::fillLength(uint8_t fill, uint8_t length) {
	ssd1306_send_start(SSD1306_DATA);
	for (uint8_t n = 0; n < length; n++) {
		ssd1306_send_byte(SSD1306_DATA, fill);
	}
	ssd1306_send_stop();
	oledX += length;
}

void SSD1306Device::switchRenderFrame(void) {
	if (renderingFrame == 1) {
		renderingFrame = 0;
	}
	else {
		renderingFrame = 1;
	}
}

void SSD1306Device::switchDisplayFrame(void) {
	if (drawingFrame == 1) {
		drawingFrame = 0;
		ssd1306_send_command(0x40);
	}
	else {
		drawingFrame = 1;
		ssd1306_send_command(0x60);
	}
}

void SSD1306Device::switchFrame(void) {
	switchDisplayFrame();
	switchRenderFrame();
}

SSD1306Device oled;

// ----------------------------------------------------------------------------
