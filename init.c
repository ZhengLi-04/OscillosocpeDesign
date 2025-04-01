#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "init.h"
#include "global.h"

//  ¶¨Ê±Æ÷0³õÊ¼
void init_timer0(void)              
{
  TMOD &= 0XF0;
  TMOD |= 0X02;	   //8Î»ÖØÔØ¶¨Ê±Æ÷t0
  TL0 = 0X06;
  TH0 = 0X06;	   //Ò»¸ö¼ÆÊýÖÜÆÚ0.25ms
  TR0 = 1;
}

//  ¶¨Ê±Æ÷1³õÊ¼
void init_timer1(void)
{
  TMOD &= 0x0F;
  TMOD |= 0x20;
  TL1 = 0x06;
  TH1 = 0x06;
  TR1 = 1;
}

//  ÖÐ¶ÏÉèÖÃ
void init_special_interrupts(void)   
{ 
  EA  = 1;		//×ÜÖÐ¶ÏÔÊÐí
  ET0 = 1;		//ÔÊÐíT0ÖÐ¶Ï
  ET1 = 1;    //ÔÊÐíT1ÖÐ¶Ï
  EADC = 1;		//ÔÊÐíADÖÐ¶Ï
  //ÖÐ¶ÏÓÅÏÈ¼¶ÅäÖÃ
  PT0 = 1;
  PT1 = 0;
  PADC = 0;
}

void timer_isr() interrupt 1
{
    EA = 0; // ????????
    adcount++;

    if (adcount == 20) // ??5ms ????€??
    {
        adcount = 0;
        dsptask();     // ????????
        key_service(); // ??????
        clocktime++;   // ???
    }

    EA = 1; // ????€??€????
}

