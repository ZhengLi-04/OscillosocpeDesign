/* mode.c */
#include <reg51.h>
#include <math.h>

#include "mode.h"
#include "global.h"
#include "led.h"

// 模式3显示状态机
typedef enum
{
    MODE3_INIT,     // 初始提示
    MODE3_FREQ,     // 显示频率
    MODE3_VOLT      // 显示电压
} Mode3State;


static Mode3State mode3State = MODE3_INIT;
static unsigned long mode3Timer = 0;

void Mode1_Process(void)
{
    fdisp(22, 0);
    fdisp(1, 1);
    fdisp(22, 2);
    fdisp(22, 3);
}

void Mode2_Process(void)
{
    fdisp(22, 0);
    fdisp(2, 1);
    fdisp(22, 2);
    fdisp(22, 3);
}


void Mode3_Process(void)
{
    // 调用测量函数（在中断中周期性执行）
    //ampMeasure();
    //freMeasure();


    switch (mode3State)
    {
    case MODE3_INIT:  
        fdisp(22, 0);  // "-"
        fdisp(3, 1);    // "3"
        fdisp(22, 2);   // "-"
        fdisp(22, 3);   // "-"

        if (clocktime - mode3Timer >= 200)   // ?1?(??clocktime?5ms??)
        {
            mode3State = MODE3_FREQ;
            mode3Timer = clocktime;
        }
        break;

    case MODE3_FREQ:   // ????
        UpdateFrequencyDisplay(freq);
        if (clocktime - mode3Timer >= 400)   // 2???
        {
            mode3State = MODE3_VOLT;
            mode3Timer = clocktime;
        }
        break;

    case MODE3_VOLT:   // ????
        UpdateVoltageDisplay(vpp);
        if (clocktime - mode3Timer >= 400)
        {
            mode3State = MODE3_FREQ;
            mode3Timer = clocktime;
        }
        break;
    }
}

void ResetMode3State(void) {
    mode3State = MODE3_INIT;
    mode3Timer = clocktime; 
}

// ????(??:FXXX)
void UpdateFrequencyDisplay(int freq)
{
    fdisp(21, 0);  // ??"F"

    freq = (freq > 999) ? 999 : freq;  // ??
    fdisp((freq / 100) % 10, 1); // ??
    fdisp((freq / 10) % 10, 2); // ??
    fdisp((freq / 1) % 10, 3); // ??
}

// ????(??:UXX.X)
void UpdateVoltageDisplay(float vpp)
{
    int value = (int)(vpp * 10);

    fdisp(20, 0);  // ???"U"??(???3????)

    fdisp((value / 100) % 10, 1); // ??
    fdisp((value / 10) % 10 + 10, 2); // ??
    fdisp((value / 1) % 10, 3); // ??
}
