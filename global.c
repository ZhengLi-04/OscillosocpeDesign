#include "global.h"

unsigned char ledbuf[4] = {0x11, 0x11, 0x11, 0x11}; //»º³åÇø
unsigned char dspbuf[4] = {0xef, 0xef, 0xef, 0xef};
unsigned char sel = 0, key_sta = 0, key_num = 0;

unsigned int         clocktime = 0, adcount = 0;