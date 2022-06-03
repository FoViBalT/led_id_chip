#include <avr/io.h>

// ===protocol time values===
#define T1H 150
#define T1L 150
#define T0H 200
#define T0L 150
#define ENDH 300
#define ENDL 300
#define JITER 20
/*
       T1H  T1L T0H  T0L
       ____      __
      |    |    |  |
      |    |    |  |
....__|    |____|  |__.......
*/

// ===protocol values===
#define ACK 0xC0
#define REQUEST 0x80

#define SK6812_5V 0xA1
#define WS2812B_5V 0xA2

// ===pinout===
#define IN_PIN PORTB0
#define OUT_PIN PORTB1
#define DEBUG_PIN1 PORTB2
//#define DEBUG_PIN2 PORTB1

// ===chip settings===
// select led type
#define LED_TYPE SK6812_5V
//#define LED_TYPE WS2812B_5V

// set led strip lenght
#define LED_LENGHT 60