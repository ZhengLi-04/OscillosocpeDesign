#include <reg51.h>
#include<absacc.h>
#include<math.h>

//---------------------------------- main.h???? ----------------------------------
#define ADC_BASE_ADDRESS 0x0000
#define CH2 0x2000
#define CH1 0x4000
#define ADC_INIT 0x83
#define ADC_START 0x8B
#define ADC_FLAG 0x10
#define SIN_BASE_ADDRESS 0x1C00
#define TRI_BASE_ADDRESS 0x1D00
#define SQU_BASE_ADDRESS 0x1E00
#define TEE_BASE_ADDRESS 0x1F00

sbit D_SER     = P1 ^ 0;
sbit D_SRCLK   = P1 ^ 1;
sbit D_RCLK    = P1 ^ 2;
sbit KEY1      = P3 ^ 4;
sbit KEY2      = P3 ^ 5;
sbit EADC      = 0xAD;
sbit PADC      = 0xBD;
sfr CLK_DIV    = 0x97;
sbit Y         = P1 ^ 4;
sbit Z         = P1 ^ 5;
sfr ADC_CONTR  = 0xBC;
sfr ADC_RES    = 0xBD;
sfr ADC_RESL   = 0xBE;
sfr P1ASF      = 0x9D;

unsigned char updateAmpFlag = 0;
unsigned char updateFreFlag = 0;
unsigned char mode1Status = 0;
//mode1status = 0初始 1循环显示 2波形 3改幅度 4该频率
unsigned int channalSelect = 0x0000; //DAC通道选择
unsigned int         p_read = 0x0000, p_write = 0x0000, ad_temp = 0;
unsigned char        dspbuf[4] = {0xef, 0xef, 0xef, 0xef}, sel = 0;
unsigned char        lcdbuf[4] = {0xF7, 0xFB, 0xFD, 0xFE};
unsigned int         clocktime = 0, adcount = 0;
unsigned char        ADC_RESULT = 0;
unsigned char        DAC_VALUE = 0;
unsigned char        OUTPUT_VALUE = 0;
unsigned int         adAddress = ADC_BASE_ADDRESS;
unsigned int         daAddress = ADC_BASE_ADDRESS;
unsigned int         sinAddress = SIN_BASE_ADDRESS;
unsigned int         triAddress = TRI_BASE_ADDRESS;
unsigned int         squAddress = SQU_BASE_ADDRESS;
unsigned int         teeAddress = TEE_BASE_ADDRESS;
unsigned char        key_sta = 0, key_num;
unsigned char        workMode = 0;
unsigned char        waveMode = 1;
unsigned char        WAVE_VALUE = 0;
unsigned char        isChange = 0;
unsigned char        freBuffer = 10;
float        ampBuffer = 1.0;
unsigned int         freq = 0;
float                vpp = 0.0;
unsigned char        value = 0;
unsigned char        valueBuffer = 0;
unsigned char        amp = 0;
unsigned char        ampl = 0;
unsigned char        amp_up = 128;
unsigned char        amp_low = 128;
int                  fre = 0;
int                  fre_up = 0;
int                  fre_low = 0;
float                fre_count = 0;
unsigned char        ledbuffer[4] = {0x11, 0x11, 0x11, 0x11};
unsigned char tmpA, tmpB, tmpC;
unsigned char initStatus = 1; //1代表为初始化状态，不更新数码管内容。

void init_timer0();
void init_special_interrupts();
void updateWaveBuffer();
void dsptask();
//void timer_isr() interrupt 1;
//void updateFeature() interrupt 3;
void fdisp(unsigned char n, unsigned char m);
void main(void);
void adc_init();
void adc_start();
void dac_work(int channalSelect, char value);
//void adc_work() interrupt 5;
void delay(int delayTime);
void waveInit();
void key_service();
void keyWork();
void ampMeasure();
void freMeasure();
//---------------------------------- main.h???? ----------------------------------

//---------------------------------- main.c???? ----------------------------------
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

void init_special_interrupts()
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
    Y = !Y;
    if (adcount == 3)
    {
        updateWaveBuffer();
    }
    if (adcount == 5)
    {
        dsptask();
        Z = !Z;
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
        daAddress = adAddress;
        if (adAddress <= 0x1Bf0)
        {
            XBYTE[adAddress] = (ADC_RESULT - 64) * 2;
            ad_temp = ADC_RESULT;
            adAddress++;
        }
        else
        {
            adAddress = ADC_BASE_ADDRESS;
            XBYTE[adAddress] = (ADC_RESULT - 64) * 2;
            ad_temp = ADC_RESULT;
            adAddress++;
        }
    }
    if (workMode == 2)
    {
        if (daAddress <= 0x1Bf0)
        {
            OUTPUT_VALUE = XBYTE[daAddress] / 2;
            daAddress = daAddress + 1;
        }
        else
        {
            daAddress = ADC_BASE_ADDRESS;
            OUTPUT_VALUE = XBYTE[daAddress] / 2;
            daAddress++;
        }
    }
    if (workMode == 3)
    {
        daAddress = adAddress;
        if (adAddress <= 0x0800)
        {
            XBYTE[adAddress] = (ADC_RESULT - 64) * 2;
            ad_temp = ADC_RESULT;
            adAddress++;
        }
        else
        {
            adAddress = ADC_BASE_ADDRESS;
            XBYTE[adAddress] = (ADC_RESULT - 64) * 2;
            ad_temp = ADC_RESULT;
            adAddress++;
        }
    }

    if (clocktime == 500)
    {
        //updateValue();
    }
    else if (clocktime == 2000)
    {
//        if (initStatus == 0)
//        {
//            dspbuf[0] = ledbuffer[0];
//            dspbuf[1] = ledbuffer[1];
//            dspbuf[2] = ledbuffer[2];
//            dspbuf[3] = ledbuffer[3];
//        }
//        else
//        {
//            fdisp(22, 0);
//            fdisp(workMode, 1);
//            fdisp(22, 2);
//            fdisp(22, 3);
//        }
    }
    else if (clocktime == 4000)
    {
//        if (initStatus == 0)
//        {
//            dspbuf[0] = ledbuffer[0];
//            dspbuf[1] = ledbuffer[1];
//            dspbuf[2] = ledbuffer[2] & 0xFE;
//            dspbuf[3] = ledbuffer[3];
//        }
//        else
//        {
//            fdisp(22, 0);
//            fdisp(workMode, 1);
//            fdisp(22, 2);
//            fdisp(22, 3);
//        }
        clocktime = 0;
    }
    EA = 1;
}

void updateWaveBuffer()
{
    if (workMode == 1)
    {
        switch (waveMode)
        {
        case 1:
        {
            if (sinAddress <= 0x1CFF)
            {
                if (ampBuffer != 1)
                {
                    WAVE_VALUE = (XBYTE[sinAddress] - 32) * ampBuffer + 32;
                }
                else
                {
                    WAVE_VALUE = XBYTE[sinAddress];
                }
                sinAddress = sinAddress + 1 + freBuffer / 1.6;
            }
            else
            {
                sinAddress = SIN_BASE_ADDRESS;
                WAVE_VALUE = (XBYTE[sinAddress] - 32) * ampBuffer + 32;
                sinAddress = sinAddress + 1 + freBuffer / 1.6;
            }
        }
        break;
        case 2:
        {
            if (triAddress <= 0x1DF3)
            {
                WAVE_VALUE = (XBYTE[triAddress] - 64) * ampBuffer + 64;
                triAddress = triAddress + 1 + freBuffer / 1.6;
            }
            else
            {
                triAddress = TRI_BASE_ADDRESS;
                WAVE_VALUE = (XBYTE[triAddress] - 64) * ampBuffer + 64;
                triAddress = triAddress + 1 + freBuffer / 1.6;
            }
        }
        break;
        case 3:
        {
            if (squAddress <= 0x1EFF)
            {
                WAVE_VALUE = (XBYTE[squAddress] - 64) * ampBuffer + 64;
                squAddress = squAddress + 1 + freBuffer / 1.6;
            }
            else
            {
                squAddress = SQU_BASE_ADDRESS;
                WAVE_VALUE = (XBYTE[squAddress] - 64) * ampBuffer + 64;
                squAddress = squAddress + 1 + freBuffer / 1.6;
            }
        }
        break;
        case 4:
        {
            if (teeAddress <= 0x1FFF)
            {
                WAVE_VALUE = (XBYTE[teeAddress] - 64) * ampBuffer + 64;
                teeAddress = teeAddress + 1 + freBuffer / 1.6 ;
            }
            else
            {
                teeAddress = TEE_BASE_ADDRESS;
                WAVE_VALUE = (XBYTE[teeAddress] - 64) * ampBuffer + 64;
                teeAddress = teeAddress + 1 + freBuffer / 1.6 ;
            }
        }
        break;
        default:
            break;
        }
    }
}

void fdisp(unsigned char n, unsigned char m)
{
    char  c;
    switch (n)
    {
    case 0:
        c = 0x11;
        break;
    case 1:
        c = 0x7d;
        break;
    case 2:
        c = 0x23;
        break;
    case 3:
        c = 0x29;
        break;
    case 4:
        c = 0x4d;
        break;
    case 5:
        c = 0x89;
        break;
    case 6:
        c = 0x81;
        break;
    case 7:
        c = 0x3d;
        break;
    case 8:
        c = 0x01;
        break;
    case 9:
        c = 0x09;
        break;
    case 10:
        c = 0x10;
        break;
    case 11:
        c = 0x7c;
        break;
    case 12:
        c = 0x22;
        break;
    case 13:
        c = 0x28;
        break;
    case 14:
        c = 0x4c;
        break;
    case 15:
        c = 0x88;
        break;
    case 16:
        c = 0x80;
        break;
    case 17:
        c = 0x3c;
        break;
    case 18:
        c = 0x00;
        break;
    case 19:
        c = 0x08;
        break;
    case 20:  // ???"U"??
        c = 0x51;  // ????:g???,???
        break;
    case 21:  // ???"F"??
        c = 0x87;  // ??:a+g??
        break;
    case 22:  // -
        c = 0xef;  // -
        break;
    case 23:  // n
        c = 0x15;
        break;
    case 24:  // t
        c = 0xc3;
        break;
    case 25:  // r
        c = 0xe7;
        break;
    case 26:  // q
        c = 0x0d;
        break;
    default:
        c = 0x11;
    }
    ledbuffer[m] = c;
    // if (initStatus == 1)dspbuf[m] = c;
    dspbuf[m] = c;
}

void main(void)
{

    CLK_DIV = CLK_DIV | 0x01;
    init_timer0();
    init_timer1();
    init_special_interrupts();
    adc_init();
    waveInit();
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
                        fdisp(21, 0);  // ??"F"

                        if (freBuffer > 999) freBuffer = 999;
                        if (freBuffer <= 0) freBuffer = 0;

                        fdisp((freBuffer / 100) % 10, 1); // ??
                        fdisp((freBuffer / 10) % 10, 2); // ??
                        fdisp(freBuffer % 10, 3); // ??
                    }
                    else
                    {
                        valueBuffer = (int)(ampBuffer * 10);

                        fdisp(20, 0);  // ???"U"??(???3????)

                        fdisp((valueBuffer / 100) % 10, 1); // ??
                        fdisp((valueBuffer / 10) % 10 + 10, 2); // ??
                        fdisp((valueBuffer / 1) % 10, 3); // ??
                    }
                }
                else if (mode1Status == 2)
                {
                    if (waveMode == 1)
                    {
                        fdisp(22, 0);
                        fdisp(5, 1);
                        fdisp(1, 2);
                        fdisp(23, 3);
                    }
                    else if (waveMode == 2)
                    {
                        fdisp(22, 0);
                        fdisp(24, 1);
                        fdisp(25, 2);
                        fdisp(1, 3);
                    }
                    else if (waveMode == 3)
                    {
                        fdisp(22, 0);
                        fdisp(5, 1);
                        fdisp(26, 2);
                        fdisp(20, 3);
                    }
                    else if (waveMode == 4)
                    {
                        fdisp(22, 0);
                        fdisp(5, 1);
                        fdisp(24, 2);
                        fdisp(22, 3);
                    }
                }
                else if (mode1Status == 3)
                {
                    valueBuffer = (int)(ampBuffer * 10);

                    fdisp(20, 0);  // ???"U"??(???3????)

                    fdisp((valueBuffer / 100) % 10, 1); // ??
                    fdisp((valueBuffer / 10) % 10 + 10, 2); // ??
                    fdisp((valueBuffer / 1) % 10, 3); // ??

                }
                else if (mode1Status == 4)
                {
                    fdisp(21, 0);  // ??"F"

                    if (freBuffer > 999) freBuffer = 999;
                    if (freBuffer <= 0) freBuffer = 0;

                    fdisp((freBuffer / 100) % 10, 1); // ??
                    fdisp((freBuffer / 10) % 10, 2); // ??
                    fdisp(freBuffer % 10, 3); // ??
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
                        //freMeasure();
                        updateFreFlag = 0;
                        fdisp(21, 0);  // ??"F"

                        if (freq > 999) freq = 999;
                        if (freq <= 0) freq = 0;

                        fdisp((freq / 100) % 10, 1); // ??
                        fdisp((freq / 10) % 10, 2); // ??
                        fdisp(freq % 10, 3); // ??
                    }

                }
                else
                {
                    updateFreFlag = 1;
                    if (updateAmpFlag == 1)
                    {
                        //ampMeasure();
                        updateAmpFlag = 0;
                        value = (int)(vpp * 10);

                        fdisp(20, 0);  // ???"U"??(???3????)

                        fdisp((value / 100) % 10, 1); // ??
                        fdisp((value / 10) % 10 + 10, 2); // ??
                        fdisp((value / 1) % 10, 3); // ??
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
//---------------------------------- main.c???? ----------------------------------

//---------------------------------- adc.c???? ----------------------------------
void adc_init()
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
        channalSelect = CH1;
        dac_work(channalSelect, DAC_VALUE);
        channalSelect = CH2;
        dac_work(channalSelect, WAVE_VALUE);
    }
    break;
    case 2:
    {
        channalSelect = CH1;
        dac_work(channalSelect, DAC_VALUE);
        channalSelect = CH2;
        dac_work(channalSelect, OUTPUT_VALUE);
    }
    break;
    case 3:
    {
        channalSelect = CH1;
        dac_work(channalSelect, DAC_VALUE);
        channalSelect = CH2;
        dac_work(channalSelect, 0x00);
    }
    break;
    default:
        break;
    }
}

void dac_work(int channalSelect, char value)
{
    XBYTE[channalSelect] = value;
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
//---------------------------------- adc.c???? ----------------------------------

//---------------------------------- workMode1-outputWave.c???? ----------------------------------
void waveInit()
{
    unsigned int address = 0;
    unsigned int i = 0;
    //sin
    i = 0;
    address = SIN_BASE_ADDRESS;
    for (; address <= 0x1CFF; address++, i++)
    {
        XBYTE[address] = floor(14 * (sin(3.14 * i / 128) +1)) + 32; //14是根据硬件调整的经验值
    }
    //triangular
    i = 0;
    address = TRI_BASE_ADDRESS;
    for (; address <= 0x1D7F; address++, i++)
    {
        XBYTE[address] = 49 + floor(30 * (i / 128.0));
    }
    i = 0;
    address = 0x1D80;
    for (; address <= 0x1DFF; address++, i++)
    {
        XBYTE[address] = 79 - floor(30 * (i / 128.0));
    }
    //square
    address = SQU_BASE_ADDRESS;
    for (; address <= 0x1E7F; address++)
    {
        XBYTE[address] = 64 + 15;
    }
    address = 0x1E80;
    for (; address <= 0x1EFF; address++)
    {
        XBYTE[address] = 64 - 15;
    }
    //teeth
    i = 0;
    address = TEE_BASE_ADDRESS;
    for (; address <= 0x1FFF; address++, i++)
    {
        XBYTE[address] = 64 - 15 + floor(30 * i / 256);
    }
}
//---------------------------------- workMode1-outputWave.c???? ----------------------------------

//---------------------------------- key.c???? ----------------------------------
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
//        if (workMode == 1)
//        {
//            if (isChange == 0)
//            {
//                isChange = 1;
//            }
//            else
//            {
//                isChange = 0;
//                freBuffer = 0;
//                ampBuffer = 1;
//            }
//        }
        delay(100);
        break;
    case 5:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                waveMode = 1;
            else if (mode1Status == 3 && ampBuffer <= 4)
                ampBuffer = ampBuffer + 1;
            else if (mode1Status == 4 && freBuffer <= 990)
                freBuffer = freBuffer + 10;
        }
//        if (workMode == 1)
//        {
//            if (!isChange)
//            {
//                waveMode = 1;
//            }
//            else
//            {
//                if (freBuffer >= 1)
//                {
//                    freBuffer = freBuffer - 1;
//                }
//            }
//        }
        delay(100);
        break;
    case 6:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                waveMode = 2;
            else if (mode1Status == 3 && ampBuffer >= 2)
                ampBuffer = ampBuffer - 1;
            else if (mode1Status == 4 && freBuffer >= 20)
                freBuffer = freBuffer - 10;
        }
//        if (workMode == 1)
//        {
//            if (!isChange)
//            {
//                waveMode = 2;
//            }
//            else
//            {
//                freBuffer = freBuffer + 1;
//            }
//        }
        delay(100);
        break;
    case 7:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                waveMode = 3;
            else if (mode1Status == 3 && ampBuffer <= 4.9)
                ampBuffer = ampBuffer + 0.1;
            else if (mode1Status == 4 && freBuffer <= 999.8)
                freBuffer = freBuffer + 1;
        }
//        if (workMode == 1)
//        {
//            if (!isChange)
//            {
//                waveMode = 3;
//            }
//            else
//            {
//                if (ampBuffer >= 2)
//                {
//                    ampBuffer = ampBuffer - 1;
//                }
//            }
//        }
        delay(100);
        break;
    case 8:
        if (workMode == 1)
        {
            if (mode1Status == 2)
                waveMode = 4;
            else if (mode1Status == 3 && ampBuffer >= 0.2)
                ampBuffer = ampBuffer - 0.1;
            else if (mode1Status == 4 && freBuffer >= 2)
                freBuffer = freBuffer - 1;
        }
//        if (workMode == 1)
//        {
//            if (!isChange)
//            {
//                waveMode = 4;
//            }
//            else
//            {
//                ampBuffer = ampBuffer + 1;
//            }
//        }
        delay(100);
        break;
    default:
        break;
    }
}
//---------------------------------- key.c???? ----------------------------------

//---------------------------------- featureExtract.c???? ----------------------------------
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
    if (adAddress > 0x0800)
    {
        vpp = (amp_up * 5.0 - amp_low * 5.0) / 128;
        amp_up = amp_low = 128;
    }
}

void freMeasure()
{
    amp = ADC_RESULT;
    if (amp > 128 && ampl <= 128)
    {
        fre_up = adAddress;
        if (fre_low != 0)
        {
            fre = fre + fabs(fre_low - fre_up);
            fre_count++;
        }
        fre_low = fre_up;
    }
    if (adAddress > 0x0800)
    {
        freq = floor(2000 / (fre * 1.0 / fre_count));
        fre = 0;
        fre_up = fre_low = 0;
        fre_count = 0;
        amp = ampl = 129;
    }
    ampl = amp;
}
//---------------------------------- featureExtract.c???? ----------------------------------