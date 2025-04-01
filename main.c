#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "global.h"
#include "init.h"
#include "key.h"
#include "LED.h"
#include "mode.h"

void main(void)
{
    unsigned char key_num_now = 0; // ����Ĭ��

    init_timer0();      //��ʼ����ʱ��0
    init_timer1();			//��ʼ����ʱ��1
    init_special_interrupts();     //�����ж�

    // ��ʼ����ʾ----
    fdisp(10, 0);
    fdisp(10, 1);
    fdisp(10, 2);
    fdisp(10, 3);

    for (;;)
    {
        if (key_sta & 0x01)
        {
            key_num_now = key_num;
            key_sta &= 0xFE;  // ??????
        }

        // ??????
        switch (key_num_now)
        {
        case 1:
            workMode = MODE_1;
            Mode1_Process();
            break;
        case 2:
            workMode = MODE_2;
            Mode2_Process();
            break;
        case 3:
            workMode = MODE_3;
            Mode3_Process();
            break;

        default:
            break;
        }
    }

}