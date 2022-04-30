#include <avr/io.h>
// protocol time values
#define T1H 150
#define T1L 100
#define T0H 200
#define T0L 100
#define ENDH 300
#define ENDL 300
#define JITER 20

/*
      T1H T1L T0H T0L
       ___     __
      |   |   |  |
      |   |   |  |
....__|   |___|  |__.......
*/

#define IN_PIN PORTB0
#define OUT_PIN PORTB1
#define DEBUG_PIN PORTB2
