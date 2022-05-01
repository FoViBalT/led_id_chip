#include "main.h"

#include <avr/interrupt.h>

#include <util/delay.h>

#include <stdlib.h>

void blink0();
void blink1();
void blink2();
void blink3();

void blink4();

void startTimer();
void stopTimer();
uint16_t readTimer();

uint8_t aboutSame(uint16_t a, uint16_t b, uint8_t delta);

void set1ToReceiveBuff(uint8_t i);
void set0ToReceiveBuff(uint8_t i);

void send1ToMaster();
void send0ToMaster();
/*
state == 0; start of pulse
state == 1; end of pulse
state == 3; packet received
*/
uint8_t state;

uint8_t receiveBuffer[3];
uint8_t receiveBufferPointer = 0;

ISR(PCINT0_vect)
{
  if (state == 0)
  {
    // start timer only on high signal
    if (PINB & (1 << IN_PIN))
    {
      startTimer();
      state = 1;
      blink0();
    }
  }
  else if (state == 1)
  {
    // some signal end
    stopTimer();
    uint16_t time = readTimer();
    blink0();
    //  debugNum(time);
    // is signal 0 or 1
    if (aboutSame(time, T1H, JITER))
    {
     // blink1();
      set1ToReceiveBuff(receiveBufferPointer);
      receiveBufferPointer++;
      state = 0;
    }
    else if (aboutSame(time, T0H, JITER))
    {
     // blink2();
      set0ToReceiveBuff(receiveBufferPointer);
      receiveBufferPointer++;
      state = 0;
    }
    else if (aboutSame(time, ENDH, JITER))
    {
      blink3();
      // end of message
      receiveBufferPointer = 0;
      state = 3; // answer to question
    }

    // reset  receiveBufferPointer if overflow
    if (receiveBufferPointer == 24)
    {
      receiveBufferPointer = 0;
    }
  }
}

int main()
{
  // setup interrupt
  PCMSK = (1 << IN_PIN); // trigger interrupt on PB0 pin change
  PCICR = (1 << PCIE0);  // enable PCINT0 interrupt
  sei();                 // enable interrupt

  // debug setup
  DDRB |= (1 << DEBUG_PIN1); // Make DEBUG_PIN be an output.
  DDRB |= (1 << DEBUG_PIN2); // Make DEBUG_PIN be an output.

  while (1)
  {
    blink4();
    if (state == 3)
    {
      _delay_us(300);
      DDRB |= (1 << IN_PIN); // make pin output
      for (uint8_t i = 0; i < 3; i++)
      {
        for (uint8_t j = 0; j < 8; j++)
        {
          if (receiveBuffer[i] & (1 << j))
          {
            send1ToMaster();
          }
          else
          {
            send0ToMaster();
          }
        }
      }
      DDRB &= ~(1 << IN_PIN); // make pin input
      state = 0;
    }
  }
}

inline void startTimer()
{
  TCNT0 = 0;             // reset timer
  TCCR0B |= (1 << CS00); // no prescaler
}

inline void stopTimer()
{
  TCCR0B = 0; // disable timer
}

uint16_t readTimer()
{
  uint8_t sreg;
  uint16_t i;
  sreg = SREG; // save interrupt state
  cli();       // disable interrupts
  i = TCNT0;   // read data
  SREG = sreg; // restore interrupt state
  return i;
}

uint8_t aboutSame(uint16_t a, uint16_t b, uint8_t delta)
{
  return abs(a - b) < delta;
}

void set1ToReceiveBuff(uint8_t i)
{
  uint8_t n = i / 8;
  i %= 8;
  receiveBuffer[n] |= (1 << i); // set 1
}

void set0ToReceiveBuff(uint8_t i)
{
  uint8_t n = i / 8;
  i %= 8;
  receiveBuffer[n] &= ~(1 << i); // set 0
}

void send1ToMaster()
{

  PORTB |= (1 << IN_PIN);
  _delay_us(T1H - 75);
  PORTB &= ~(1 << IN_PIN);
  _delay_us(T1L - 90);
}

void send0ToMaster()
{
  PORTB |= (1 << IN_PIN);
  _delay_us(T0H - 75); 
  PORTB &= ~(1 << IN_PIN);
  _delay_us(T0L - 90);
}

// debug
inline void blink0()
{
  PORTB |= (1 << DEBUG_PIN1);  // Turn the LED on.
  PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}

inline void blink1()
{
  PORTB |= (1 << DEBUG_PIN1); // Turn the LED on.
  _delay_us(10);
  PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}

inline void blink2()
{
  PORTB |= (1 << DEBUG_PIN1); // Turn the LED on.
  _delay_us(20);
  PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}

inline void blink3()
{
  PORTB |= (1 << DEBUG_PIN1); // Turn the LED on.
  _delay_us(30);
  PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}
inline void blink4()
{
  PORTB |= (1 << DEBUG_PIN2);  // Turn the LED on.
  PORTB &= ~(1 << DEBUG_PIN2); // Turn the LED off.
}