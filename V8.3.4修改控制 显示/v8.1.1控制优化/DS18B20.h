#ifndef __DS18B20_H__
#define __DS18B20_H__


/* ============================================================
 *  ds18b20.h
 * ============================================================ */
# include <msp430x16x.h>

#include <msp430.h>
#include <stdint.h>

/* ---------- 引脚定义（按你的硬件改） ---------- */
#define IO_18B20_Out    (P1DIR |=  BIT7)
#define IO_18B20_In     (P1DIR &= ~BIT7)
#define DQ1             (P1OUT |=  BIT7)
#define DQ0             (P1OUT &= ~BIT7)
#define Read_18B20_IO   (P1IN & BIT7)

/* ---------- DS18B20 专用延时（不影响全局 delay） ---------- */
#define DS_CPU_F        ((double)3260000)   /* 逻辑分析仪实测反推 */
#define ds_delay_us(x)  __delay_cycles((long)(DS_CPU_F * (double)(x) / 1000000.0))

/* ---------- 对外接口 ---------- */
void     ds_init(void);
uint8_t  ds_reset(void);          /* 1=检测到器件, 0=无应答 */
void     ds_write_byte(uint8_t b);
uint8_t  ds_read_byte(void);
int16_t  ds_read_temp(void);      /* 返回原始值, 单位 1/16 ℃ */
float    ds_read_temp_c(void);    /* 返回摄氏度 */
uint8_t  ds_read_temp_crc(float *temp); /* 带 CRC 校验, 1=成功 */



#endif



