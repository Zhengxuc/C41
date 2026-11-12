#include "Device.h"




/* ============================================================
 *  ds18b20.c
 * ============================================================ */
#include "ds18b20.h"

/* ---------- 内部：写一位 ---------- */
static void ds_write_bit(uint8_t b)
{
    IO_18B20_Out;
    DQ0;
    if (b) {
        ds_delay_us(6);          /* 拉低 <15us，写 1 */
        DQ1;
        IO_18B20_In;
        ds_delay_us(60);         /* 凑满时隙 ≥60us */
    } else {
        ds_delay_us(60);         /* 拉低 ≥60us，写 0 */
        DQ1;
        IO_18B20_In;
        ds_delay_us(2);          /* 恢复时间 */
    }
}

/* ---------- 内部：读一位 ---------- */
static uint8_t ds_read_bit(void)
{
    IO_18B20_Out;
    DQ0;
    ds_delay_us(2);              /* 拉低 >1us */
    DQ1;
    IO_18B20_In;
    ds_delay_us(10);             /* 采样点约 12us，落在 15us 内 */
    uint8_t b = (Read_18B20_IO != 0);
    ds_delay_us(50);             /* 等时隙结束 */
    return b;
}

/* ---------- 初始化 GPIO ---------- */
void ds_init(void)
{
    P1SEL  &= ~BIT7;             /* 设为普通 GPIO */
    IO_18B20_Out;
    DQ1;                         /* 空闲拉高 */
}

/* ---------- 复位 + 存在脉冲检测 ---------- */
uint8_t ds_reset(void)
{
    uint8_t presence;

    IO_18B20_Out;
    DQ0;
    ds_delay_us(480);            /* 拉低 ≥480us */

    DQ1;
    IO_18B20_In;                 /* 释放，切输入 */
    ds_delay_us(70);             /* 等 70us，落在存在脉冲窗口 */

    presence = (Read_18B20_IO == 0) ? 1 : 0;

    ds_delay_us(400);            /* 等存在脉冲结束 */
    DQ1;
    IO_18B20_Out;                /* 恢复输出高 */
    return presence;
}

/* ---------- 写字节（LSB 先出） ---------- */
void ds_write_byte(uint8_t b)
{
    for (uint8_t i = 0; i < 8; i++) {
        ds_write_bit(b & 0x01);
        b >>= 1;
    }
}

/* ---------- 读字节（LSB 先出） ---------- */
uint8_t ds_read_byte(void)
{
    uint8_t b = 0;
    for (uint8_t i = 0; i < 8; i++) {
        b >>= 1;
        if (ds_read_bit()) b |= 0x80;
    }
    return b;
}

/* ---------- 读原始温度 ---------- */
int16_t ds_read_temp(void)
{
    uint8_t lsb, msb;

    if (!ds_reset()) return 0x8000;   /* 无应答，返回错误标志 */

    ds_write_byte(0xCC);              /* 跳过 ROM */
    ds_write_byte(0x44);              /* 启动转换 */

    /* 12 位分辨率等待 750ms，拆成循环避免 __delay_cycles 溢出 */
    for (uint16_t i = 0; i < 750; i++) {
        ds_delay_us(1000);
    }

    if (!ds_reset()) return 0x8000;
    ds_write_byte(0xCC);
    ds_write_byte(0xBE);              /* 读暂存器 */

    lsb = ds_read_byte();
    msb = ds_read_byte();

    return (int16_t)((msb << 8) | lsb);
}

/* ---------- 读摄氏度 ---------- */
float ds_read_temp_c(void)
{
    int16_t raw = ds_read_temp();
    if (raw == 0x8000) return -999.0f;   /* 错误标志 */
    return raw * 0.0625f;
}

/* ---------- CRC8 校验（可选，推荐） ---------- */
static uint8_t ds_crc8(const uint8_t *data, uint8_t len)
{
    uint8_t crc = 0;
    while (len--) {
        uint8_t inbyte = *data++;
        for (uint8_t i = 0; i < 8; i++) {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

/* ---------- 带 CRC 的读温度 ---------- */
uint8_t ds_read_temp_crc(float *temp)
{
  _DINT();
    uint8_t buf[9];

    if (!ds_reset()) return 0;
    ds_write_byte(0xCC);
    ds_write_byte(0x44);

    for (uint16_t i = 0; i < 750; i++) {
        ds_delay_us(1000);
    }

    if (!ds_reset()) return 0;
    ds_write_byte(0xCC);
    ds_write_byte(0xBE);

    for (uint8_t i = 0; i < 9; i++) {
        buf[i] = ds_read_byte();
    }

    if (ds_crc8(buf, 8) != buf[8]) return 0;   /* CRC 错 */

    int16_t raw = (int16_t)((buf[1] << 8) | buf[0]);
    *temp = raw * 0.0625f;
    return 1;
    _EINT();
}