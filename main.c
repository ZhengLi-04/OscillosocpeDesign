#include <reg51.h>
#include <absacc.h>
#include <math.h>

#define ADC_INIT 0x83
#define ADC_START 0x8B
#define ADC_FLAG 0x10
#define ADC_BASE_ADDR 0x0000

#define DAC_CH2 0x2000
#define DAC_CH1 0x4000

#define SIN_BASE_ADDR 0x1C00
#define TRI_BASE_ADDR 0x1D00
#define SQU_BASE_ADDR 0x1E00
#define STW_BASE_ADDR 0x1F00

sbit D_SER     = P1 ^ 0;
sbit D_SRCLK   = P1 ^ 1;
sbit D_RCLK    = P1 ^ 2;
sbit KEY1      = P3 ^ 4;
sbit KEY2      = P3 ^ 5;
sbit EADC      = 0xAD;
sbit PADC      = 0xBD;

sfr CLK_DIV    = 0x97;
sfr ADC_CONTR  = 0xBC;
sfr ADC_RES    = 0xBD;
sfr P1ASF      = 0x9D;

/*-----------变量定义-----------*/
// 按键
unsigned char  	key_sta = 0, key_num;
// 显示
unsigned char  	dspbuf[4] = {0xef, 0xef, 0xef, 0xef}, sel = 0;
code unsigned char segCode[] =
{
    /* 0-9 */  0x11, 0x7d, 0x23, 0x29, 0x4d, 0x89, 0x81, 0x3d, 0x01, 0x09,
    /* 0-9. */ 0x10, 0x7c, 0x22, 0x28, 0x4c, 0x88, 0x80, 0x3c, 0x00, 0x08,
    /* U */    0x51,
    /* F */    0x87,
    /* - */    0xef,
    /* ntrq */ 0x15, 0xc3, 0xe7, 0x0d
}; // 显示码表
// 模式控制
unsigned char		workMode = 0;				// 0:初始 1:模式1 2:模式2 3:模式3
unsigned char 	mode1Status = 0; 		// 0:初始 1:循环显示 2:改波形 3:改幅度 4:改频率
unsigned char 	initStatus = 1; 		// 1:初始化状态，不得更新数码管内容
unsigned char 	updateAmpFlag = 0;	// 0:不可更新幅度 1:可以更新幅度
unsigned char 	updateFreFlag = 0;	// 0:不可更新频率 1:可以更新频率
// 地址
unsigned int    adAddr = ADC_BASE_ADDR;
unsigned int    daAddr = ADC_BASE_ADDR;
unsigned int    sinAddr = SIN_BASE_ADDR;
unsigned int   	triAddr = TRI_BASE_ADDR;
unsigned int  	squAddr = SQU_BASE_ADDR;
unsigned int 		stwAddr = STW_BASE_ADDR;
// 系统及A/D
unsigned int         clocktime = 0, adcount = 0;
unsigned char        ADC_RESULT = 0;
unsigned char        DAC_VALUE = 0;
// 模式1输出
unsigned char		outputWaveMode = 1;	// 1:正弦 2:三角 3:方波 4:锯齿
unsigned char   outputWaveValue = 0;// 输出值
unsigned char   outputFreq = 10;		// 设定输出频率
float        		outputAmp = 1.0;		// 设定输出幅值
// 模式2回放
unsigned char   mode2OutputValue = 0;
// 模式3测量
unsigned int    inputFreq = 0;			// 测定输入频率
float           inputAmp = 0.0;			// 测定输入幅值
unsigned char   inputAmp10x = 0;
unsigned char   outputAmp10x = 0;
// 临时测量值
unsigned char   amp = 0;
unsigned char   amp_last = 0;
unsigned char   amp_max = 128;
unsigned char   amp_min = 128;
int             fre = 0;
int             fre_up = 0;
int             fre_low = 0;
float           fre_count = 0;
unsigned int         ad_temp = 0;

/*-----------函数声明-----------*/
// 初始化
void init_all();
void init_timer0();
void init_interrupts();
void init_adc();
void adc_start();
// 按键函数
void keyService();
void keyWork();
// 显示函数
void dspTask();
void dspNum(unsigned char n, unsigned char m);
// 模式1信号发生函数
void init_outputWave();
void updateOutputWave();
// 模式3波形测量函数
void ampMeasure();
void freMeasure();
// 主函数、其他函数
void delay(int delayTime);
void main(void);

/*-----------初始化函数-----------*/
void init_timer0()
{
    TMOD &= 0XF0;
    TMOD |= 0X02;
    TL0 = 0X06;
    TH0 = 0X06;
    TR0 = 1;
}

void init_timer1()
{
    TMOD &= 0x0F;
    TMOD |= 0x20;
    TL1 = 0x06;
    TH1 = 0x06;
    TR1 = 1;
}

void init_interrupts()
{
    EA  = 1;
    ET0 = 1;
    ET1 = 1;
    EADC = 1;
    PT0 = 1;
    PT1 = 0;
    PADC = 0;
}

void init_adc()
{
    P1ASF = 0x08;
    ADC_CONTR = ADC_INIT;
    delay(2);
}

void init_all()
{
    CLK_DIV = CLK_DIV | 0x01;
    init_timer0();
    init_timer1();
    init_interrupts();
    init_adc();
    init_outputWave();
}


/*----------中断相关函数-----------*/
void timer_isr() interrupt 1
{
    EA = 0;
    adcount++;
    adc_start();
    if (adcount == 3)
    {
        updateOutputWave();
    }
    if (adcount == 5)
    {
        dspTask();
        keyService();
        adcount = 0;
    }
    EA = 1;
}

void updateFeature() interrupt 3
{
    EA = 0;
    clocktime++;
    if (workMode == 3)
    {
        ampMeasure();
        freMeasure();
    }
    if (workMode == 1)
    {
        daAddr = adAddr;
        if (adAddr > 0x1Bf0)
        {
            adAddr = ADC_BASE_ADDR;
        }
        XBYTE[adAddr] = (ADC_RESULT - 64) * 2;
        ad_temp = ADC_RESULT;
        adAddr++;
    }
    if (workMode == 2)
    {
        if (daAddr > 0x1Bf0)
        {
            daAddr = ADC_BASE_ADDR;
        }
        mode2OutputValue = XBYTE[daAddr] / 2;
        daAddr++;
    }
    if (workMode == 3)
    {
        daAddr = adAddr;
        if (adAddr > 0x0800)
        {
            adAddr = ADC_BASE_ADDR;
        }
        XBYTE[adAddr] = (ADC_RESULT - 64) * 2;
        ad_temp = ADC_RESULT;
        adAddr++;
    }
    if (clocktime == 4000)
    {
        clocktime = 0;
    }
    EA = 1;
}

void adc_work() interrupt 5
{
    ADC_CONTR = ADC_INIT;
    ADC_RESULT = ADC_RES / 2 + 64;
}

void adc_start()
{
    ADC_CONTR = ADC_START;
    switch (workMode)
    {
    case 1:
    {
        XBYTE[DAC_CH1] = DAC_VALUE;
        XBYTE[DAC_CH2] = outputWaveValue;
    }
    break;
    case 2:
    {
        XBYTE[DAC_CH1] = DAC_VALUE;
        XBYTE[DAC_CH2] = mode2OutputValue;
    }
    break;
    case 3:
    {
        XBYTE[DAC_CH1] = DAC_VALUE;
        XBYTE[DAC_CH2] = 0x00;
    }
    break;
    default:
        break;
    }
}


/*----------按键相关函数-----------*/
// 按键扫描
void keyService()
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
// 根据不同按下的键执行不同动作
void keyWork()
{
    unsigned int i, j;
    switch (key_num)
    {
    case 1:
    case 2:
    case 3:
        workMode = key_num;
        initStatus = 1;
        for (i = 0; i < 1000; i++)
        {
            for (j = 0; j < 15; j++)
            {
                dspNum(22, 0);
                dspNum(workMode, 1);
                dspNum(22, 2);
                dspNum(22, 3);
            }
        }
        if (workMode == 1) mode1Status = 1;
        initStatus = 0;
        break;
    case 4:
        if (workMode == 1)
        {
            if (mode1Status == 4)
                mode1Status = 1;
            else
                mode1Status = mode1Status + 1;
        }
        delay(100);
        break;
    case 5:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                outputWaveMode = 1;
            else if (mode1Status == 3 && outputAmp <= 4)
                outputAmp = outputAmp + 1;
            else if (mode1Status == 4 && outputFreq <= 990)
                outputFreq = outputFreq + 10;
        }
        delay(100);
        break;
    case 6:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                outputWaveMode = 2;
            else if (mode1Status == 3 && outputAmp >= 2)
                outputAmp = outputAmp - 1;
            else if (mode1Status == 4 && outputFreq >= 20)
                outputFreq = outputFreq - 10;
        }
        delay(100);
        break;
    case 7:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                outputWaveMode = 3;
            else if (mode1Status == 3 && outputAmp <= 4.9)
                outputAmp = outputAmp + 0.1;
            else if (mode1Status == 4 && outputFreq <= 999.8)
                outputFreq = outputFreq + 1;
        }
        delay(100);
        break;
    case 8:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                outputWaveMode = 4;
            else if (mode1Status == 3 && outputAmp >= 0.2)
                outputAmp = outputAmp - 0.1;
            else if (mode1Status == 4 && outputFreq >= 2)
                outputFreq = outputFreq - 1;
        }
        delay(100);
        break;
    default:
        break;
    }
}

/*----------显示相关函数-----------*/
//根据段码表显示不同值
void dspNum(unsigned char n, unsigned char m)
{
    dspbuf[m] = (n < sizeof(segCode)) ? segCode[n] : 0x11;
}

// 数码管及led更新
void dspTask()
{
    unsigned char i;
    unsigned char a, b;
    switch (sel)
    {
    case 0:
        a = 0x01;
        break;
    case 1:
        a = 0x02;
        break;
    case 2:
        a = 0x04;
        break;
    case 3:
        a = 0x08;
        break;
    default:
        a = 0x10;
        break;
    }
    for (b = 0x80, i = 0; i < 8; i++)
    {
        if (a & b)  D_SER = 1;
        else     D_SER = 0;
        D_SRCLK = 0;
        D_SRCLK = 1;
        D_SRCLK = 0;
        b = b >> 1;
        b = b & 0x7f;
    }
    if (sel <= 3)
    {
        a = dspbuf[sel];
    }
    else
    {
        switch (workMode)
        {
        case 1:
            a = 0xF7;
            break;
        case 2:
            a = 0xFB;
            break;
        case 3:
            a = 0xFD;
            break;
        default:
            a = 0xFE;
            break;
        }
    }
    if (sel <= 3)
        key_num = sel;
    sel++;
    if (sel > 4) sel = 0;
    for (b = 0x80, i = 0; i < 8; i++)
    {
        if (a & b)  D_SER = 1;
        else     D_SER = 0;
        D_SRCLK = 0;
        D_SRCLK = 1;
        D_SRCLK = 0;
        b = b >> 1;
        b = b & 0x7fff;
    }
    D_RCLK = 0;
    D_RCLK = 1;
    D_RCLK = 0;
}

/*----------模式1函数-----------*/
// 初始化标准波形
void init_outputWave()
{
    unsigned int addr = 0;
    unsigned int i = 0;
    //Sin Wave
    i = 0;
    addr = SIN_BASE_ADDR;
    for (; addr <= 0x1CFF; addr++, i++)
    {
        XBYTE[addr] = floor(14 * (sin(3.14 * i / 128) +1)) + 32; //14是根据硬件调整的经验值
    }
    //Triangular Wave
    i = 0;
    addr = TRI_BASE_ADDR;
    for (; addr <= 0x1D7F; addr++, i++)
    {
        XBYTE[addr] = 49 + floor(30 * (i / 128.0));
    }
    i = 0;
    addr = 0x1D80;
    for (; addr <= 0x1DFF; addr++, i++)
    {
        XBYTE[addr] = 79 - floor(30 * (i / 128.0));
    }
    //Square Wave
    addr = SQU_BASE_ADDR;
    for (; addr <= 0x1E7F; addr++)
    {
        XBYTE[addr] = 64 + 15;
    }
    addr = 0x1E80;
    for (; addr <= 0x1EFF; addr++)
    {
        XBYTE[addr] = 64 - 15;
    }
    //Sawtooth Wave
    i = 0;
    addr = STW_BASE_ADDR;
    for (; addr <= 0x1FFF; addr++, i++)
    {
        XBYTE[addr] = 64 - 15 + floor(30 * i / 256);
    }
}

// 根据设定值更新波形
void updateOutputWave()
{
    if (workMode == 1)
    {
        switch (outputWaveMode)
        {
        case 1:
        {
            if (sinAddr > 0x1CFF) sinAddr = SIN_BASE_ADDR;
            outputWaveValue = (XBYTE[sinAddr] - 32) * outputAmp + 32;
            sinAddr = sinAddr + 1 + outputFreq / 1.6;
        }
        break;
        case 2:
        {
            if (triAddr > 0x1DF3) triAddr = TRI_BASE_ADDR;
            outputWaveValue = (XBYTE[triAddr] - 64) * outputAmp + 64;
            triAddr = triAddr + 1 + outputFreq / 1.6;
        }
        break;
        case 3:
        {
            if (squAddr > 0x1EFF) squAddr = SQU_BASE_ADDR;
            outputWaveValue = (XBYTE[squAddr] - 64) * outputAmp + 64;
            squAddr = squAddr + 1 + outputFreq / 1.6;
        }
        break;
        case 4:
        {
            if (stwAddr > 0x1FFF) stwAddr = STW_BASE_ADDR;
            outputWaveValue = (XBYTE[stwAddr] - 64) * outputAmp + 64;
            stwAddr = stwAddr + 1 + outputFreq / 1.6 ;
        }
        break;
        default:
            break;
        }
    }
}
/*----------模式3函数-----------*/
// 幅值测量函数
void ampMeasure()
{
    amp = ADC_RESULT;
    if (amp > amp_max)
    {
        amp_max = amp;
    }
    if (amp < amp_min)
    {
        amp_min = amp;
    }
    if (adAddr > 0x0800)
    {
        inputAmp = (amp_max - amp_min) * 5.0 / 1.1 / 128;
        amp_max = amp_min = 128;
    }
}
//频率测量函数
void freMeasure()
{
    amp = ADC_RESULT;
    if (amp > 128 && amp_last <= 128)
    {
        fre_up = adAddr;
        if (fre_low != 0)
        {
            fre = fre + fabs(fre_low - fre_up);
            fre_count++;
        }
        fre_low = fre_up;
    }
    if (adAddr > 0x0800)
    {
        inputFreq = floor(2000 / (fre * 1.0 / fre_count));
        fre = 0;
        fre_up = fre_low = 0;
        fre_count = 0;
        amp = amp_last = 129;
    }
    amp_last = amp;
}

/*----------其他函数-----------*/
// 延时模块
void delay(int delayTime)
{
    unsigned int x;
    while (delayTime--)
    {
        x = 1000;
        while (x--);
    }
}
/*----------主函数-----------*/
void main(void)
{
    init_all();
    for (;;)
    {
        switch (workMode)
        {
        case 0:
        {
            initStatus = 1;
            dspNum(22, 0);
            dspNum(22, 1);
            dspNum(22, 2);
            dspNum(22, 3);
        }
        break;
        case 1:
        {
            DAC_VALUE = ADC_RESULT;
            if (initStatus == 0)
            {
                if (mode1Status == 1)
                {
                    if (clocktime < 2000)
                    {
                        dspNum(21, 0);
                        if (outputFreq > 999) outputFreq = 999;
                        if (outputFreq <= 0) outputFreq = 0;
                        dspNum((outputFreq / 100) % 10, 1);
                        dspNum((outputFreq / 10) % 10, 2);
                        dspNum(outputFreq % 10, 3);
                    }
                    else
                    {
                        outputAmp10x = (int)(outputAmp * 10);
                        dspNum(20, 0);
                        dspNum((outputAmp10x / 100) % 10, 1);
                        dspNum((outputAmp10x / 10) % 10 + 10, 2);
                        dspNum((outputAmp10x / 1) % 10, 3);
                    }
                }
                else if (mode1Status == 2)
                {
                    if (outputWaveMode == 1)
                    {
                        dspNum(22, 0);
                        dspNum(5, 1);
                        dspNum(1, 2);
                        dspNum(23, 3);
                    }
                    else if (outputWaveMode == 2)
                    {
                        dspNum(22, 0);
                        dspNum(24, 1);
                        dspNum(25, 2);
                        dspNum(1, 3);
                    }
                    else if (outputWaveMode == 3)
                    {
                        dspNum(22, 0);
                        dspNum(5, 1);
                        dspNum(26, 2);
                        dspNum(20, 3);
                    }
                    else if (outputWaveMode == 4)
                    {
                        dspNum(22, 0);
                        dspNum(5, 1);
                        dspNum(24, 2);
                        dspNum(22, 3);
                    }
                }
                else if (mode1Status == 3)
                {
                    outputAmp10x = (int)(outputAmp * 10);
                    dspNum(20, 0);
                    dspNum((outputAmp10x / 100) % 10, 1);
                    dspNum((outputAmp10x / 10) % 10 + 10, 2);
                    dspNum((outputAmp10x / 1) % 10, 3);
                }
                else if (mode1Status == 4)
                {
                    dspNum(21, 0);
                    if (outputFreq > 999) outputFreq = 999;
                    if (outputFreq <= 0) outputFreq = 0;
                    dspNum((outputFreq / 100) % 10, 1);
                    dspNum((outputFreq / 10) % 10, 2);
                    dspNum(outputFreq % 10, 3);
                }
            }
        }
        break;
        case 2:
        {
            DAC_VALUE = ADC_RESULT;
        }
        break;
        case 3:
        {
            DAC_VALUE = ADC_RESULT;
            if (initStatus == 0)
            {
                if (clocktime < 2000)
                {
                    updateAmpFlag = 1;
                    if (updateFreFlag == 1)
                    {
                        updateFreFlag = 0;
                        dspNum(21, 0);
                        if (inputFreq > 999)inputFreq = 999;
                        if (inputFreq <= 0)inputFreq = 0;
                        dspNum((inputFreq / 100) % 10, 1);
                        dspNum((inputFreq / 10) % 10, 2);
                        dspNum(inputFreq % 10, 3);
                    }

                }
                else
                {
                    updateFreFlag = 1;
                    if (updateAmpFlag == 1)
                    {
                        updateAmpFlag = 0;
                        inputAmp10x = (int)(inputAmp * 10);
                        dspNum(20, 0);
                        dspNum((inputAmp10x / 100) % 10, 1);
                        dspNum((inputAmp10x / 10) % 10 + 10, 2);
                        dspNum((inputAmp10x / 1) % 10, 3);
                    }

                }
            }
        }
        break;
        default:
            break;
        }
        if (key_sta & 0x01)
        {
            keyWork();
            key_sta = key_sta & 0xfe;
        }

    }
}

