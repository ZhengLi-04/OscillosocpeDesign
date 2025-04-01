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
	init_timer0();      //初始化定时器0
  init_timer1();			//初始化定时器1
  init_special_interrupts();     //设置中断
	
	for(;;) // 无限循环
    { 
        if (key_sta & 0x01) // 如果有按键被按下
        { 
            fdisp(key_num,1); // 显示按键编号-》修改
            key_sta = key_sta & 0xfe; // 释放
        }
    }
		
}