#include "Device.h"
//------------------------------------------------------------------------------
TICK_REG CLK;
//------------------------------------------------------------------------------
void Cpu_Tick_Initial(void)
{
  unsigned int a;
    //-------------------------------------------------------------------
    // DCOCTL = 0X80;              // 1110 0000  80
    // BCSCTL1= 0X87;              // 1000 0110  87
    // BCSCTL2= 0X06;              // 0000 0000  //辅助时钟八分频
     DCOCTL = 0X80;              // 1110 0000  80
     BCSCTL1= 0X07;              // 1000 0110  87
     BCSCTL2= 0X06; // BCSCTL2= 0Xce;              // 0000 0000  //
  
     delay(50); 
  
    // delay(50);                  // 等待时钟稳定 
    //----------------------------------------   定时器B  预初始化 ---
     TBCTL = MC_0 + ID_3 + TBSSEL_2 + TBCLR; // 定时器B时钟源为SMCLK,并清TAR
     TBCCR0 = 20;                            // 计数值为50000个SMCLK周期 
     TBCCTL0 |= CCIE;                        // CCR0中断使能
     //-----------------------------------------------------
 //    TBCCR1 = 20;                           
 //    TBCCTL1 |= CCIE;                        // CCR1中断使能
     //-----------------------------------------------------
  //   TBCCR2 = 20;                           
  //   TB CCTL2 |= CCIE;                        // CCR2中断使能
    //--------------------------------------------------------------------
}

//------------------------------------------------------------------------------
// 定时器b中断        中断服务程序
//------------------------------------------------------------------------------ 


int Flag_Adc = 0; 
int Flag_Message = 0;
int Flag_Display = 0;

static uint16_t tick = 0;

#pragma vector = TIMERB0_VECTOR 
__interrupt void TimerB0_Isr(void)         // ccr0中断服务            
{ 
//    _EINT();
//    _NOP();
  if(0 ==WDogflag)
  {
    WDTCTL= WDTPW + WDTCNTCL;              // 喂狗 
  }
  
   CLK.Time_Cnt_Hart++;
  if(CLK.Time_Cnt_Hart>2700000)//24h
    CLK.Time_Cnt_Hart=0;
  
  
    CLK.Ttem++;                            // 32最大
    
    if(CLK.Ttem%CLK.Message_Load_Rate == 0)    {   Flag_Message = 1 ;       MessageCenter();   } //放中断里是因为 菜单里的功能有死循环
         
    if(CLK.Ttem%CLK.Display_Load_Rate == 0)    { Flag_Display = 1 ;  Display_Data_Calculate();  Alarm_Deal();  Display();  }
    if(CLK.Ttem%2 == 0)                       
    { 
      //CLK.PIDCon_Flag =1;
      if(CLK.OneKey_Flag==1) 
      { 
        CLK.Adj_Flag =1;  
      }
      else CLK.PIDCon_Flag =1;
    }
        
    if(CLK.Ttem == CLK.Second_modulate)    
    {
       CLK.Ttem=0;
       SYS.bit.Dida=~SYS.bit.Dida;
       if(CLK.SecRun) CLK.SecRun--; // 127
       if(CLK.SecKB)  CLK.SecKB--;  // 16
       if(CLK.SecAL)  CLK.SecAL--;  // 127
       if(OP.Rdj_flg) OP.Rdj_cnt++; // 
    }
    

      
    
    if(CLK.Hart_Flag == 1)
    {    
      CLK.SecHT++;
      //if(CLK.SecHT>15)    //延时300ms
      if((OCDIN&0x01) ==0)   //如果OCD被拉低,
      {
        if(CLK.received==1){
        HartMessage();
        CLK.SecHT =0;
        }
        else if(CLK.received==0){
          CLK.Hart_Flag =0;
          rec_ptr=0;
          memset(receive,0,100);
        }
      }
    }
    
    
    if(CLK.received ==2)
    {
      hart_task ();
    }
    
   if(0 ==WDogflag)
   {
    WDTCTL= WDTPW + WDTCNTCL; 
   }
    TBCCR0 = CLK.DIS_SPEED*50;  //20201219 
}

/*
#pragma vector = TIMERB1_VECTOR 
__interrupt void TimerB1_Isr(void)         // ccr0中断服务            
{                     
   unsigned int TE = TBIV;


   if(TE==0x0002)        {       _BIC_SR_IRQ(LPM1_bits);   TBCCR1 = CLK.RUN_SPEED*10;        }
   else if (TE==0x0004)  
   {   
           if (CLK.Timer_Config!=50000)  { CLK.Timer_Config++;  }
           if(TBCCR2 < TBCCR0-50) { TBCCR2 = TBCCR2 + 50;}
           else                   { TBCCR2 = 50;         }
   }
} 
*/
