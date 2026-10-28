//*************************************************************************************************
//  Module Name   :  main
//  Target Device :  MSP430F149 
//  CreateDate    :  2007-10-4
//  ModifData     :  2009-6-28
//  Description   :  智能阀门定位器
//                 
//  Author        :  
//  Version       :  V 6.28
//*************************************************************************************************

//*************************************************************************************************
//  Version Description      :  V 6.14     版本修改说明
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  1.  对键盘操作做修改                                               4  
//  2.  增加组态步骤功能显示  
//  3.  P2.3 组态增加 CUST
//  4.  P2 组态  2.8  2.9    
//  5.  P5 组态增加 5.9  5.10  5.11
//  6.  P8 组态增加 8.6 8.7                         
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  7.  报警显示修改   在自动模式下                  (闪烁显示)         4
//  8.  自动模式下增加 混合控制  PID控制  自适应控制                    4
//  9.  在每个组态下面增加退出保存功能                                  4
//  10. 自动控制中的严重错误下的功能上锁并显示                          4
//*************************************************************************************************

//*************************************************************************************************
//  Version Description      :  V 6.14     版本修改说明
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//  1.  P6 组态  6.6  6.7  
//  2.  修改 负号显示  
//  3.  修改边界问题                   
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//-------------------------------------------------------------------------------------------------
//                                            Includes
//-------------------------------------------------------------------------------------------------
# include "Device.H"
extern const unsigned int Cv_Fit[][17];
//-------------------------------------------------------------------------------------------------
//                                           Definition
//-------------------------------------------------------------------------------------------------
# define DIS_LOAD_SPEED 10         //  DIS 负载率   从1-30级可调   1级最重 速度最快  30速度最慢
# define RUN_LOAD_SPEED 100         //  RUN 负载率   从1-255级可调   1级10 速度最快  30速度 2550
//--------------------------------------------------------------------------------------------------

/*************************************************
*		函数名：CTRInit
*		说  明	PID控制初始化					
*		日  期：2015.6.16
*************************************************/
void CTRInit(void)
{
    g_PIDServiceData.SumDeltaU=0;  
    g_PIDServiceData.LastError=0;
    g_PIDServiceData.Prev_Error=0;
    
//    RelayReadyflag =0;
//    ValveMoveDIRflag=0;
    CLK.AdjReady =0;
   // PWMduty_ZDF =800; // 曾用过
   // PWMduty_ZDS =800;

}

/*************************************************
*		函数名：TestInit
*		说  明 为测试PID控制程序做的初始化					
*		日  期：2015.6.16
*************************************************/
void TestInit(void)
{
  /*
    g_PIDServiceData.Kp=5;   //20151222 10
    g_PIDServiceData.Ki=1;
    g_PIDServiceData.Kd=2;
  
    g_PIDServiceData.Transform= 20;  //  1222 10
    g_DeviceFeature.ValveRodSpeed1=1; //1222 3   50  4 
    g_DeviceFeature.ValveRodSpeed2=2; //1222 2   50  3
    g_DeviceFeature.FastSector_Lower=30;
    g_DeviceFeature.SlowSector_Lower=20;
    g_DeviceFeature.DeadSevtion=10;   //1222 4
       
  
//    g_PWM_Duty.Fast_Gasfilled=480;   //    ;PWM_duty 0.48
//    g_PWM_Duty.Fast_Outgassing=490;  //   ;... 0.49
//    g_PWM_Duty.Slow_Gasfilled=430;   //   ;... 0.43
//    g_PWM_Duty.Slow_Outgassing=440;  //   ;... 0.44
//    g_PWM_Duty.Weak_Gasfilled=410;   //   ;... 0.41
//    g_PWM_Duty.Weak_Outgassing=420;  //   ;... 0.42
    */


           
    //g_PWM_Duty.Slow_Gasfilled = VAL.V_p5_1__K1;   //   ;... 0.43
    //g_PWM_Duty.Slow_Outgassing = VAL.V_p5_2__K2;  //   ;... 0.44
    //g_PWM_Duty.Weak_Gasfilled = VAL.V_p5_3__K3;   //   ;... 0.41
    //g_PWM_Duty.Weak_Outgassing = VAL.V_p5_4__K4 ;  //   ;... 0.42    
    
    //g_PIDServiceData.Kp = VAL.V_p5_5__K5;   //20151222 10
    //g_PIDServiceData.Ki = VAL.V_p5_6__K6;     //3
    //g_PIDServiceData.Kd = VAL.V_p5_7__K7;     //2
 /*    
    g_PIDServiceData.Transform =  VAL.V_p5_8__K8;   //20  1222 10
    g_DeviceFeature.ValveRodSpeed1 = VAL.V_p5_9__K9; //1222 3   50  4 
    g_DeviceFeature.ValveRodSpeed2 = VAL.V_p5_10_K0; //1222 2   50  3
    g_DeviceFeature.FastSector_Lower=VAL.V_p5_7__K7;  //30   50
    
*/    
    //g_DeviceFeature.SlowSector_Lower=40;   //20    40
    //g_DeviceFeature.DeadSevtion=VAL.V_p1_3BRND;   //1222 4
 
     //g_DeviceFeature.FastSector_Lower= VAL.V_p5_5__K5*1000.0/OP.Range_M;  //30   50
   //  g_PIDServiceData.Kp =VAL.V_p5_6__K6; 
   //  g_PIDServiceData.Ki =VAL.V_p5_7__K7;
  //   g_PIDServiceData.Transform =  VAL.V_p5_8__K8;   //20  1222 10
   //  g_DeviceFeature.ValveRodSpeed1 = VAL.V_p5_9__K9; //1222 3   50  4 
   //  g_DeviceFeature.ValveRodSpeed2 = VAL.V_p5_10_K0; //1222 2   50  3       
} 



//------------------------------------------------------------------------------
//   FUNCTIONS
//------------------------------------------------------------------------------
//  模块初始化
//------------------------------------------------------------------------------
void Device_initial(void)
{ 
    _DINT();
    WDTCTL = WDTPW + WDTHOLD;  
    //----------------------
    //PWM.FREQUENCY = 100;
    PWM.FREQUENCY = 100;
    //----------------------
    Cpu_Tick_Initial();
    Keyboard_Initial();
    AD421_Initial();
    ADC12_Initial();
    HT1621_Initial();
    Control_Initial();
    Uart_Initial ();
    TestInit();
    CTRInit();
    //Dis_LOGO ();  //显示MORC
    //-----------------------
    _EINT();                   
}

//------------------------------------------------------------------------------
// 系统初始化
//------------------------------------------------------------------------------
void Sys_initial(void) //初始化
{  
  unsigned char i;
    _DINT();
 #if  WRITE_FACTORY  
    Factory_Start();
 #endif
    Read_SegA ();                
    _NOP();       
    
    SYS.bit.Step =0;    
   //------------------------
   //SYS.bit.PageStateL = AUTO;
   //SYS.bit.PageStateF = AUTO;
   //------------------------
   CLK.DIS_SPEED = DIS_LOAD_SPEED;
   CLK.RUN_SPEED = RUN_LOAD_SPEED;
   
   CLK.Second_modulate = 540/CLK.DIS_SPEED;
   
//   CLK.Message_Load_Rate = CLK.Second_modulate/4-1;   // 消息处理慢
//   CLK.Display_Load_Rate = CLK.Second_modulate/3-1;   // 显示快 
    CLK.Message_Load_Rate = CLK.Second_modulate/3-1;   // 消息处理慢
   CLK.Display_Load_Rate = CLK.Second_modulate/2-1;   // 显示快   
   
   
   //------------------------
   DIS.Alarm_Dis[0] = 14;
   DIS.Alarm_Dis[2] = 36; 
   DIS.Alarm_Dis[3] = 0;
   ALA.bit.KB__FLAG = 1;  //报警初始化
   //------------------------
   OP.Range_C = (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*(VAL.V_p2_2_MAX - VAL.V_p2_1_MIN);  //计算电流的全程标定    这里要做检查
   OP.Up_C = VAL.V_p8_5_MAX - (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*(200.0-VAL.V_p2_2_MAX);    //计算电流的标定最大值
   OP.Down_C = VAL.V_p8_4_MIN + (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*(VAL.V_p2_1_MIN-40.0);  //计算电流标定的最小值   
   OP.Up_C_alarm = VAL.V_p8_4_MIN + (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*(VAL.V_p6_7_MAX-40.0);    //计算电流的标定最大值
   OP.Down_C_alarm = VAL.V_p8_5_MAX -(VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*(200.0-VAL.V_p6_6_MIN);  //计算电流标定的最小值  
//   OP.Up_C = (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*VAL.V_p2_2_MAX;    //计算电流的标定最大值
//   OP.Down_C = (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*VAL.V_p2_1_MIN;  //计算电流标定的最小值
   
//   OP.Up_C_alarm = (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*VAL.V_p6_7_MAX;    //计算电流的标定最大值
//   OP.Down_C_alarm = (VAL.V_p8_5_MAX - VAL.V_p8_4_MIN)/160.0*VAL.V_p6_6_MIN;  //计算电流标定的最小值
   
   for( i=0;i<17;i++ )
   {  
      VAL.Fit_c[i] = OP.Down_C + (float)OP.Range_C/(16.0)*i;    //VAL.Fit_c[i] = OP.Down_C + OP.Range_C/16.0*i; 
   }   
   VAL.Fit_c[17] = (float)OP.Range_C/(16.0); 
   //---------------------------------------------------------------------------
   VAL.Cust_c[0] = VAL.V_p2_8_ST1;
   VAL.Cust_c[1] = VAL.V_p2_8_ST2;
   VAL.Cust_c[2] = VAL.V_p2_8_ST3;
   VAL.Cust_c[3] = VAL.V_p2_8_ST4;
   VAL.Cust_c[4] = VAL.V_p2_8_ST5;
   VAL.Cust_c[5] = VAL.V_p2_8_ST6;
   VAL.Cust_c[6] = VAL.V_p2_8_ST7;
   VAL.Cust_c[7] = VAL.V_p2_8_ST8;
   VAL.Cust_c[8] = VAL.V_p2_8_ST9;
   VAL.Cust_c[9] = VAL.V_p2_8_ST10;
   VAL.Cust_c[10] = VAL.V_p2_8_ST11;
   VAL.Cust_c[11] = VAL.V_p2_8_ST12;
   VAL.Cust_c[12] = VAL.V_p2_8_ST13;
   VAL.Cust_c[13] = VAL.V_p2_8_ST14;
   VAL.Cust_c[14] = VAL.V_p2_8_ST15;
   VAL.Cust_c[15] = VAL.V_p2_8_ST16;
   VAL.Cust_c[16] = VAL.V_p2_8_ST17;
   //--------------------------------------------------------------------------- 

//   g_dangel_down = OP.Down_Limited;   
//   g_dangel_down = (double)(g_dangel_down - 2048)/4096*3.141592653; 
//   g_dsinAngel_down = sin(g_dangel_down); 
//   
//
//   g_dangel_up = OP.Up_Limited;
//   g_dangel_up = (double)(g_dangel_up - 2048)/4096*3.141592653;         
//   g_dsinAngel_up = sin(g_dangel_up);  
   UpCalculate();   
   DownCalculate();    
   OP.Range_M = OP.Up_Limited - OP.Down_Limited;                        // 实际机械行程范围
   
   //OP.Machine_Up   = 4095 * ( 1 - MCH_SEN_PER)/2 + 4095 * MCH_SEN_PER * VAL.V_p3_2_MAX / 1000.0 ;  //机械行程的极限位置 这个值是提醒用户要注意安装范围的
   //OP.Machine_Down = 4095 * ( 1 - MCH_SEN_PER)/2 + 4095 * MCH_SEN_PER * VAL.V_p3_1_MIN / 1000.0 ;  //机械行程的极限位置
      
   //OP.Machine_Up   = 4095.0 * ( VAL.V_p3_2_MAX/1000.0); 
   //OP.Machine_Down = 4095.0 * ( VAL.V_p3_1_MIN/1000.0); 
   
  OP.Down_4_1 = OP.Down_Limited;     //20221006
  OP.Up_4_2 = OP.Range_M + OP.Down_Limited;       //20221006
//   OP.Down_4_1 = OP.Range_M*(VAL.V_p4_1_MIN/1000.0) + OP.Down_Limited;     //控制行程最小 这个百分比是按照机械行程范围计算的
//   OP.Up_4_2 = OP.Range_M*(VAL.V_p4_2_MAX/1000.0) + OP.Down_Limited;       //控制行程最大 这个百分比是按照机械行程范围计算的
   OP.Middle_Valve = ( OP.Down_4_1 + OP.Up_4_2 )/2;
   OP.Range_D = OP.Up_4_2 - OP.Down_4_1;
   //---------------------------------------------------------------------------0位置超差计算
   if ( ( 4095 - OP.Up_4_2 ) > OP.Down_4_1 )  { OP.Up_50 = (float)Cv_Fit[ VAL.V_p2_3_CHR ][ 8 ]*4.095;}                            // - (4095 - OP.Up_4_2 - OP.Down_4_1);    }
   else                                       { OP.Up_50 = (float)Cv_Fit[ VAL.V_p2_3_CHR ][ 8 ]*4.095*( OP.Up_4_2 - OP.Down_4_1);} // + OP.Down_4_1 - ( 4095 - OP.Up_4_2 );  }
   //---------------------------------------------------------------------------   
   OP.Down_2_5 = OP.Range_D*(VAL.V_p2_5_QOF/1000.0) + OP.Down_4_1;    // 速关最小
   OP.Up_2_5 = OP.Up_4_2 - OP.Range_D*(VAL.V_p2_5_QOF/1000.0);        // 速关最大
   
   OP.BRND_1_3 = OP.Range_D/1000.0*VAL.V_p1_3BRND;                    // 控制精度
   OP.BRND = OP.Range_D/1000.0*2.0;                                   // 关死范围控制
   
   //-------------------------------------------------------------------------------从这里开始启动项目
   switch( VAL.V_p8_6_POP )  
   {
      case V_AT_R : SYS.bit.PageStateF = SYS.bit.PageStateL = AUTO; SYS.bit.PageS1 = 0; DIS.SignR |= MODE_PERC; break;// AUTO_RDJ 
      case V_AT_P : SYS.bit.PageStateF = SYS.bit.PageStateL = AUTO; SYS.bit.PageS1 = 1; DIS.SignR |= MODE_PERC; break;// AUTO_PID 
      case V_AT_M : SYS.bit.PageStateF = SYS.bit.PageStateL = AUTO; SYS.bit.PageS1 = 2; DIS.SignR |= MODE_PERC; break;// AUTO_MIX 
      case V_MA_P : SYS.bit.PageStateF = SYS.bit.PageStateL = MAN;  SYS.bit.PageS1 = 0; DIS.SignR |= MODE_PERC; break;// MAN_PEN 
      case V_MA_S : SYS.bit.PageStateF = SYS.bit.PageStateL = MAN;  SYS.bit.PageS1 = 1;  break;// MAN_SEN 
      default:;break;
   }  
   ALA.bit.KB__FLAG = 1;
   CLK.OneKey_Flag =0;
   WDogflag = 0;
   //---------------------------------------------------------------------------
   TBCTL |= MC_1;
   WDTCTL = WDT_MRST_32;   //SMCLK/32768     
   _EINT(); 
}


//------------------------------------------------------------------------------
//                                    主函数
//------------------------------------------------------------------------------



void main( void )
{
      
      
      Device_initial();
      Sys_initial();

      for(;;)
      {  
         //_BIS_SR(LPM1_bits+GIE);
         _NOP();
//          TestInit();          

          
           if(Flag_Message == 1)
          {
            Flag_Message = 0 ;  
//            Config_IP();
             SEND_AD421();
          }
          // 主循环中


   
         if(Flag_Display == 1)
         {
           Flag_Display = 0; 
         }
              
         
          if(CLK.received ==0)
          {
#if  WRITE_FACTORY 
            ;
#else              
             if(CLK.PIDCon_Flag ==1)
             {
                 RunControl();
                 CLK.PIDCon_Flag =0;
             }          
             if(CLK.Adj_Flag ==1)
             {
//                 if(1 == VAL.V_p6_4__VR)//高摩擦力
//                 {
                  P1_2_Rdj_H();
//                 }
//                 else
//                 {
//                  P1_2_Rdj();                 
//                 }
                 CLK.Adj_Flag =0;
             } 
#endif             
          }
          
          
        // hart_task (); 
          
          
      }
}
//------------------------------------------------------------------------------
//                                EOF
//------------------------------------------------------------------------------
