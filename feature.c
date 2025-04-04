//�ο� feature.c
#include <feature.h>
#include "global.h"

extern char ADC_RESULT;
extern int adAddress;


extern int freq;        // ?????(??:Hz)
extern float vpp;       // ?????(??:V)
extern unsigned char amp;  // ?????
unsigned char ampl=0;//ǰһ�εĲ�������ֵ

unsigned char amp_up=128;
unsigned char amp_low=128;//
int fre=0;//�������ڵĲ������ܸ���
int fre_up=0;//���ڵ�ĩ��ַ
int fre_low=0;//���ڵ��׵�ַ
float fre_count=0;//�Ѿ����������ڸ���

void ampMeasure()
{
	if(workMode == MODE_3)
	{
    amp=ADC_RESULT;
    if (amp>amp_up){amp_up=amp;}
    if (amp<amp_low){amp_low=amp;}

    if(adAddress>0x0400){
        vpp=(amp_up*5.0-amp_low*5.0)/128;
        amp_up=amp_low=128;
    }
	}
	else
	{
	vpp=0;
	freq=0;}
}

void freMeasure()
{
		if(workMode == MODE_3)
	{
    amp=ADC_RESULT;

    if (amp>128 && ampl<=128)
    {
    fre_up=adAddress; 
    if(fre_low!=0)	  
    {
    fre=fre+fabs(fre_low-fre_up);
    fre_count++;
    }
    fre_low=fre_up;	//����
    }

    if(adAddress>0x0400){

        freq=floor(2000/(fre*1.0/fre_count));
        fre=0;
        fre_up=fre_low=0;
        fre_count=0;
        amp=ampl=129;
    }
    ampl=amp;
	}else
	{
	vpp=0;
	freq=0;}
}