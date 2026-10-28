# include "Device.h"




#define CONSTRAIN(x, min, max) (((x) < (min)) ? (min) : (((x) > (max)) ? (max) : (x)))

//double Error_IP = 0.0f;
//
//int pwm  = 0 ;
//// 定义PID参数
//#define KP 10000.0f  // 比例系数
//#define KI 0.0f      // 积分系数(需要调整)
//#define KD 0.0f    // 微分系数(需要调整)
//
//static float integral = 0;
//static float prev_error = 0;
//// 定义静摩擦阈值
//#define FRICTION_THRESHOLD 0.05f  // 5%的开度
//
//void IP_PidControl(void){
//    ADC12_Convert(1); 
//    LineCalculate(); 
//    
//    g_PIDServiceData.Cur_ValvePoint = (g_dsinAngel_now - g_dsinAngel_down)*1000.0/(g_dsinAngel_up - g_dsinAngel_down);  //计算反馈
//    
//    //计算电流对应的位置 （期望位置）
//    if(AD.CurrentIn < OP.Down_C) {g_PIDServiceData.SetPoint =0;}      //如果电流小于最小
//    
//    else if (AD.CurrentIn > OP.Up_C ){g_PIDServiceData.SetPoint =1000;}
//    
//    else {  g_PIDServiceData.SetPoint =  Formula( AD.CurrentIn );}     //计算控制位置
//    
//    if(g_PIDServiceData.SetPoint > 1000)  {
//        g_PIDServiceData.SetPoint =1000;             //限幅
//    }
//    
//  if( VAL.V_p2_4_ACT == V_REVR) {//定位器反向        set point 为相反
//        g_PIDServiceData.SetPoint = 1000 - g_PIDServiceData.SetPoint;
//   }
//
//   
//   Error_IP =   (g_PIDServiceData.SetPoint - g_PIDServiceData.Cur_ValvePoint) / 1000.0f;
//   
//   // 计算PID各项
//      integral += Error_IP;  // 积分项累积
//      
//      if(fabs(Error_IP) > 0.3) {  // 只在误差较大时积分
//          integral += Error_IP;
//      } else {
//          integral = 0;  // 抗积分饱和
//      }
//
//      float derivative = Error_IP - prev_error;  // 微分项
//      prev_error = Error_IP;
//
//      pwm = KP * Error_IP + KI * integral + KD * derivative;
//      
////      if(fabs(Error_IP) > FRICTION_THRESHOLD) {
////    // 在初始移动时增加额外的脉冲克服静摩擦
////      pwm += (pwm > 0) ? FRICTION_COMP : -FRICTION_COMP;
////        }
//      
//       // 3. 动态脉冲生成（引入积分项）
//    if(fabs(Error_IP) > ( VAL.V_p1_3BRND/ 1000.0f)) {
//        // 比例项 + 积分项
//        pwm =    Error_IP  *  11000  ;
//        
//        // 脉冲宽度限幅
////        if(pulse < 15) pulse = 15;
////        if(pulse > 400) pulse = 400;
////            DAC12_0DAT= 1200  + pwm *1.2;
////            DAC12_1DAT = 0;    //改为反向输出
//        
//        
//        
//        // 方向控制
//        if(Error_IP > 0) {
//              OutPut_Positive(pwm);
////            delay(pulse+11);
////            OutPut_Negetive(0);
//        } else  {
//           
//            
//            OutPut_Negetive(pwm);
////            delay(pulse+11);
////            OutPut_Positive(0);
//        }
//
//    }
//    
//    else {
//      OutPut_Positive(0);
//      
//    }
//    
//}
    double pwm =0.0f ;
    double Error_IP = 0.0f;
    double dynamic_KP =0.0f ;
    /* 状态变量 */
    static float integral = 0;
    static float prev_error = 0;
    static uint8_t direction_changes = 0;
    static uint32_t error_persist_time = 0;
    static uint32_t stationary_time = 0;
    static float prev_pwm = 0;

    double KP_BIG       =   1200.0f  ;  // 大误差比例系数
    double KP_SMALL     =    1000.0f  ;   // 小误差比例系数
  
void IP_PidControl_1(void) {
    /* 参数定义 */

    #define KI              0.0f        // 积分系数
    #define KD              0.0f      // 微分系数
    #define FRICTION_COMP   1200        // 基础摩擦补偿(需实测调整)
    #define DEAD_ZONE       0.01f       // 1%死区
    #define OSCILLATION_THRESHOLD 3     // 振荡检测阈值
    #define MIN_RESPONSE_TIME 200       // 最小响应时间(ms)
    #define CONTROL_PERIOD 10          //控制周期10ms

    ValveRodSpeedNEW(5);
    
    /* 1. 数据采集与基本计算 */
    ADC12_Convert(1);
    LineCalculate();
    g_PIDServiceData.Cur_ValvePoint = (g_dsinAngel_now - g_dsinAngel_down)*1000.0/(g_dsinAngel_up - g_dsinAngel_down);
    
    /* 设定值处理 */
    if(AD.CurrentIn < OP.Down_C) g_PIDServiceData.SetPoint = 0;
    else if(AD.CurrentIn > OP.Up_C) g_PIDServiceData.SetPoint = 1000;
    else g_PIDServiceData.SetPoint = Formula(AD.CurrentIn);
    
    if(g_PIDServiceData.SetPoint > 1000) g_PIDServiceData.SetPoint = 1000;
    if(VAL.V_p2_4_ACT == V_REVR) g_PIDServiceData.SetPoint = 1000 - g_PIDServiceData.SetPoint;
    
    /* 2. 误差计算与动态处理 */
    Error_IP = (g_PIDServiceData.SetPoint - g_PIDServiceData.Cur_ValvePoint) / 1000.0f;
    
    /* 振荡检测 */
//    if((Error_IP * prev_error) < 0) {   //误差变号
//        if(++direction_changes > OSCILLATION_THRESHOLD) {
//          
//            KP_BIG *= 0.7f;  // 自动降增益
//            direction_changes = 0;
//        }
//    } else {
//        direction_changes = 0;
//    }
    
//    /* 3. 动态PID计算 */
//     dynamic_KP = (fabs(Error_IP) > 0.5f) ? KP_BIG   : 
//                      (fabs(Error_IP) > 0.2f) ? (KP_BIG+KP_SMALL)/2 : KP_SMALL;
    
    /* 积分项处理(抗饱和) */
    if(fabs(Error_IP)  <  0.3f) {
        integral += Error_IP;
        integral = CONSTRAIN(integral, -1000, 1000);  // 限制积分范围
    } else {
//        integral *= 0.9f;  // 渐消积分
        integral = 0.0f;  // 渐消积分
    }
    
    /* 微分项处理 */
    float derivative = (Error_IP - prev_error);  // 假设1ms周期s
    
    /* 4. 摩擦补偿策略 */
//    float actual_comp = FRICTION_COMP;
    
//    if(fabs(g_vl) < 2) {
//        stationary_time += CONTROL_PERIOD;
//        if(stationary_time > 1000) actual_comp *= 1.5f;
//        else if(stationary_time > 500) actual_comp *= 1.2f;
//    } else {
//        stationary_time = 0;
//    }
    
    /* 5. 综合输出计算 */
     pwm = KP_BIG * Error_IP + KI * integral + KD * derivative;
    
    /* 小误差区特殊处理 */
//    if(fabs(Error_IP) < 0.03f && fabs(prev_pwm) < FRICTION_COMP*0.8f) {
//        pwm = (Error_IP > 0) ? actual_comp : -actual_comp;
//    }
    
    /* 6. 响应延时和死区处理 */
//    if(fabs(Error_IP) > DEAD_ZONE) {
//        error_persist_time += CONTROL_PERIOD;
//        if(error_persist_time < MIN_RESPONSE_TIME) {
//            pwm = 0;  // 未达到最小响应时间
//        }
//    } else {
//        error_persist_time = 0;
//        pwm = 0;  // 死区内无输出
//    }
    
    /* 7. 输出限幅和保护 */
//    pwm = constrain(pwm, -MAX_PWM, MAX_PWM);
    

     //输出
     
     if(fabs(Error_IP) > VAL.V_p1_3BRND / 1000.0f){
       if(  pwm > 0)
       {
         OutPut_Positive(VAL.V_p5_3__K3 + abs(pwm) );  
       }
        if( pwm < 0  )
       {
         OutPut_Negetive(VAL.V_p5_4__K4+ abs(pwm) );
       }
     }
     else{
        OutPut_Negetive(0);
       
     }
    
     
    /* 状态更新 */
    prev_error = Error_IP;
    prev_pwm = pwm;
    
}
  
  
  
  
  
  
  
  
 