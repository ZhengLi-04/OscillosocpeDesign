#ifndef __adc_H__
#define __adc_H__

#include <reg51.h>                              
#include <absacc.h>
#include <math.h>

sfr ADC_CONTR=0xBC;	 //ADC���ƼĴ���
sfr ADC_RES =0xBD;	//ADC����Ĵ����߰�λ
sfr ADC_RESL=0xBE;	//ADC����Ĵ�����2λ
sfr P1ASF=0x9D;		//P1����;�Ĵ���


#define CH2 0x2000;
#define CH1 0x4000;


#define ADC_INIT 0x83	 //ADC_FLAG��0ʱ���״̬
#define ADC_START 0x8B
#define ADC_FLAG 0x10		//����ADC�жϱ�־λ


void adc_init();
void adc_start();

void delay(int delayTime);//��λ��ʱΪ5ms

#endif