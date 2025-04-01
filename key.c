#include <reg51.h>
#include <math.h>
#include <absacc.h>
#include <intrins.h>

#include "key.h"
#include "global.h"

void key_service()
{
    if (key_sta & 0x01) return;
    if (KEY2)
    {
        key_num = key_num + 1;
        key_sta = key_sta | 0x01;
    }
    else if (KEY1)
    {
        key_num = key_num + 5;
        key_sta = key_sta | 0x01;
    }
}