#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "led.h"
#include "global.h"

//void dsptask()
//{
//    unsigned char i;
//    unsigned char a, b;

//    switch (sel)
//    {
//    case 0:
//        a = 0x01;
//        break;
//    case 1:
//        a = 0x02;
//        break;
//    case 2:
//        a = 0x04;
//        break;
//    default:
//        a = 0x08;
//    }

//    for (b = 0x80, i = 0; i < 8; i++)
//    {
//        D_SER = (a & b) ? 1 : 0;
//        D_SRCLK = 0;
//        D_SRCLK = 1;
//        b = b >> 1;
//    }

//    a = dspbuf[sel];
//    key_num = sel;
//    sel++;
//    if (sel >= 4) sel = 0;

//    for (b = 0x80, i = 0; i < 8; i++)
//    {
//        D_SER = (a & b) ? 1 : 0;
//        D_SRCLK = 0;
//        D_SRCLK = 1;
//        b = b >> 1;
//    }

//    D_RCLK = 0;
//    D_RCLK = 1;
//}

void dsptask()
{
    unsigned char i;
    unsigned char a, b;

    // 第一部分：位选信号
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
    case 3:
        a = 0x08;
        break;
    case 4:
        a = 0x10;
        break;
    }

    // 位移输出位选信号
    for (b = 0x80, i = 0; i < 8; i++)
    {
        D_SER = (a & b) ? 1 : 0;
        D_SRCLK = 0;
        D_SRCLK = 1;
        b >>= 1;
    }

    // 第二部分：段选信号生成
    if (sel < 4)
    {
        // 正常数码管显示
        a = dspbuf[sel];
        key_num = sel;
    }
    else
    {
        // LED模式控制
        switch (workMode)
        {
        case MODE_1:
            a = 0xF7;
            break;
        case MODE_2:
            a = 0xFB;
            break;
        case MODE_3:
            a = 0xFD;
            break;
        default:
            a = 0xFF;
        }
    }

    // 位移输出段选信号
    for (b = 0x80, i = 0; i < 8; i++)
    {
        D_SER = (a & b) ? 1 : 0;
        D_SRCLK = 0;
        D_SRCLK = 1;
        b >>= 1;
    }

    // 锁存输出
    D_RCLK = 0;
    D_RCLK = 1;
    D_RCLK = 0;

    // 循环控制（数码管0-3，LED控制4）
    sel++;
    if (sel > 4) sel = 0;
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
    case 9:
        c = 0x09;
        break;
    case 10:
        c = 0x10;
        break;
    case 11:
        c = 0x7c;
        break;
    case 12:
        c = 0x22;
        break;
    case 13:
        c = 0x28;
        break;
    case 14:
        c = 0x4c;
        break;
    case 15:
        c = 0x88;
        break;
    case 16:
        c = 0x80;
        break;
    case 17:
        c = 0x3c;
        break;
    case 18:
        c = 0x00;
        break;
    case 19:
        c = 0x08;
        break;
    case 20:  // ???"U"??
        c = 0x51;  // ????:g???,???
        break;
    case 21:  // ???"F"??
        c = 0x87;  // ??:a+g??
        break;
		case 22:  // -
        c = 0xef;  // -
        break;
    default:
        c = 0xef;
    }
    dspbuf[m] = c;
}