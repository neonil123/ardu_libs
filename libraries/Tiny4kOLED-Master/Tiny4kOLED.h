/*
 * SSD1306xLED - Drivers for SSD1306 controlled dot matrix OLED/PLED 128x64 displays
 *
 * @created: 2014-08-12
 * @author: Neven Boyanov
 *
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 *
 */
#include <stdint.h>
#include <Arduino.h>
#include <TinyWireM.h>  // Version with buffer bugfix: https://github.com/adafruit/TinyWireM
#include <util/delay.h>


/* Uncomment from the 3 line below this, the fonts you intend to use:
*  FONT6X8  = 764 bytes
*  FONT8X16 = 1776 bytes
*  FONT16X32= 1334 bytes
*/

#define FONT6X8	 0
#define FONT8X16	 1
#define FONT16X32 	2


// ----------------------------------------------------------------------------

#ifndef SSD1306
	#define SSD1306		0x3C	// Slave address
#endif

// ----------------------------------------------------------------------------

class SSD1306Device: public Print {

	public:
		SSD1306Device(void);
		void begin(void);
		void on(void);
		void off(void);
		void switchRenderFrame(void);
		void switchDisplayFrame(void);
		void switchFrame(void);
		void setFont(uint8_t font);
		void setCursor(uint8_t x, uint8_t y);
		void fill(uint8_t fill);
		void fillToEOL(uint8_t fill);
		void fillLength(uint8_t fill, uint8_t length);
		void clear(void);
		void clearToEOL(void);
		void bitmap(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, const uint8_t bitmap[]);
		virtual size_t write(byte c);
		using Print::write;
	private:
		void ssd1306_send_command(uint8_t command);
		void ssd1306_send_start(uint8_t transmission_type);
		void ssd1306_send_byte(uint8_t transmission_type, uint8_t byte);
		void ssd1306_send_stop();
};

extern SSD1306Device oled;

// ----------------------------------------------------------------------------

