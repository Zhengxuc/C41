# include "Device.h"
# include <msp430x16x.h>
AD_REG AD;


unsigned int Initdata[4];
unsigned int result_gd=0;
unsigned int Current_F;
unsigned int Current_L; //
unsigned int Current_Difference;
unsigned char Temp_error_flag=0;

/**************起始信号***************/
void start()
{       
        SDA_Out;
        _NOP();
	SDA_L;
        SCL_Out;
	SCL_H;
	SDA_H;
	SDA_L;  
	SCL_L;
}
/*************终止信号*************/
void stop()
{
        SDA_Out;
        _NOP();
	SDA_L;
	SCL_H;
	SDA_H;
}

/*************向I2C总线发送一个字节************/
void Send1byte(unsigned int byte)
{
	unsigned int i;
        ack();
        SDA_Out;
	for(i=0;i<8;i++)
	{
	   if((byte<<i) & 0x80)
		   SDA_H;
	   else
		   SDA_L;
	   SCL_H;
           SCL_L;
	 }
  SDA_H;
  _NOP();
  SCL_H;
  SCL_L;
}

/**************读取一个字节**************/
unsigned int Read1byte()
{
    unsigned int temp=0,i;
    SDA_In;
    _NOP();
    for(i=0;i<8;i++)
    {
    	temp=temp<<1;
    	SCL_L;
        SCL_H;
        if(P2IN&0x02)
        //if(P2IN&0x04)
        {
        	temp|=0x01;
        }

    }
//    ack();
    SDA_Out;
    SCL_L;
    SDA_L;
    SCL_H;
    SCL_L;
    SDA_H;
    return temp;
}

void ack ()
{
       unsigned int i;
       i =0;
      // SDA_In;
      // SCL_H;
       //delay_ns(15);
       while((P2IN&BIT1)&&(i<80))i++;
     // SDA_Out;
     //  SCL_L;
     //  delay_ns(15);
}



/*******************Config寄存器*********************/
void Confige_1115(unsigned char channel )
{
    unsigned int i=0;
    switch(channel)
    {
    case 0:  Initdata[2] = 0xc4; break;
    case 1:  Initdata[2] = 0xd4; break;
    case 2:  Initdata[2] = 0xe4; break;
    case 3:  Initdata[2] = 0xf4; break; 
    }
    Initdata[0] =0x90;       // 启动配置命令
    Initdata[1] =0x01;       // 指向配置寄存器
    Initdata[3] =0xe3;       // 配置字低字节
    SCL_H;
    start();
    for(i=0;i<4;i++)
    {
    	  Send1byte(Initdata[i]);
    	  _NOP();
    }
    ack();
    stop();
}
/************指向转换结果寄存器***************/
void Point_reg(void)
{
	unsigned char i=0;
	Initdata[0] =0x90;           // 地址 ＋ 写命令
	Initdata[1] =0x00;            // 指向转换结果寄存器
	  SDA_Out;
	  SCL_H;
           ack();
	  start();
	  for(i=0;i<2;i++)
	    {
                  //ack();
		  Send1byte(Initdata[i]);
		  _NOP();
                 // ack();
	    }
          ack();
	  stop();
}
/**************读取转换结果***************/
unsigned int Read_1115(void)
{
  unsigned char   ResultL,ResultH;
  unsigned int    Result;
  SCL_H;
  ack();
  start();
  Send1byte(0x91);
  ResultH=Read1byte();//高8位
  ResultL=Read1byte();//低8位
  ack();
  stop();
  Result=ResultH*256+ResultL;
  return   Result;
}



//------------------------------------------------------------------------------
//        初始化ADC12
//------------------------------------------------------------------------------
void ADC12_Initial(void) 
{
  P2SEL &= ~BIT0; 
  P2SEL &= ~BIT1; 
  P2IE &= ~BIT0; 
  P2IE &= ~BIT1; 
  SCL_Out;
  SDA_In;
  //------------------------------------以下是对DAC进行初始化
  //ADC12CTL0 = REF2_5V+REFON;                    // Internal 2.5V ref on    
  //DAC12_1CTL = DAC12IR + DAC12AMP_5 + DAC12ENC;   // Internal ref gain 1   
  //DAC12_1DAT = 0x0fff;  
  
  ADC12CTL0 =  REFON + REF2_5V;                    // Internal 2.5V ref on     //ADC12CTL0 =  REFON;                    // Internal 2.5V ref on    
  DAC12_1CTL = DAC12IR + DAC12AMP_2 + DAC12ENC ;   // Internal ref gain 1   
  DAC12_0CTL = DAC12IR + DAC12AMP_2 + DAC12ENC ; 
 
  
  //ADC12CTL0 =  REFON;                    // Internal 2.5V ref on    
  //DAC12_1CTL = DAC12IR + DAC12AMP_2 + DAC12ENC ;   // Internal ref gain 1   
 // DAC12_0CTL = DAC12IR + DAC12AMP_2 + DAC12ENC ; 

}

//---------------------------------------------
// 标准排序算法  1  冒泡
//---------------------------------------------
void Bubble_Sort( unsigned int * list, unsigned int length ) 
{
      //---------------------
      unsigned int i,j,flg = 1,Tmp;
	  //-------------------------------
      for(j = 0; j < length - 1 ; j++ )
      {
	  		for(i = 0; i < length - j ; i++ )
      		{
          		if( list[i] > list[i + 1] )   
          		{
               		Tmp = list[i];
               		list[i] = list[i + 1];
               		list[i + 1] = Tmp;  
					flg = 0;
          		}

      		}
			if( flg ) { break;  }
			else      { flg = 1;}
      }		
}

void ADC12_Convert( unsigned char mode )
{  
   unsigned char  i;
   unsigned char  j;
   unsigned long t = 0;

    _NOP();
       Confige_1115(2) ;
       delay(2);
       Point_reg();    
       delay(2);
  // for(i=0;i<4;i++)
  //  {
  //     delay_ns(15);
   //    AD.Temp_Back[i]=Read_1115();
   // } 
   // Bubble_Sort( AD.Temp_Back , 4 );  
   // AD.CurrentBack =AD.Temp_Back[2]*0.209;  
       AD.CurrentBack =Read_1115();       
       AD.CurrentBackRel = AD.CurrentBack; 
       AD.CurrentBack =AD.CurrentBack*0.209;  
       
       
       
       
       Confige_1115(0) ;
       delay(2);
       Point_reg();   
       delay(2);
  // for(j=0;j<4;j++)
  //  {
  //     delay_ns(15);
  //     AD.Temp_In[j]=Read_1115();
  //  } 
  // _NOP();
  //  Bubble_Sort( AD.Temp_In , 4 );
  //  AD.CurrentIn = AD.Temp_In[2]*0.23;   //0.1706
      AD.CurrentIn =  Read_1115();
      
      AD.CurrentIn  =AD.CurrentIn*0.23;
       
    //加校准
//    if(AD.CurrentIn>1000 && AD.CurrentIn <3400)
//    {
 //      AD.CurrentIn -=3;
//    }
    AD.CurrentDis =AD.CurrentIn;
    _NOP();

   if( mode==1 )
   {  
     
     
     
      OP.Meterage_F  = OP.Meterage_L;      // 保存上次采样
      
      OP.Meterage_L  = AD.CurrentBack;     // 保存当前采样
      
      OP.Difference_F = OP.Difference;     // 保存上次偏差     
      
      if (OP.Meterage_L > OP.Meterage_F)   { OP.Difference_Direction = 1;  OP.Difference  = OP.Meterage_L - OP.Meterage_F; } //生成当前正向偏差
      else                                 { OP.Difference_Direction = 0;  OP.Difference  = OP.Meterage_F - OP.Meterage_L; } //生成当前反向偏差
    
      
      
      
      
      Current_F = Current_L;    //保存上次电流值
      Current_L = AD.CurrentIn; //保存当前电流值
      if(Current_F >Current_L)     { Current_Difference = Current_F -Current_L; }
      else                         { Current_Difference = Current_L -Current_F; }
      if(Current_Difference > 11)  { CLK.CurrentInRef =2;  }
   
   }
}
/* DS18B20 单总线驱动（开漏 + 外部 4.7k 上拉到 3V）
 * 空闲：切输入，由上拉拉高；主机只主动拉低，不主动与从机抢高
 * Temp_Process() 非阻塞；AD.Temperature = 实际温度 * 10
 * DS18B20_init()：0=有器件，1=无应答
 */
unsigned char DS18B20_init(void)
{
    unsigned char presence;

    P1SEL &= ~BIT7;              /* 确保普通 GPIO */
    IO_18B20_Out;
    DQ0;
    DS18B20_delay_us(550);       /* 复位拉低 >=480us */

    IO_18B20_In;                 /* 释放，靠上拉变高 */
    DS18B20_delay_us(70);        /* 存在脉冲窗口内采样(约15~60us起) */

    presence = (Read_18B20_IO == 0) ? 0 : 1;  /* 低=有器件 */

    DS18B20_delay_us(410);       /* 等存在脉冲结束，空闲保持输入+上拉 */
    return presence;
}

static void write_bit(unsigned char b)
{
    IO_18B20_Out;
    DQ0;
    if (b) {
        DS18B20_delay_us(6);     /* 写1：拉低 <15us */
        IO_18B20_In;             /* 释放，上拉拉高 */
        DS18B20_delay_us(64);    /* 时隙 >=60us */
    } else {
        DS18B20_delay_us(60);    /* 写0：拉低 >=60us */
        IO_18B20_In;             /* 释放 */
        DS18B20_delay_us(10);    /* 恢复时间 >1us */
    }
}

static unsigned char read_bit(void)
{
    unsigned char b;

    IO_18B20_Out;
    DQ0;
    DS18B20_delay_us(2);         /* 拉低 >1us 启动读时隙 */
    IO_18B20_In;                 /* 立刻释放，勿 DQ1，避免与从机拉低冲突 */
    DS18B20_delay_us(10);        /* 采样点约 12us，须在 15us 内 */
    b = (Read_18B20_IO != 0);
    DS18B20_delay_us(55);        /* 凑满时隙，空闲由上拉保持高 */
    return b;
}

void send_byte(unsigned char cmd)
{
    unsigned char num;
    for (num = 0; num < 8; num++) {
        write_bit(cmd & 0x01);
        cmd >>= 1;
    }
}

unsigned char read_byte(void)
{
    unsigned char i;
    unsigned char data = 0;
    for (i = 0; i < 8; i++) {
        data >>= 1;
        if (read_bit()) {
            data |= 0x80;
        }
    }
    return data;
}

int ucNeFlg = 0; /* 负温度标志，供其它模块使用 */

short read_temp(void)
{
    unsigned char low;
    unsigned char high;
    short raw;
    short wData;

    low  = read_byte();
    high = read_byte();

    /* 总线异常/未接时常读到 0xFFFF */
    if (low == 0xFF && high == 0xFF) {
        Temp_error_flag = 1;
        return AD.Last_Temperature;
    }

    raw = (short)(((unsigned short)high << 8) | low);
    ucNeFlg = (raw < 0) ? 1 : 0;

    /* raw 单位 1/16℃，*10/16 => 温度*10；四舍五入 */
    wData = (short)((raw * 10 + (raw >= 0 ? 8 : -8)) / 16);

    Temp_error_flag = 0;
    AD.Last_Temperature = wData;
    return wData;
}

void skip(void)
{
    send_byte(0xCC);
}

void Temp_convert(void)
{
    send_byte(0x44);
}

void read_sp(void)
{
    send_byte(0xBE);
}

/* 非阻塞读温状态机：启动转换后立刻返回，约1s后再取结果，不堵主循环 */
#define TEMP_ST_IDLE   0
#define TEMP_ST_WAIT   1

static unsigned char temp_state = TEMP_ST_IDLE;
static unsigned int  temp_start_tick = 0;

static unsigned char Temp_StartConvert(void)
{
    _DINT();
    if (DS18B20_init() != 0) {
        _EINT();
        Temp_error_flag = 1;
        return 0;
    }
    skip();
    Temp_convert();
    _EINT();
    return 1;
}

static short Temp_ReadResult(void)
{
    short t;

    _DINT();
    if (DS18B20_init() != 0) {
        _EINT();
        Temp_error_flag = 1;
        return AD.Last_Temperature;
    }
    skip();
    read_sp();
    t = read_temp();
    _EINT();
    return t;
}

/* 主循环每次调用；Flag_Temp 置位时启动转换，等待期满后更新 AD.Temperature */
void Temp_Process(void)
{
    unsigned int elapsed;
    unsigned int wait_ticks;

    if (temp_state == TEMP_ST_IDLE) {
        if (Flag_Temp == 0) {
            return;
        }
        Flag_Temp = 0;
        if (Temp_StartConvert() == 0) {
            return;
        }
        temp_start_tick = CLK.TempTimer;
        temp_state = TEMP_ST_WAIT;
        return;
    }

    /* TEMP_ST_WAIT：等满约 1s（覆盖 DS18B20 12 位 750ms 转换） */
    wait_ticks = CLK.Second_modulate;
    if (wait_ticks < 40) {
        wait_ticks = 40;
    }

    elapsed = (CLK.TempTimer - temp_start_tick) & 0x0FFF;
    if (elapsed < wait_ticks) {
        return;
    }

    AD.Temperature = Temp_ReadResult();
    temp_state = TEMP_ST_IDLE;
}