# include "Device.h"
PWM_REG PWM;

//---pwm输出端口初始化 ，定时器a的初始化
void Control_Initial(void)
{
  
    TACTL = TASSEL_2 + TACLR; 
    P1SEL |= BIT2 + BIT3 ; 
    P1DIR |= BIT2 + BIT3 ;
    //CCR0 = 5000/(0.136*PWM.FREQUENCY); //之前是0.012      
    CCR0 = 5000/(0.136*PWM.FREQUENCY); //之前是0.012     //0.136
    CCTL1 = OUTMOD_6;
    CCTL2 = OUTMOD_6;   
}
/*
void OutPut_Negetive(unsigned int m) 
{
      if(m>950){m=950;}
 //   if(VAL.V_p3_3_RCT ==1)
 //   {
       DAC12_1DAT = m*1.25;  
       DAC12_0DAT = 0; 
 //   }
//    else 
//    {
//       DAC12_0DAT = m*4;  
//       DAC12_1DAT = 0; 
//    }

}
//----------反向电压输出可控子程序-------
void OutPut_Positive( unsigned int n) 
{
      if(n>950){n=950;}
  //  if(VAL.V_p3_3_RCT ==1)
  //  {
       DAC12_0DAT = n*1.25;  
       DAC12_1DAT = 0; 
  //  }
  //  else
  //  {
  //     DAC12_1DAT = n*4;  
  //     DAC12_0DAT = 0; 
  //  }
    
}  
*/

float K_With_Temperature(int temp)
{
  
      float k;

      k=0.98557+0.00367*temp;
      
      if(k<0.5)k=0.5;
      
      if(k>1)k=1;
      
      
      return k;
}


//DA输出 1200 是1.5v 输出2340是2.88   2925是3.588




void OutPut_Negetive(unsigned int m) 
{    
  
//      if(AD.Temperature<0)
//      m= m * K_With_Temperature(AD.Temperature);
//      
      
      if(m>950){m=950;}
     
      if(0 == VAL.V_p8_2_POS)//故障安全
      {

        //零度整体反向都慢
        //-40度的平衡位是0.55mA 1v   0位按比例往下降，0v也能到0位，但是速度慢 此时给反向输出电压  
        //1v对应的da为 787
        
//        if(   AD.Temperature <= 50    )    //低温小于5度
//        {
//          if(   1200 - m * 1.2    -  (  9 *  (float)( 50 - AD.Temperature  )/10    )  > 0 )
//          {
//            DAC12_0DAT=   1200 - m * 1.2    -  (  9 *  (float)( 50 - AD.Temperature  )/10    )   ;
//            DAC12_1DAT =  0  ;    //改为反向输出     
//            
//          }
//          else if(   1200 - m * 1.2    -  (  9 *  (float)( 50 - AD.Temperature  )/10    )  <= 0   )
//          {
//            DAC12_0DAT=   0   ;
//            DAC12_1DAT =  - (1200 - m * 1.2    -  (  9 *  (float)( 50 - AD.Temperature  )/10 )   ) ;    //改为反向输出  
//            
//          }
//            
//        }
//        
//        
//        else if(AD.Temperature > 50 && AD.Temperature < 350  )   //不动
//        {
            DAC12_0DAT= 1200 - m*1.2;
            DAC12_1DAT = 0;    //改为反向输出
//        }
//        
//        else if(AD.Temperature >= 350  )
//         {
//          DAC12_0DAT= (1200 - m*1.2)  +    (0.33 * (float)((AD.Temperature - 350)/10) )  ;
////            DAC12_0DAT= (1200 - m*1.2)  +    (int)( 13 * (float)((AD.Temperature - 350)/10) )  ;   //这里是否要修改？
//            DAC12_1DAT = 0;           
//         }
        
             
//         else if(AD.Temperature <350  && ucNeFlg == 0 )
//         {    
//           //老化的问题      给0v 动作缓慢, 还是能到0位,
//           
//           
//           
////           if( m  ==  950)
////           {
////            DAC12_0DAT = 0 ;
////            DAC12_1DAT = 700;    //改为反向输出       
////           }
////           else 
////           {
//           
//           
//            DAC12_0DAT= 1200 - m*1.2;
//            DAC12_1DAT = 0;    //改为反向输出
//            
////           }
//           
//           
//           
//
//         }
  
      AD.IP_Current_Out = (1200 - m*1.2)/1600.0;//IP电流
      AD.IP_Current_Percent = 50.0-m*0.05;
      }

      else if(1 == VAL.V_p8_2_POS) //故障闭锁
      {
        DAC12_1DAT = m*1.25;  
        DAC12_0DAT = 0;
        AD.IP_Current_Out = -((m*1.25)/1666.6);
        AD.IP_Current_Percent = -m*0.1;    
      }
}


//低温排气变小    //  0.75mA默认平衡     out1不出气   电流变大能出气 ,         
                      
//  现在低温0.55mA出气
// -40  0.55mA     平衡位置降至 1V
//  25  0.75mA     平衡位置为1.5V


//----------反向电压输出可控子程序-------
void OutPut_Positive( unsigned int n) 
{
//      if(AD.Temperature<0)
//      n=n*K_With_Temperature(AD.Temperature);
//  


      if(n>950){n=950;}//  充气限定0.75
    
      
      if(0 == VAL.V_p8_2_POS)//故障安全
      {  
//       DAC12_0DAT =1200 + n*1.2;
//       DAC12_0DAT =1400 + n*1.4;

        
        
//        if(AD.Temperature <=50)
//        {
//            DAC12_0DAT= 1200  +  n *1.2     -  (  9 * (float) ( 50 - AD.Temperature  )/10  )  ;   //   1200 +  1800  = 3000   =  3v
//            DAC12_1DAT = 0; 
//        }  
//        
//        else if(AD.Temperature <=350 &&  AD.Temperature  > 50  )     //低温区不变
//         { 
            DAC12_0DAT= 1200  +  n*1.2;   //   1200 +  1800  = 3000   =  3v
            DAC12_1DAT = 0;
            
//         }
//        
//        
//        else if(AD.Temperature >350 )
//         {
//            DAC12_0DAT= (1200     +   n*1.2) +   (int) ( 13 *( (float) ((  AD.Temperature - 350  ) / 10 ))  ) ;          
//            DAC12_1DAT = 0;           
//           
//         }

         
       AD.IP_Current_Out= (1200 + n*1.2)/1600.0;
       
       AD.IP_Current_Percent = 50.0+n*0.05;
       
       
      }
      
      else if(1 == VAL.V_p8_2_POS) //故障闭锁
      {
       DAC12_0DAT = n*1.25;  
       DAC12_1DAT = 0;
       AD.IP_Current_Out = (n*1.25)/1666.6;
       AD.IP_Current_Percent = n*0.1;  
       
      }
      
      
}  


void OutPut(   int  value){

  if(value > 950)
  {
    value = 950;
  }
  if(value < -950){
    
    value = -950;
  }
DAC12_0DAT= 1200  +  value*1.2;   //   1200 +  1800  = 3000   =  3v
DAC12_1DAT = 0; 

}



