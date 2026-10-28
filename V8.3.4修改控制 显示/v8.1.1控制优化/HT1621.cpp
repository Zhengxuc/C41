# include <msp430x16x.h>
# include "Device.h"
# include "Dis_lib.h"

# define MOSFET  0
//------------------------------------------------------------------------------
DIS_REG DIS;    // 有MOSFET
//------------------------------------------------------------------------------
#if  MOSFET
void send_data(unsigned int data_temp,unsigned char loop)
{
    asm("nop");asm("nop");asm("nop");asm("nop");
    for(;loop!=0;loop--)
    {
       LCD_PORT |= WR;                  // 产生下降沿               1
       asm("nop");asm("nop");asm("nop");asm("nop");
       if((data_temp&0X8000)==0)   
       {
          LCD_PORT |= DATA;             // 数据生成                 1
       }
       else
       {
          LCD_PORT &=~DATA;                                 //     1
       }
       asm("nop");asm("nop");asm("nop");asm("nop");
       LCD_PORT &=~WR;                 // 数据锁定                 1 
       asm("nop");asm("nop");asm("nop");asm("nop");
       data_temp=(data_temp<<1);
    }
}

//------------------------------------------------------------------------
// HT1621初始化    
//------------------------------------------------------------------------

void HT1621_Initial(void)
{
    unsigned char t;             
  //  P3DIR = 0;                  //将P1口所有的管脚在初始化的时候设置为输入方式
  //  P3SEL = 0;                  //将P1口所有的管脚设置为一般I/O口
    //--------------------------------------------
    P3DIR |= (WR+DATA+CS1+CS0);
    LCD_PORT &=~(CS1);          // 液晶引脚置为输出
    LCD_PORT &=~(CS0);          // 液晶引脚置为输出
    LCD_PORT &=~WR; 
    LCD_PORT &=~DATA;           // 液晶引脚置为输出
    //---------------------------------------------  
    LCD_PORT |=(CS1|CS0);       // HT1621使能   
    
    asm("nop");asm("nop");asm("nop");asm("nop");
    
    send_data(COMMAND,3);       // 送3位命令模式代码100
    for(t=0;t<=7;t++) 
    {
      send_data(init_tab[t],9); // 送9位命令代码 
    }
    
    LCD_PORT &=~CS1;            // HT1621关闭                           
    LCD_PORT &=~CS0;            // HT1621关闭   
    asm("nop");asm("nop");asm("nop");asm("nop");
    //-------------------------------------------------------------
    LCD_PORT |=CS1;               //HT1621使能    因硬件电路取反 
    LCD_PORT |=CS0;               //HT1621使能   因硬件电路取反
    asm("nop");asm("nop");asm("nop");asm("nop");
    send_data(0xa000,3);          //送3位“写 ”模式命令代码101
    send_data(0x0000,6);          //送6位首地址00h   
    for(;t<=32;t++)                
    {
        send_data(0x00,4);        //下面的四个数字
    }         
    LCD_PORT &=~CS1;              //HT1621关闭   因硬件电路取反   
    LCD_PORT &=~CS0;              //HT1621关闭   因硬件电路取反  
    asm("nop");asm("nop");asm("nop");asm("nop");
    //--------------------------------------------------------------
    LCD_PORT |=CS0+CS1;         // HT1621使能    因硬件电路取反 
    asm("nop");asm("nop");asm("nop");asm("nop");
    send_data(0xa000,3);        // 送3位“写 ”模式命令代码101
    send_data(0x0000,6);        // 送6位首地址00h  
    for(t=0;t<=7;t++)
    {
       send_data(0xffff,16);       
    }  
    LCD_PORT &=~(CS0+CS1);       // HT1621关闭   因硬件电路取反  
    asm("nop");asm("nop");asm("nop");asm("nop");
    //delay(5000); 
}


void Display(void)
{
    unsigned char t;
    //----------------------------------------------------------小
    LCD_PORT |=CS0;             //HT1621使能   因硬件电路取反
    asm("nop");asm("nop");asm("nop");asm("nop");
    send_data(0xa000,3);        //送3位“写 ”模式命令代码101
    send_data(0x0000,6);        //送6位首地址00h
    if( DIS.ScreenS == ALARM_S )
    {
        for( t=0;t<=7;t++ )
        {
           send_data( number0[ DIS.Small_Dis[t] ], 16 );       
        }
    }
    else
    {
        for( t=0;t<=7;t++ )
        {
          send_data(number0[ small_sting[DIS.ScreenS][t] ],16);       
        }
    }  
    LCD_PORT &=~CS0;            //HT1621关闭   因硬件电路取反
    //------------------------------------------------------------大
    delay(10); 
    LCD_PORT |=CS1;                      // HT1621使能    因硬件电路取反  
    asm("nop");asm("nop");asm("nop");asm("nop");    
    send_data(0xa000,3);                 // 送3位“写 ”模式命令代码101
    send_data(0x0400,6);                 // 送6位首地址00h 
    if(DIS.ScreenB==DIGITAL)
    {
        for(t=0;t<=3;t++)                
        {   
            if(DIS.Point==t) { send_data(number1[ DIS.Digital_Dis[t] ] | 0X1000,16); }
            else             { send_data(number1[ DIS.Digital_Dis[t] ],16);          } //下面的四个数字
        } 
    }  
    else if(DIS.ScreenB==ALARM)
    {
        for(t=0;t<=3;t++)                
        {   
             send_data(number1[ DIS.Alarm_Dis[t] ],16); 
        } 
    }    
    else
    {  
        if( SYS.bit.PageS2 == 0 && SYS.bit.PageStateL == CONFIG )
        {  
            for(t=0;t<=3;t++)                
            {   
              if(DIS.Point==t) { send_data(number1[  big_sting[DIS.ScreenB][t]  ] | 0X1000,16); }
              else             { send_data(number1[  big_sting[DIS.ScreenB][t]  ],16);     } //下面的四个数字
            } 
        }
        else
        {
            for(t=0;t<=3;t++)                
            {   
                send_data(number1[  big_sting[DIS.ScreenB][t]  ],16);   //下面的四个数字
            }  
        }  
    }
    LCD_PORT &=~CS1;                     //HT1621关闭   因硬件电路取反 
    delay(10); 
    //------------------------------------------------------------
    LCD_PORT |=CS1;               //  HT1621使能    因硬件电路取反 
    asm("nop");asm("nop");asm("nop");asm("nop");
    send_data(0xa000,3);          //  送3位“写 ”模式命令代码101
    send_data(0x0000,6);          //  送6位首地址00h 
    send_data(DIS.SignL,4);       //  写入左边需要显示的图形
    LCD_PORT &=~CS1;              //  HT1621关闭   因硬件电路取反 
     delay(10); 
    //---------------------------------------------------------------
    LCD_PORT |=CS1;               //  HT1621使能    因硬件电路取反 
    asm("nop");asm("nop");asm("nop");asm("nop");
    send_data(0xa000,3);          // 送3位“写 ”模式命令代码101
    send_data(0x4400,6);          // 送6位首地址00h 
    send_data(DIS.SignR,12);      // 写入左边需要显示的图形
    LCD_PORT &=~CS1;              //  HT1621关闭   因硬件电路取反  
}

#else

//------------------------------------------------------------------------------
// 没有MOSFET
void send_data(unsigned int data_temp,unsigned char loop)
{
    for(;loop!=0;loop--)
    {
       LCD_PORT &= ~WR;                  // 产生下降沿               1
       if((data_temp&0X8000)==0)   
       {
          LCD_PORT &= ~DATA;             // 数据生成                 1
       }
       else
       {
          LCD_PORT |=DATA;                                 //     1
       }
       LCD_PORT |= WR;                 // 数据锁定                 1 
       data_temp=(data_temp<<1);
    }
}

//-----------------------------------------------------------------------
// HT1621初始化    
//-----------------------------------------------------------------------

void HT1621_Initial(void)
{
    unsigned char t;             
  //  P3DIR = 0;                  //将P1口所有的管脚在初始化的时候设置为输入方式
  //  P3SEL = 0;                  //将P1口所有的管脚设置为一般I/O口
    //--------------------------------------------
    P3DIR |= (WR+DATA+CS1+CS0);
    LCD_PORT |=(CS1);          // 液晶引脚置为输出
    LCD_PORT |=(CS0);          // 液晶引脚置为输出
    LCD_PORT |=WR; 
    LCD_PORT |=DATA;           // 液晶引脚置为输出
    //---------------------------------------------  
    LCD_PORT &=~(CS1|CS0);       // HT1621使能                           
    send_data(COMMAND,3);       // 送3位命令模式代码100
    for(t=0;t<=7;t++) 
    {
      send_data(init_tab[t],9); // 送9位命令代码 
    }
    LCD_PORT |=CS1;            // HT1621关闭                           
    LCD_PORT |=CS0;            // HT1621关闭                           
    //-------------------------------------------------------------
    LCD_PORT &=~CS1;               //HT1621使能    因硬件电路取反 
    LCD_PORT &=~CS0;               //HT1621使能   因硬件电路取反
    send_data(0xa000,3);          //送3位“写 ”模式命令代码101
    send_data(0x0000,6);          //送6位首地址00h   
    for(;t<=32;t++)                
    {
        send_data(0x00,4);        //下面的四个数字
    }         
    LCD_PORT |=CS1;              //HT1621关闭   因硬件电路取反   
    LCD_PORT |=CS0;              //HT1621关闭   因硬件电路取反  
    //--------------------------------------------------------------
    LCD_PORT &=~(CS0+CS1);         // HT1621使能    因硬件电路取反 
    send_data(0xa000,3);        // 送3位“写 ”模式命令代码101
    send_data(0x0000,6);        // 送6位首地址00h 
    for(t=0;t<=7;t++)
    {
       send_data(0xffff,16);       
    }  
    LCD_PORT |=(CS0+CS1);       // HT1621关闭   因硬件电路取反  
    //delay(5000); 
}


void Display(void)
{
    unsigned char t;
    //----------------------------------------------------------小
    LCD_PORT &=~CS0;             //HT1621使能   因硬件电路取反
    send_data(0xa000,3);        //送3位“写 ”模式命令代码101
    send_data(0x0000,6);        //送6位首地址00h
    if( DIS.ScreenS == ALARM_S )
    {
        for( t=0;t<=7;t++ )
        {
           send_data( number0[ DIS.Small_Dis[t] ], 16 );       
        }
    }
    else
    { //增加自动模式下显示目标值

        if( SYS.bit.PageStateL == AUTO )
        {
           for( t=0;t<=2;t++ )
           {
             //send_data( number0[ DIS.Small_Dis[t] ], 16 );
             // Y:增加小数显示
                if (g_PIDServiceData.SetPointDis < 1005)
                {
                    if (t == 1)
                    {
                        send_data((number0[DIS.Small_Dis[t]] | 0x0001), 16); // Y：带小数的显示
                    }
                    else
                    {
                        send_data(number0[DIS.Small_Dis[t]], 16);
                    }
                }
                else
                    send_data(number0[DIS.Small_Dis[t]], 16);
           }       
           for( t=3;t<=7;t++ )
           {
              send_data(number0[ small_sting[DIS.ScreenS][t] ],16);       
           }         
        
        }
        else
        {
           for( t=0;t<=7;t++ )
           {
              send_data(number0[ small_sting[DIS.ScreenS][t] ],16);       
           }
        }
    }  
    LCD_PORT |=CS0;            //HT1621关闭   因硬件电路取反
    //------------------------------------------------------------大
    delay(10); 
    LCD_PORT &=~CS1;                      // HT1621使能    因硬件电路取反      
    send_data(0xa000,3);                 // 送3位“写 ”模式命令代码101
    send_data(0x0400,6);                 // 送6位首地址00h 
    if(DIS.ScreenB==DIGITAL)
    {
        for(t=0;t<=3;t++)                
        {   
            if(DIS.Point==t) { send_data(number1[ DIS.Digital_Dis[t] ] | 0X1000,16); }
            else             { send_data(number1[ DIS.Digital_Dis[t] ],16);          } //下面的四个数字
        } 
    }  
    else if(DIS.ScreenB==ALARM)
    {
        for(t=0;t<=3;t++)                
        {   
             send_data(number1[ DIS.Alarm_Dis[t] ],16); 
        } 
    }    
    else
    {  
        if( SYS.bit.PageS2 == 0 && SYS.bit.PageStateL == CONFIG )
        {  
            for(t=0;t<=3;t++)                
            {   
              if(DIS.Point==t) { send_data(number1[  big_sting[DIS.ScreenB][t]  ] | 0X1000,16); }
              else             { send_data(number1[  big_sting[DIS.ScreenB][t]  ],16);     } //下面的四个数字
            } 
        }
        else
        {
            for(t=0;t<=3;t++)                
            {   
                send_data(number1[  big_sting[DIS.ScreenB][t]  ],16);   //下面的四个数字
            }  
        }  
    }
    LCD_PORT |=CS1;                     //HT1621关闭   因硬件电路取反 
    delay(10); 
    //------------------------------------------------------------
    LCD_PORT &=~CS1;               //  HT1621使能    因硬件电路取反 
    send_data(0xa000,3);          //  送3位“写 ”模式命令代码101
    send_data(0x0000,6);          //  送6位首地址00h 
    send_data(DIS.SignL,4);       //  写入左边需要显示的图形
    LCD_PORT |=CS1;              //  HT1621关闭   因硬件电路取反 
     delay(10); 
    //---------------------------------------------------------------
    LCD_PORT &=~CS1;               //  HT1621使能    因硬件电路取反 
    send_data(0xa000,3);          // 送3位“写 ”模式命令代码101
    send_data(0x4400,6);          // 送6位首地址00h 
    send_data(DIS.SignR,12);      // 写入左边需要显示的图形
    LCD_PORT |=CS1;              //  HT1621关闭   因硬件电路取反  
}



//----------------------------------------------------------------
//--有来显示摩擦LOGO      2017年3月10日
//----------------------------------------------------------------
//                             Welcome
//                               MORC
//----------------------------------------------------------------

void Dis_LOGO (void)
{
    unsigned char t;
    //----------------------------------------------------------小
    LCD_PORT &=~CS0;             //HT1621使能   因硬件电路取反
    send_data(0xa000,3);        //送3位“写 ”模式命令代码101
    send_data(0x0000,6);        //送6位首地址00h
    
    for( t=0;t<=7;t++ )
    {
       send_data( number0[ small_sting[130][t]  ], 16 );      
    }    
      
    LCD_PORT |=CS0;            //HT1621关闭   因硬件电路取反
    
    delay(10); 
    LCD_PORT &=~CS1;                      // HT1621使能    因硬件电路取反      
    send_data(0xa000,3);                 // 送3位“写 ”模式命令代码101
    send_data(0x0400,6);                 // 送6位首地址00h 

    for(t=0;t<=3;t++)                
    {   
       send_data(number1[ big_sting[101][t] ],16); 
    }     
              
    LCD_PORT |=CS1;                     //HT1621关闭   因硬件电路取反 
    delay(10); 
    //------------------------------------------------------------
    LCD_PORT &=~CS1;               //  HT1621使能    因硬件电路取反 
    send_data(0xa000,3);          //  送3位“写 ”模式命令代码101
    send_data(0x0000,6);          //  送6位首地址00h 
    send_data(DIS.SignL,4);       //  写入左边需要显示的图形
    LCD_PORT |=CS1;              //  HT1621关闭   因硬件电路取反 
     delay(10); 
    //---------------------------------------------------------------
    LCD_PORT &=~CS1;               //  HT1621使能    因硬件电路取反 
    send_data(0xa000,3);          // 送3位“写 ”模式命令代码101
    send_data(0x4400,6);          // 送6位首地址00h 
    send_data(DIS.SignR,12);      // 写入左边需要显示的图形
    LCD_PORT |=CS1;              //  HT1621关闭   因硬件电路取反    
    delay(800); 
   
}

#endif
