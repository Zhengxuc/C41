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
/*
①单片机拉低这个引脚，持续480~960us (以持续500us为例)
②单片机释放总线，即拉高电平。
③15 ~60us后，如果DS18B20这个器件存在，就会主动拉低这个引脚，返回一个低脉冲（为了确保读到这个脉冲，选择延时60us,但不能超过75us）
④持续60 ~240us后，DS18B20释放总线，I0端口被上拉电阻拉高。
————————————————
版权声明：本文为CSDN博主「PCONST」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/m0_50620305/article/details/113976976
*/
//DS18B20驱动
unsigned char DS18B20_init(void)
{
    unsigned char i = 2;
    IO_18B20_Out;
    DQ1;             //先上拉电平打断初始化之前的操作
    DQ0;             //拉低电平等待响应
    delay_us(520);
    DQ1;
    delay_us(15);
    IO_18B20_In;
    delay_us(80);  
    if(P1IN & BIT7){i=1;IO_18B20_Out;} 
    else{i=0;IO_18B20_Out;} //如果是低电平，则初始化成功
    delay_us(300);
    return i;
}

//发送一个字节的命令
void send_byte(unsigned char cmd) 
{ 
   unsigned char num=0;
   unsigned char testb=0;
    for(num=0;num<8;num++){  //进行循环以发送每一位的数据
      IO_18B20_Out;
      DQ0;
      delay_us(8);
      if(cmd & 0x01){DQ1;}
      delay_us(60);
      DQ1;
      cmd >>= 1;
        
    }

}

  


//读取一个字节
unsigned char read_byte(void)
{   
    unsigned char i;
    unsigned char data=0;
    for(i=0;i<8;i++){
       data>>=1;         //右移，读取下一位
       DQ0;
       delay_us(5);
       DQ1;      
       delay_us(8);
       IO_18B20_In;
       _NOP();
       if(P1IN & BIT7){data|=0x80;} 
       delay_us(50);
       IO_18B20_Out;
       DQ1;
       delay_us(10);
    }
    return data;
}


   int ucNeFlg=0; /*定义达拉斯测试是否为负温度标准*/
//读取操作进行两次并将结果合并
short read_temp(void)
{     
    unsigned char low=0;
    unsigned char high=0;
    int               t1=0;    /*温度整数部分的值*/
    unsigned char     t2=0;   /*温度小数部分的值*/
    short    wData=0;
    
    char  ucTemph,ucTempl;
    float    ucfloat;
    low = read_byte();
    high= read_byte();
    
    if(low==0xff && high==0xff)
    {
        return AD.Last_Temperature;
    }
    
    ucTemph=high;
    ucTempl=low;
    
    if((high&0xF0)>0)
    { //判断是否为负温度
      ucNeFlg=1;   //
    }
    else
    {
      ucNeFlg=0;
    }
    
    if(ucNeFlg)
    { //如果为负温度取反加1
      ucTempl=~ucTempl+1;
      ucTemph=~ucTemph+(!ucTempl?1:0);
    }
    
    
    t1=ucTemph<<4; //得到温度整数部分
    t1|=(ucTempl>>4);
    ucfloat =(float)(ucTempl&0x0F);
    ucfloat *=0.625;	
    t2 = (unsigned char)ucfloat;
    wData = t1*10+t2;
    
    if(ucNeFlg)
    { 
      //wData=~wData+1;
      return - wData;   //
    
    }
    else
      return wData;
}

//向18B20发送跳过ROM命令
void skip(void){         
   send_byte(0xcc);
}

//温度转换命令
void Temp_convert(void){
   send_byte(0x44);      
}

//读取ScratchPad命令
void read_sp(void){     
   send_byte(0xbe);
}

//做一次转换
short Get_Temp(void)
{         
    DS18B20_init();  //初始化
    skip();                 //跳过ROM
    Temp_convert();                   //转换
    delay_ms(750);          //转换时延时，12位精度最少需要750ms
    DS18B20_init();   //初始化
    skip();                 //跳过ROM命令
    read_sp();              //读取数据 
    return read_temp();     //返回数据为实际温度*10
}