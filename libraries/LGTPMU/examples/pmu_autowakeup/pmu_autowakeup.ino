#include <PMU.h>

void setup()
{
  Serial.begin(9600);
}
 
void loop()
{
  Serial.write("hello, I am working!\n");
  delay(100);
  
  // power/down mode 
  // disable all peripherals

    PMU.sleep(PM_POFFS0, SLEEP_1S);  
}
