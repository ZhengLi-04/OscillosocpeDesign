#ifndef __adc_H__
#define __adc_H__

#include <reg51.h>                              
#include <absacc.h>
#include <math.h>

sfr ADC_CONTR=0xBC;	 //ADC控制寄存器
sfr ADC_RES =0xBD;	//ADC结果寄存器高八位
sfr ADC_RESL=0xBE;	//ADC结果寄存器低2位
sfr P1ASF=0x9D;		//P1口用途寄存器


#define CH2 0x2000;
#define CH1 0x4000;


#define ADC_INIT 0x83	 //ADC_FLAG置0时候的状态
#define ADC_START 0x8B
#define ADC_FLAG 0x10		//清零ADC中断标志位


void adc_init();
void adc_start();

void delay(int delayTime);//单位延时为5ms

#endif