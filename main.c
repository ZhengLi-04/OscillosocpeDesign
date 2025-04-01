#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "global.h"
#include "init.h"
#include "key.h"
#include "LED.h"

void main(void)
{
	init_timer0();      //��ʼ����ʱ��0
  init_timer1();			//��ʼ����ʱ��1
  init_special_interrupts();     //�����ж�
	
	for(;;) // ����ѭ��
    { 
        if (key_sta & 0x01) // ����а���������
        { 
            fdisp(key_num,1); // ��ʾ�������-���޸�
            key_sta = key_sta & 0xfe; // �ͷ�
        }
    }
		
}