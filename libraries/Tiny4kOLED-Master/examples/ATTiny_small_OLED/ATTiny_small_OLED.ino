/* Digispark ATtiny85 to small OLED 128x32 by I2C
 *  https://github.com/datacute/Tiny4kOLED 
 *  Tiny4kOLED.h: SSD1306xLED-Drivers for OLED 128x32 displays
 * @created: 2014-08-12  @author: Neven Boyanov
 * Source code available at: https://bitbucket.org/tinusaur/ssd1306xled
 * @ modified for 16x32 fonts: 2017-01-12 @author: Mihai Valentin Predoi 
 * Fits (2944 bytes) into Digispark  ATtiny85 (6012 bytes). 52 bytes of dynamic memory.
 *  ATtiny85   OLED
 *    <<------>>
 *  PB0        SDA
 *  PB2        SCL
 *  5V         VCC
 *  GND        GND
 *  
 */

#include <Tiny4kOLED.h>
#include <TinyWireM.h>

// ============================================================================

void setup() {
  oled.begin();
  oled.clear();
  oled.on();
}

void loop() {
  oled.clear(); //all black
  oled.setFont(FONT16X32); // 2 lines of 16 characters exactly fills 128x32
   //next line: oled.setCursor(X IN PIXELS, Y - NO EFFECT since 32 bits is max height);
  oled.setCursor(0, 0);
  oled.print(F("-23.56:/")); //wrap strings in F() to save RAM!
  delay(5000);    // To see the display "refresh"
}

void setMaxClockSpeed()
{ // Call this function for Attiny85 processor 8MHz speed
// Examine Page 33
CLKPR = 0x80; // Setup CLKPCE to be receptive
CLKPR = 0x00; // No scalar

// Maximum clock speeds!
PLLCSR = _BV(PLLE) | _BV( PCKE );
OSCCAL = 0xff;
} 
