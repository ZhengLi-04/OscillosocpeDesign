#include <reg51.h>
#include<absacc.h>
#include<math.h>

#define ADC_BASE_ADDR 0x0000
#define DAC_CH2 0x2000
#define DAC_CH1 0x4000
#define ADC_INIT 0x83
#define ADC_START 0x8B
#define ADC_FLAG 0x10
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

unsigned char updateAmpFlag = 0;
unsigned char updateFreFlag = 0;
unsigned char mode1Status = 0;
//mode1status = 0初始 1循环显示 2波形 3改幅度 4该频率
unsigned int         ad_temp = 0;
unsigned char        dspbuf[4] = {0xef, 0xef, 0xef, 0xef}, sel = 0;
unsigned int         clocktime = 0, adcount = 0;
unsigned char        ADC_RESULT = 0;
unsigned char        DAC_VALUE = 0;
unsigned char        OUTPUT_VALUE = 0;
unsigned int         adAddr = ADC_BASE_ADDR;
unsigned int         daAddr = ADC_BASE_ADDR;
unsigned int         sinAddr = SIN_BASE_ADDR;
unsigned int         triAddr = TRI_BASE_ADDR;
unsigned int         squAddr = SQU_BASE_ADDR;
unsigned int         stwAddr = STW_BASE_ADDR;
unsigned char        key_sta = 0, key_num;
unsigned char        workMode = 0;
unsigned char        outputWaveMode = 1;
unsigned char        outputWaveValue = 0;
unsigned char        outputFreq = 10;
float        					outputAmp = 1.0;
unsigned int        inputFreq = 0;
float               inputAmp = 0.0;
unsigned char        value = 0;
unsigned char        valueBuffer = 0;
unsigned char        amp = 0;
unsigned char        amp_last = 0;
unsigned char        amp_up = 128;
unsigned char        amp_low = 128;
int                  fre = 0;
int                  fre_up = 0;
int                  fre_low = 0;
float                fre_count = 0;
unsigned char initStatus = 1; //1代表为初始化状态，不更新数码管内容。

void init_timer0();
void init_interrupts();
void updateWaveBuffer();
void dsptask();
//void timer_isr() interrupt 1;
//void updateFeature() interrupt 3;
void fdisp(unsigned char n, unsigned char m);
void main(void);
void init_adc();
void adc_start();
//void adc_work() interrupt 5;
void delay(int delayTime);
void init_outputWave();
void key_service();
void keyWork();
void ampMeasure();
void freMeasure();

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

void dsptask()
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

void timer_isr() interrupt 1
{
    EA = 0;
    adcount++;
    adc_start();
    if (adcount == 3)
    {
        updateWaveBuffer();
    }
    if (adcount == 5)
    {
        dsptask();
        key_service();
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
        OUTPUT_VALUE = XBYTE[daAddr] / 2;
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

void updateWaveBuffer()
{
    if (workMode == 1)
    {
        switch (outputWaveMode)
        {
        case 1:
        {
            if (sinAddr <= 0x1CFF)
            {
                if (outputAmp != 1)
                {
                    outputWaveValue = (XBYTE[sinAddr] - 32) * outputAmp + 32;
                }
                else
                {
                    outputWaveValue = XBYTE[sinAddr];
                }
                sinAddr = sinAddr + 1 + outputFreq / 1.6;
            }
            else
            {
                sinAddr = SIN_BASE_ADDR;
                outputWaveValue = (XBYTE[sinAddr] - 32) * outputAmp + 32;
                sinAddr = sinAddr + 1 + outputFreq / 1.6;
            }
        }
        break;
        case 2:
        {
            if (triAddr <= 0x1DF3)
            {
                outputWaveValue = (XBYTE[triAddr] - 64) * outputAmp + 64;
                triAddr = triAddr + 1 + outputFreq / 1.6;
            }
            else
            {
                triAddr = TRI_BASE_ADDR;
                outputWaveValue = (XBYTE[triAddr] - 64) * outputAmp + 64;
                triAddr = triAddr + 1 + outputFreq / 1.6;
            }
        }
        break;
        case 3:
        {
            if (squAddr <= 0x1EFF)
            {
                outputWaveValue = (XBYTE[squAddr] - 64) * outputAmp + 64;
                squAddr = squAddr + 1 + outputFreq / 1.6;
            }
            else
            {
                squAddr = SQU_BASE_ADDR;
                outputWaveValue = (XBYTE[squAddr] - 64) * outputAmp + 64;
                squAddr = squAddr + 1 + outputFreq / 1.6;
            }
        }
        break;
        case 4:
        {
            if (stwAddr <= 0x1FFF)
            {
                outputWaveValue = (XBYTE[stwAddr] - 64) * outputAmp + 64;
                stwAddr = stwAddr + 1 + outputFreq / 1.6 ;
            }
            else
            {
                stwAddr = STW_BASE_ADDR;
                outputWaveValue = (XBYTE[stwAddr] - 64) * outputAmp + 64;
                stwAddr = stwAddr + 1 + outputFreq / 1.6 ;
            }
        }
        break;
        default:
            break;
        }
    }
}

code unsigned char segCode[] =
{
    /* 0-9 */  0x11, 0x7d, 0x23, 0x29, 0x4d, 0x89, 0x81, 0x3d, 0x01, 0x09,
    /* 0-9. */ 0x10, 0x7c, 0x22, 0x28, 0x4c, 0x88, 0x80, 0x3c, 0x00, 0x08,
    /* U */    0x51,
    /* F */    0x87,
    /* - */    0xef,
    /* ntrq */ 0x15, 0xc3, 0xe7, 0x0d
};

void fdisp(unsigned char n, unsigned char m)
{
    dspbuf[m] = (n < sizeof(segCode)) ? segCode[n] : 0x11;
}

void main(void)
{

    CLK_DIV = CLK_DIV | 0x01;
    init_timer0();
    init_timer1();
    init_interrupts();
    init_adc();
    init_outputWave();
    for (;;)
    {
        switch (workMode)
        {
        case 0:
        {
            initStatus = 1;
            fdisp(22, 0);
            fdisp(22, 1);
            fdisp(22, 2);
            fdisp(22, 3);
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
                        fdisp(21, 0);
                        if (outputFreq > 999) outputFreq = 999;
                        if (outputFreq <= 0) outputFreq = 0;
                        fdisp((outputFreq / 100) % 10, 1);
                        fdisp((outputFreq / 10) % 10, 2);
                        fdisp(outputFreq % 10, 3);
                    }
                    else
                    {
                        valueBuffer = (int)(outputAmp * 10);

                        fdisp(20, 0);
                        fdisp((valueBuffer / 100) % 10, 1);
                        fdisp((valueBuffer / 10) % 10 + 10, 2);
                        fdisp((valueBuffer / 1) % 10, 3);
                    }
                }
                else if (mode1Status == 2)
                {
                    if (outputWaveMode == 1)
                    {
                        fdisp(22, 0);
                        fdisp(5, 1);
                        fdisp(1, 2);
                        fdisp(23, 3);
                    }
                    else if (outputWaveMode == 2)
                    {
                        fdisp(22, 0);
                        fdisp(24, 1);
                        fdisp(25, 2);
                        fdisp(1, 3);
                    }
                    else if (outputWaveMode == 3)
                    {
                        fdisp(22, 0);
                        fdisp(5, 1);
                        fdisp(26, 2);
                        fdisp(20, 3);
                    }
                    else if (outputWaveMode == 4)
                    {
                        fdisp(22, 0);
                        fdisp(5, 1);
                        fdisp(24, 2);
                        fdisp(22, 3);
                    }
                }
                else if (mode1Status == 3)
                {
                    valueBuffer = (int)(outputAmp * 10);

                    fdisp(20, 0);
                    fdisp((valueBuffer / 100) % 10, 1);
                    fdisp((valueBuffer / 10) % 10 + 10, 2);
                    fdisp((valueBuffer / 1) % 10, 3);

                }
                else if (mode1Status == 4)
                {
                    fdisp(21, 0);
                    if (outputFreq > 999) outputFreq = 999;
                    if (outputFreq <= 0) outputFreq = 0;
                    fdisp((outputFreq / 100) % 10, 1);
                    fdisp((outputFreq / 10) % 10, 2);
                    fdisp(outputFreq % 10, 3);
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
                        fdisp(21, 0);
                        if (inputFreq > 999)inputFreq = 999;
                        if (inputFreq <= 0)inputFreq = 0;
                        fdisp((inputFreq / 100) % 10, 1);
                        fdisp((inputFreq / 10) % 10, 2);
                        fdisp(inputFreq % 10, 3);
                    }

                }
                else
                {
                    updateFreFlag = 1;
                    if (updateAmpFlag == 1)
                    {
                        updateAmpFlag = 0;
                        value = (int)(inputAmp * 10);
                        fdisp(20, 0);
                        fdisp((value / 100) % 10, 1);
                        fdisp((value / 10) % 10 + 10, 2);
                        fdisp((value / 1) % 10, 3);
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

void init_adc()
{
    P1ASF = 0x08;
    ADC_CONTR = ADC_INIT;
    delay(2);
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

        XBYTE[DAC_CH2] = OUTPUT_VALUE;
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


void adc_work() interrupt 5
{
    ADC_CONTR = ADC_INIT;
    ADC_RESULT = ADC_RES / 2 + 64;
}

void delay(int delayTime)
{
    unsigned int x;
    while (delayTime--)
    {
        x = 1000;
        while (x--);
    }
}

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
                fdisp(22, 0);
                fdisp(workMode, 1);
                fdisp(22, 2);
                fdisp(22, 3);
            }
        }
        if (workMode == 1) mode1Status = 1;
        initStatus = 0;
        break;
    case 4:
        if (workMode == 1)
        {
            if (mode1Status == 4)
            {
                mode1Status = 1;
            }
            else
            {
                mode1Status = mode1Status + 1;
            }
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
void ampMeasure()
{
    amp = ADC_RESULT;
    if (amp > amp_up)
    {
        amp_up = amp;
    }
    if (amp < amp_low)
    {
        amp_low = amp;
    }
    if (adAddr > 0x0800)
    {
        inputAmp = (amp_up * 5.0 / 1.1 - amp_low * 5.0 / 1.1) / 128;
        amp_up = amp_low = 128;
    }
}

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