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
    unsigned char key_num_now = 0; // 开机默认

    init_timer0();      //初始化定时器0
    init_timer1();			//初始化定时器1
    init_special_interrupts();     //设置中断

    // 初始化显示----
    fdisp(22, 0);
    fdisp(22, 1);
    fdisp(22, 2);
    fdisp(22, 3);

    // 无限循环
    for (;;)
    {
        if (key_sta & 0x01)
        {
            key_num_now = key_num;
            key_sta &= 0xFE;  // 清除状态
            if (key_num_now == 3)
            {
                ResetMode3State();
            }
        }

        // 设置工作模式
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