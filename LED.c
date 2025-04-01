#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "led.h"
#include "global.h"

void dsptask()
{
    unsigned char i;
    unsigned char a, b;

    switch (sel)
    {
    case 0:
        a = 0x01;
        break;
    case 1:
        a = 0x02;
        break;
    case 2:
        a = 0x04;
        break;
    default:
        a = 0x08;
    }

    for (b = 0x80, i = 0; i < 8; i++)
    {
        D_SER = (a & b) ? 1 : 0;
        D_SRCLK = 0;
        D_SRCLK = 1;
        b = b >> 1;
    }

    a = dspbuf[sel];
    key_num = sel;
    sel++;
    if (sel >= 4) sel = 0;

    for (b = 0x80, i = 0; i < 8; i++)
    {
        D_SER = (a & b) ? 1 : 0;
        D_SRCLK = 0;
        D_SRCLK = 1;
        b = b >> 1;
    }

    D_RCLK = 0;
    D_RCLK = 1;
}

void fdisp(unsigned char n, unsigned char m)
{
    char  c;
    switch (n)
    {
    case 0:
        c = 0x11;
        break;
    case 1:
        c = 0x7d;
        break;
    case 2:
        c = 0x23;
        break;
    case 3:
        c = 0x29;
        break;
    case 4:
        c = 0x4d;
        break;
    case 5:
        c = 0x89;
        break;
    case 6:
        c = 0x81;
        break;
    case 7:
        c = 0x3d;
        break;
    case 8:
        c = 0x01;
        break;
    default:
        c = 0x09;
    }
    dspbuf[m] = c;
}