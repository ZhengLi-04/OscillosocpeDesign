#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "global.h"

unsigned char ledbuf[4] = {0x11, 0x11, 0x11, 0x11}; //»º³åÇø
unsigned char dspbuf[4] = {0xef, 0xef, 0xef, 0xef};
unsigned char sel = 0, key_sta = 0, key_num = 0;
SystemMode workMode = MODE_IDLE;

int freq = 153;
float vpp = 2.5;
unsigned char amp = 0;
unsigned int clocktime = 0, adcount = 0;


