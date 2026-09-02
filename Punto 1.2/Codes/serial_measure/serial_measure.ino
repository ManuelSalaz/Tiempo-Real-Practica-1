/*
 * LAB Name: Arduino Interrupt Latency (Speed) Measurement2
 * Author: Khaled Magdy
 * For More Info Visit: www.DeepBlueMbedded.com
*/

volatile int var;

ISR(TIMER1_OVF_vect)
{
  var = TCNT1;
  Serial.println(var);
}

void setup()
{
  TCCR1A = 0;           // Init Timer1
  TCCR1B = 0;           // Init Timer1
  TCCR1B |= B00000001;  // Prescalar = 1
  TIMSK1 |= B00000001;  // Enable Timer Overflow Interrupt
  Serial.begin(115200);
}
 
void loop()
{
  // Do Nothing
}