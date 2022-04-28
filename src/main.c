#include "main.h"

#include <avr/interrupt.h>

#include <util/delay.h>

#include <stdlib.h>

void blink();

void startTimer();
void stopTimer();
uint16_t readTimer();

uint8_t aboutSame(uint16_t a, uint16_t b, uint8_t delta);

void set1ToReceiveBuff(uint8_t i);
void set0ToReceiveBuff(uint8_t i);
/*
state == 0; start of pulse
state == 1; end of pulse
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
      blink();
    }
  }
  else if (state == 1)
  {
    // some signal end
    stopTimer();
    uint16_t time = readTimer();

    // is signal 0 or 1
    if (aboutSame(time, T1H, JITER))
    {
      blink();
      set1ToReceiveBuff(receiveBufferPointer);
      receiveBufferPointer++;
    }
    else if (aboutSame(time, T0H, JITER))
    {
      set0ToReceiveBuff(receiveBufferPointer);
      receiveBufferPointer++;
    }
    else if (aboutSame(time, ENDH, JITER))
    {
      // end of message
      receiveBufferPointer = 0;
    }

    // reset  receiveBufferPointer if overflow
    if (receiveBufferPointer == 24)
    {
      receiveBufferPointer = 0;
    }
    state = 0;
  }
}

int main()
{
  // setup interrupt
  PCMSK = (1 << IN_PIN); // trigger interrupt on PB0 pin change
  PCICR = (1 << PCIE0);  // enable PCINT0 interrupt
  sei();                 // enable interrupt

  // debug setup
  DDRB |= (1 << DEBUG_PIN); // Make DEBUG_PIN be an output.

  while (1)
  {
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
  uint8_t n = i % 8;
  i /= 8;
  receiveBuffer[n] |= (1 << i); // set 1
}

void set0ToReceiveBuff(uint8_t i)
{
  uint8_t n = i % 8;
  i /= 8;
  receiveBuffer[n] &= ~(1 << i); // set 0
}

void blink()
{
  PORTB |= (1 << DEBUG_PIN); // Turn the LED on.
  _delay_ms(100);
  PORTB &= ~(1 << DEBUG_PIN); // Turn the LED off.
  _delay_ms(100);
}
