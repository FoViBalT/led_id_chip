/* 
 * File:   main.h
 * Author: mrkev
 *
 * Created on 4 ???? 2022 ?., 19:45
 */

#ifndef MAIN_H
#define	MAIN_H

#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* MAIN_H */

/*
 * main.h
 *
 * Created: 04.06.2022 17:46:10
 *  Author: mrkev
 */


#ifndef MAIN_H_
#define MAIN_H_
#endif /* MAIN_H_ */

#include <avr/io.h>
#define F_CPU 1000000UL

// ===protocol time values===
#define T1H 140
#define T1L 150
#define T0H 190
#define T0L 150
#define ENDH 290
#define ENDL 300
#define JITER 20 // allowed timing deviation
#define TCORR 0  // timing correction
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
#define DEBUG_PIN2 PORTB1

// ===chip settings===
// select led type
#define LED_TYPE SK6812_5V
//#define LED_TYPE WS2812B_5V

// set led strip length
#define LED_LENGHT 60


