/* global.h */
#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <reg51.h>

sbit EADC   = 0xAD;  // ADCʹ��λ
sbit PADC   = 0xBD;  // ADC�ж����ȼ�����λ
sbit KEY1   = P3^4;  // ����1
sbit KEY2   = P3^5;  // ����2

sbit D_SER = P1^0;   // 
sbit D_SRCLK = P1^1; // 
sbit D_RCLK = P1^2;  //

extern unsigned char ledbuffer[4];
extern unsigned char dspbuf[4];
extern unsigned char key_num;
extern unsigned char key_sta;
extern unsigned char sel;

extern unsigned int clocktime;
extern unsigned int adcount;

void DelayMs(unsigned int ms);  //��ʱ

// ģʽѡ��
typedef enum {
    MODE_IDLE = 0, 
    MODE_1,        
    MODE_2,       
    MODE_3          
} SystemMode;

extern SystemMode workMode;  

#endif
