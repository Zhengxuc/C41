# include "Device.h"

PWM_REG PWM;

/* DA标定: V = DAC_VOLT_RATIO * DAC12_0DAT
 * 平衡位1200→1.44V, 1500→1.8V, 2200→2.64V */
#define DA_BALANCE              1200
#define DA_PWM_STEP             1.2
#define DAC_VOLT_RATIO          0.0012
#define PWM_LIMIT               950
#define TEMP_REF                200     /* 参考温度30.0°C, 补偿基准点 */
#define R_CALIB                 1750.0  /* 标定电阻(Ω), 对应TEMP_REF */
#define RESIST_TEMP_COEFF       5.875f  /* 温度每升1°C, 线圈电阻约增5.875Ω */
#define IP_CURRENT_SCALE        1600.0
#define IP_PERCENT_STEP         0.05
#define FAILSAFE_PWM_SCALE      1.25
#define FAILSAFE_CURRENT_SCALE  1666.6

/* 调试变量, 可在调试器观察 */
double DA_Out = 0;
double R_current = 0;
double U_compensate = 0;

static unsigned int ClampPwm(unsigned int pwm)
{
    return (pwm > PWM_LIMIT) ? PWM_LIMIT : pwm;
}

/* 计算当前温度下的线圈电阻
 * R = R标定 + 5.875 × (T - Tref)
 * AD.Temperature 单位0.1°C */
static double CalcCurrentResistance(void)
{
    return R_CALIB + RESIST_TEMP_COEFF * (AD.Temperature - TEMP_REF) / 10.0f;
}

/* 由目标DA值计算电压、温度补偿并写入DAC */
static void OutputByDaValue(double da_out)
{
    DA_Out = da_out;
    AD.Voltage = DAC_VOLT_RATIO * DA_Out;

    R_current = CalcCurrentResistance();
    U_compensate = AD.Voltage * (R_current / R_CALIB);

    DAC12_0DAT = (int)(U_compensate / DAC_VOLT_RATIO);
    DAC12_1DAT = 0;
}

static void OutputFailsafeNegative(unsigned int pwm)
{
    DAC12_1DAT = (int)(pwm * FAILSAFE_PWM_SCALE);
    DAC12_0DAT = 0;
    AD.IP_Current_Out = -(pwm * FAILSAFE_PWM_SCALE / FAILSAFE_CURRENT_SCALE);
    AD.IP_Current_Percent = -pwm * 0.1;
}

static void OutputFailsafePositive(unsigned int pwm)
{
    DAC12_0DAT = (int)(pwm * FAILSAFE_PWM_SCALE);
    DAC12_1DAT = 0;
    AD.IP_Current_Out = pwm * FAILSAFE_PWM_SCALE / FAILSAFE_CURRENT_SCALE;
    AD.IP_Current_Percent = pwm * 0.1;
}

void Control_Initial(void)
{
    TACTL = TASSEL_2 + TACLR;
    P1SEL |= BIT2 + BIT3;
    P1DIR |= BIT2 + BIT3;
    CCR0 = 5000 / (0.136 * PWM.FREQUENCY);
    CCTL1 = OUTMOD_6;
    CCTL2 = OUTMOD_6;
}

/* 反向输出(关阀方向), pwm=0为平衡位 */
void OutPut_Negetive(unsigned int m)
{
    double da_out;

    m = ClampPwm(m);

    if(0 == VAL.V_p8_2_POS)
    {
        da_out = DA_BALANCE - m * DA_PWM_STEP;
        OutputByDaValue(da_out);
        AD.IP_Current_Out = da_out / IP_CURRENT_SCALE;
        AD.IP_Current_Percent = 50.0 - m * IP_PERCENT_STEP;
    }
    else
    {
        OutputFailsafeNegative(m);
    }
}

/* 正向输出(开阀方向), pwm=0为平衡位 */
void OutPut_Positive(unsigned int n)
{
    double da_out;

    n = ClampPwm(n);

    if(0 == VAL.V_p8_2_POS)
    {
        da_out = DA_BALANCE + n * DA_PWM_STEP;
        OutputByDaValue(da_out);
        AD.IP_Current_Out = da_out / IP_CURRENT_SCALE;
        AD.IP_Current_Percent = 50.0 + n * IP_PERCENT_STEP;
    }
    else
    {
        OutputFailsafePositive(n);
    }
}

/* 双向输出, value>0正向 value<0反向 */
void OutPut(int value)
{
    if(value > PWM_LIMIT)  value = PWM_LIMIT;
    if(value < -PWM_LIMIT) value = -PWM_LIMIT;

    OutputByDaValue(DA_BALANCE + value * DA_PWM_STEP);
}
