/*
 * LAB Name: Arduino Execution Time Measurement (IO)
 * Author: Khaled Magdy
 * For More Info Visit: www.DeepBlueMbedded.com
*/
// ---[ Arduino Pin Manipulation Macros ]---
#define SET_PIN_HIGH(port, pin) (PORT ## port |= (1 << pin))
#define SET_PIN_LOW(port, pin) ((PORT ## port) &= ~(1 << (pin)))
#define LOOPS 1
float x=1.2345, y=6.789;

void TestFunction(void)
{
  cli(); // Disable Interrupts
  SET_PIN_HIGH(B, 0); // Set IO TestPin
  //----[ Function Body]----
  for(int i=0; i<LOOPS; i++)
  {
    x *= y;
  }
  //------------------------
  SET_PIN_LOW(B, 0); // Clear IO TestPin
  sei(); // Re-Enable Interrupts
}

void setup()
{
  pinMode(8, OUTPUT); // Set Pin8 (PB0) As Output
}
 
void loop()
{
  TestFunction();
  delay(100);
}