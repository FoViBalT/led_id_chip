#!/bin/sh
/opt/microchip/xc8/v2.36/bin/xc8-cc -mcpu=ATtiny10 -c -x c -funsigned-char -funsigned-bitfields -mext=cci -D__ATtiny10__ -DNDEBUG -v -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums  -Wall  "main.c"

/opt/microchip/xc8/v2.36/bin/xc8-cc -o XC8Application1.elf  main.o   -mcpu=ATtiny10  -Wl,-Map="XC8Application1.map" -funsigned-char -funsigned-bitfields -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax

avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "XC8Application1.elf" "XC8Application1.hex"
