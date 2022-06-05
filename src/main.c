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
void sendEndToMaster();
void sendTrashToMaster();
void sendACKToMaster();

void send1ToSlave();
void send0ToSlave();
void sendEndToSlave();

void sendBuferToMaster(uint8_t buf[3]);
void sendReceiveBuferBack();

/*
state == 0; start of pulse
state == 1; end of pulse
state == 3; packet received
state == 100; ignore interrupt
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
	sei();				   // enable interrupt

	// debug setup
	DDRB |= (1 << DEBUG_PIN1); // Make DEBUG_PIN be an output.
	DDRB |= (1 << DEBUG_PIN2); // Make DEBUG_PIN be an output.
	DDRB &= ~(1 << IN_PIN);	   // Make IN_PIN be an input.

	while (1)
	{
		//????????????
		DDRB |= (1 << DEBUG_PIN1); // Make DEBUG_PIN be an output

		// if packet is fully received
		if (state == 3)
		{
			//  check packet data
			if (receiveBuffer[0] == REQUEST)
			{
				PCICR = 0; // disable PCINT
				_delay_us(150);
				PCMSK = (1 << OUT_PIN); // trigger interrupt on OUT_PIN pin change
				// clear receive buffer
				receiveBuffer[0] = 0;
				receiveBuffer[1] = 0;
				receiveBuffer[2] = 0;

				// say to master that id chip is connected
				DDRB |= (1 << IN_PIN); // master pin as output
				sendTrashToMaster();
				DDRB &= ~(1 << IN_PIN); // master pin as input

				// request length data from slave chip
				// send request (10; 0x80)
				DDRB |= (1 << OUT_PIN); // slave pin as output
				send1ToSlave();
				send0ToSlave();
				sendEndToSlave();
				DDRB &= ~(1 << OUT_PIN); // slave pin as input

				PCICR = (1 << PCIE0);	// enable PCINT back
				PCMSK = (1 << OUT_PIN); // trigger interrupt on OUT_PIN pin change
				// wait until data is received
				state = 0;
				// setup watchdog timer
				_delay_us(1000);
				uint32_t watchdogTimer = 0;
				while (state != 1) // some data is available
				{
					if (watchdogTimer == 0xFFFFFFFF)
					{
						state = 3;
						break;
					}
					watchdogTimer++;
				}
				// wait for data end
				while (state != 3)
					;

				// parse received packet
				uint16_t ledLenght = 0;
				// validation
				if (receiveBuffer[0] == WS2812B_5V ||
					receiveBuffer[0] == SK6812_5V)
				{
					ledLenght = receiveBuffer[1] << 8 | receiveBuffer[2];
				}

				// create end packet
				ledLenght += LED_LENGHT;
				uint8_t data[3];
				data[0] = LED_TYPE;
				data[1] = ledLenght & (0xFF << 8);
				data[2] = ledLenght & 0xFF;

				PCICR = 0;
				sendBuferToMaster(data);
				PCICR = (1 << PCIE0);
				PCMSK = (1 << IN_PIN); // trigger interrupt on IN_PIN pin change
			}
			else if (receiveBuffer[0] == ACK)
			{
				PCICR = 0;
				sendACKToMaster();
				PCICR = (1 << PCIE0);
			}
			//...
			//  sendReceiveBuferBack();
			state = 0;
		}
	}
}
/*
void sendReceiveBuferBack()
{
sendBuferToMaster(receiveBuffer);
}
*/

void sendBuferToMaster(uint8_t buf[3])
{
	_delay_us(300);		   // protocol delay
	DDRB |= (1 << IN_PIN); // make pin output

	// send back all received data(3 bytes)
	for (uint8_t i = 0; i < 3; i++) // foreach byte
	{
		for (uint8_t j = 7; j != 255; j--) // foreach bit
		{
			if (buf[i] & (1 << j))
			{
				send1ToMaster();
			}
			else
			{
				send0ToMaster();
			}
		}
	}
	sendEndToMaster();

	DDRB &= ~(1 << IN_PIN); // make pin input
}

void sendACKToMaster()
{
	DDRB |= (1 << IN_PIN); // make pin output
	send1ToMaster();
	send1ToMaster();
	sendEndToMaster();
	DDRB &= ~(1 << IN_PIN); // make pin input
}

void startTimer()
{
	TCNT0 = 0;			   // reset timer
	TCCR0B |= (1 << CS00); // no prescaler
}

void stopTimer()
{
	TCCR0B = 0; // disable timer
}

uint16_t readTimer()
{
	uint8_t sreg;
	uint16_t i;
	sreg = SREG; // save interrupt state
	cli();		 // disable interrupts
	i = TCNT0;	 // read data
	SREG = sreg; // restore interrupt state
	return i;
}

uint8_t aboutSame(uint16_t a, uint16_t b, uint8_t delta)
{
	return abs(a - b) < delta;
}

void set1ToReceiveBuff(uint8_t i)
{
	uint8_t n = i / 8;			  // get byte number
	i %= 8;						  // get bit number
	i = 7 - i;					  // invert(or received byte will be rotated)
	receiveBuffer[n] |= (1 << i); // set 1
}

void set0ToReceiveBuff(uint8_t i)
{
	// uint8_t n = i / 8;
	// i %= 8;
	// i = -i;
	// receiveBuffer[n] &= ~(1 << i); // set 0
}

void send1ToMaster()
{

	PORTB |= (1 << IN_PIN);
	_delay_us(T1H - TCORR);
	PORTB &= ~(1 << IN_PIN);
	_delay_us(T1L - TCORR);
}

void send0ToMaster()
{
	PORTB |= (1 << IN_PIN);
	_delay_us(T0H - TCORR);
	PORTB &= ~(1 << IN_PIN);
	_delay_us(T0L - TCORR);
}

void sendEndToMaster()
{
	PORTB |= (1 << IN_PIN);
	_delay_us(ENDH - TCORR);
	PORTB &= ~(1 << IN_PIN);
	_delay_us(ENDH - TCORR);
}

void sendTrashToMaster()
{
	PORTB |= (1 << IN_PIN);
	_delay_us(500);
	PORTB &= ~(1 << IN_PIN);
	_delay_us(ENDH - TCORR);
}

void send1ToSlave()
{

	PORTB |= (1 << OUT_PIN);
	_delay_us(T1H - TCORR);
	PORTB &= ~(1 << OUT_PIN);
	_delay_us(T1L - TCORR);
}

void send0ToSlave()
{
	PORTB |= (1 << OUT_PIN);
	_delay_us(T0H - TCORR);
	PORTB &= ~(1 << OUT_PIN);
	_delay_us(T0L - TCORR);
}

void sendEndToSlave()
{
	PORTB |= (1 << OUT_PIN);
	_delay_us(ENDH - TCORR);
	PORTB &= ~(1 << OUT_PIN);
	_delay_us(ENDH - TCORR);
}

// ===debug===
void blink0()
{
	DDRB |= (1 << DEBUG_PIN1);	 // Make DEBUG_PIN be an output.
	PORTB |= (1 << DEBUG_PIN1);	 // Turn the LED on.
	PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}

void blink1()
{
	DDRB |= (1 << DEBUG_PIN1);	// Make DEBUG_PIN be an output.
	PORTB |= (1 << DEBUG_PIN1); // Turn the LED on.
	_delay_us(10);
	PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}

void blink2()
{
	DDRB |= (1 << DEBUG_PIN1);	// Make DEBUG_PIN be an output.
	PORTB |= (1 << DEBUG_PIN1); // Turn the LED on.
	_delay_us(20);
	PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}

void blink3()
{
	DDRB |= (1 << DEBUG_PIN1);	// Make DEBUG_PIN be an output.
	PORTB |= (1 << DEBUG_PIN1); // Turn the LED on.
	_delay_us(30);
	PORTB &= ~(1 << DEBUG_PIN1); // Turn the LED off.
}
void blink4()
{
	DDRB |= (1 << DEBUG_PIN2);	 // Make DEBUG_PIN be an output.
	PORTB |= (1 << DEBUG_PIN2);	 // Turn the LED on.
	PORTB &= ~(1 << DEBUG_PIN2); // Turn the LED off.
}

void blink5()
{
	DDRB |= (1 << DEBUG_PIN2);	// Make DEBUG_PIN be an output.
	PORTB |= (1 << DEBUG_PIN2); // Turn the LED on.
	_delay_us(30);
	PORTB &= ~(1 << DEBUG_PIN2); // Turn the LED off.
}