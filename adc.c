//参考 adc.c
#include <adc.h>
#include "globals.h"

extern char ADC_RESULT;
extern char DAC_VALUE;
extern char workMode;
extern char WAVE_VALUE;
extern char OUTPUT_VALUE;

unsigned int channalSelect=0x0000;//DAC通道选择

void adc_init()
{
    P1ASF=0x08;     
    ADC_CONTR=ADC_INIT;
    delay(2);
}



void adc_start()		
{
	//启动AD转换
	ADC_CONTR=ADC_START;

    switch(workMode){
        case 1:

        break;
        case 2:

        break;
        case 3:

        break;
        default:break;
    }


}



void adc_work() interrupt 5
{

	ADC_CONTR=ADC_INIT;
	ADC_RESULT=ADC_RES/2+64;
}

void delay(int delayTime)
{
	unsigned int x;
	while(delayTime--){
	x=1000;
	while(x--);
	}
}