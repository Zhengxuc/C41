#ifndef __ad_H__
#define __ad_H__
//-------------------------------------------------------------------------------------------------
# include <msp430x16x.h>
# include "delay.h"
# define  REFVOL 1.5

#define SCL_Out P2DIR |=BIT0  //SCL设置为输出模式
#define SCL_H  P2OUT |= BIT0   //P2^3高电平
#define SCL_L  P2OUT &= ~BIT0  //P2^3低电平

#define SDA_H  P2OUT |= BIT1   //P2^2高电平
#define SDA_L  P2OUT &= ~BIT1  //P2^2低电平

#define SDA_In  P2DIR &=~BIT1 //SDA改成输入模式
#define SDA_Out P2DIR |=BIT1  //SDA变回输出模式
#define SDA_val P2IN&0x02
#define TRUE 1
#define FALSE 0

//DS18B20
#define IO_18B20_Out     P1DIR |=BIT7  
#define IO_18B20_In      P1DIR &=~BIT7
#define DQ1             P1OUT |=BIT7
#define DQ0             P1OUT &=~BIT7
#define Read_18B20_IO   P1IN & BIT7

#define CPU_F ((double)3000000)
#define delay_us(x) __delay_cycles((long)(CPU_F*(double)x/1000000.0)) 
#define delay_ms(x) __delay_cycles((long)(CPU_F*(double)x/1000.0))

extern unsigned int Initdata[4];
extern unsigned char Temp_error_flag;

extern int ucNeFlg;

typedef struct                     //---- 这个结构体有点太大了
{    
    unsigned int Temp_In[6];
    unsigned int Counter;
    unsigned int Temp_Back[6];
    unsigned int CurrentBack;
    unsigned int CurrentIn;
    short Temperature;   //定义为2个字节 16位
    short Last_Temperature;
    unsigned int CurrentDis;     //电流显示值, 等于实际输入值
    unsigned int FIFOI[9];
    unsigned int FIFOBack[9];
//    unsigned int K;
    unsigned int Pos[12];
    int Speed[12]; 
    int Acc[12]; 
    unsigned char firsttime;
    unsigned int CurrentBackRel; 
    int  maxSpeedUp;
    int  maxSpeedDown;     
    unsigned int  maxSpeedUpPos;    
    unsigned int  maxSpeedDownPos;
    int  maxDletaUp; 
    int  maxDletaDown;   
    unsigned char pretime;
    int g_vlow;
    int g_vlow2;
    float opening_back;//开度反馈
    float IP_Current_Out;//IP电流值
    float IP_Current_Percent;//IP电流值百分比
}AD_REG;

extern void ADC12_Initial(void);
extern void ADC12_Convert( unsigned char mode );
extern void ack ();
short Get_Temp(void);
//-------------------------------------------------------------------------------------------------
#endif
