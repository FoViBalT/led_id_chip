# led id chip
identification chip firmware for led strips
 
 uses custom self tacting protocol
 
```
       T1H  T1L T0H  T0L
       ____      __
      |    |    |  |
      |    |    |  |
....__|    |____|  |__.......
```
protocol time values are located in main.h

compiled with XC8 MPLAB compiler with -Os flag (uses 868 of 1024 bytes)
