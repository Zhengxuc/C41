//*****************************************************************************|
// Author:      stefanie                                                       |
// date:        2007-10-24                                                     |
// date:        2008-09-18                                                     |
// description: Keyboard for msp430F                                           |
// cpu:         MSP430 F 149                                                   |
// crystal:                                                                    |
// Version:     V1.0                                                           |
//*****************************************************************************|
//# include "KEYBOARD.H"
#include "Device.h"
//------------------------------------------------------------------------------
// 键盘初始化
//------------------------------------------------------------------------------
KEYBORAD_REG KB;
//------------------------------------------------------------------------------
void Keyboard_Initial(void)            // 9
{
    KEYSEL &=~(KEY1|KEY2|KEY3|KEY4);   // 定义引脚功能       （普通IO口）
    KEYDIR &=~(KEY1|KEY2|KEY3|KEY4);   // 定义引脚方向
    KEYIES &=~(KEY1|KEY2|KEY3|KEY4);   // 由低跳变到高会引发中断
    KEYIFG &=~(KEY1|KEY2|KEY3|KEY4);   // 清除中断标志位    （必须由手动清除）                                                                    
    KEYIE  |= (KEY1|KEY2|KEY3|KEY4);   // 引脚中断功能开
    KB.all = 0x0000;
} 

#pragma vector=PORT2_VECTOR 
__interrupt void Port_Isr(void)
{
   unsigned char temp;
   unsigned char hw_cur;
   unsigned char hw_last;
   WDTCTL = WDTPW + WDTHOLD;         // 关闭看门狗
   hw_cur = KEYIN & 0XF0;            // 读物理引脚
   temp = SwapKeyHi4(hw_cur);        // ← 翻转成逻辑按键状态
   delay(100);                       // 延时祛痘  这个延时影响功耗
   hw_last = KEYIN & 0XF0;           // 再读一次物理引脚做确认
   if(hw_cur == hw_last)             // 确认（物理位一致即可）
   { 
      KB.bit.Refresh = 1; 
      // KEYIFG 是硬件中断标志，先翻转成逻辑位再用 F 值做 switch
      switch( SwapKeyHi4(KEYIFG) & 0xF0 )
      {
         case  INC_F     : {
                            if( SwapKeyHi4(KEYIES) & INC_F ) { KB.bit.PressLtmp = 0; } //抬起中断
                            else                              { KB.bit.PressLtmp = 1; } //按下中断
                        } break;     
         case  DEC_F     : {
                            if( SwapKeyHi4(KEYIES) & DEC_F ) { KB.bit.PressLtmp = 0; } //抬起中断
                            else                              { KB.bit.PressLtmp = 1; } //按下中断       
                        } break;       
         case  MODE_F    : {
                            if( SwapKeyHi4(KEYIES) & MODE_F ) { KB.bit.PressLtmp = 0; } //抬起中断
                            else                               { KB.bit.PressLtmp = 1;  } //按下中断      
                        } break;      
         case  CONFIRM_F : {
                            if( SwapKeyHi4(KEYIES) & CONFIRM_F ) { KB.bit.PressLtmp = 0; } //抬起中断
                            else                                  { KB.bit.PressLtmp = 1; } //按下中断       
                        } break;  
         default:  break;
      }
      // KEYIES 是硬件寄存器，写回前要把逻辑 temp 反转成物理位
      KEYIES = SwapKeyHi4(temp);
      if( (KB.bit.KeyTmpF==INC_CONFIRM)||(KB.bit.KeyTmpF==DEC_CONFIRM)||(KB.bit.KeyTmpF==MODE_CONFIRM))//||(KB.bit.KeyF==INC_MODE)||(KB.bit.KeyF==DEC_MODE) )  
      { 
         //KB.bit.PressL = 0;
         if( temp==0 )  KB.bit.KeyTmpF = 0;
      }
      else { KB.bit.KeyTmpF = KB.bit.KeyTmpL; }
    }
    ALA.bit.KB__FLAG = 0;
//    ALA.bit.Delay = 0;
    KB.bit.LongKB = 0;
    //----------------------------------
    if( CLK.SecKB ) CLK.SecCONFIRM = 0;
    //----------------------------------
    //CLK.SecCONFIRM = 0;
    //KB.bit.LongKB_M = 0;
    KB.bit.KeyTmpL = temp>>4;        // temp 已经是逻辑按键状态，直接用
    KEYIFG &= 0X0F;                    // 清除所有标记（清物理位）     
    WDTCTL = WDT_MRST_32;              // 放狗,并选时钟源为SMCLK,即定时时间为SMCLK/32768
  // _BIC_SR_IRQ(LPM1_bits);          // 退出低功耗  
}


         
