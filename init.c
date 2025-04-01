#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "init.h"
#include "global.h"

//  ��ʱ��0��ʼ
void init_timer0(void)              
{
  TMOD &= 0XF0;
  TMOD |= 0X02;	   //8λ���ض�ʱ��t0
  TL0 = 0X06;
  TH0 = 0X06;	   //һ����������0.25ms
  TR0 = 1;
}

//  ��ʱ��1��ʼ
void init_timer1(void)
{
  TMOD &= 0x0F;
  TMOD |= 0x20;
  TL1 = 0x06;
  TH1 = 0x06;
  TR1 = 1;
}

//  �ж�����
void init_special_interrupts(void)   
{ 
  EA  = 1;		//���ж�����
  ET0 = 1;		//����T0�ж�
  ET1 = 1;    //����T1�ж�
  EADC = 1;		//����AD�ж�
  //�ж����ȼ�����
  PT0 = 1;
  PT1 = 0;
  PADC = 0;
}

void timer_isr() interrupt 1
{
    EA = 0; // ????????
    adcount++;

    if (adcount == 20) // ??5ms ????�??
    {
        adcount = 0;
        dsptask();     // ????????
        key_service(); // ??????
        clocktime++;   // ???
    }

    EA = 1; // ????�??�????
}

