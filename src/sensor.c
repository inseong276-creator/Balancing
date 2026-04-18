#include "sensor.h"
#include "serial.h"
#include <math.h>

#define MPU6050_ADDR          0x68
#define MPU6050_PWR_MGMT_1    0x6B
#define MPU6050_ACCEL_XOUT_H  0x3B

#define RAD_TO_DEG            57.2957795f
#define GYRO_SENS_250DPS      131.0f
//#define DT_SEC                0.01f

#define KP                    60.0f
#define KI                    0.0f
#define KD                    1.2f
#define PWM_MAX               900.0f
#define I2C_SR1_BERR          (1UL << 8)
#define I2C_SR1_ARLO          (1UL << 9)
#define I2C_SR1_AF            (1UL << 10)

static float integral = 0.0f;

typedef struct {
    const char *op;
    const char *step;
    uint8_t dev_addr;
    uint8_t reg;
    uint8_t index;
    uint8_t scl_level;
    uint8_t sda_level;
    uint32_t sr1_seen;
    uint32_t sr1;
    uint32_t sr2;
} I2C2_ErrorInfo;

static I2C2_ErrorInfo g_i2c2_last_error = {
    "none", "none", 0U, 0U, 0xFFU, 0U, 0U, 0U, 0U, 0U
};

static uint32_t g_i2c2_sr1_seen = 0U;

static void delay_loop(volatile uint32_t count)
{
    while (count--) {
        __asm volatile("nop");
    }
}

static void Serial_WriteHex4(uint8_t value)
{
    char c;

    value &= 0x0FU;
    c = (char)(value < 10U ? ('0' + (char)value) : ('A' + (char)(value - 10U)));
    Serial_WriteChar(c);
}

static void Serial_WriteHex8(uint8_t value)
{
    Serial_WriteHex4((uint8_t)(value >> 4));
    Serial_WriteHex4(value);
}

static void Serial_WriteHex16(uint16_t value)
{
    Serial_WriteHex8((uint8_t)(value >> 8));
    Serial_WriteHex8((uint8_t)value);
}

static void I2C2_SetError(const char *op,
                          const char *step,
                          uint8_t dev_addr,
                          uint8_t reg,
                          uint8_t index)
{
    g_i2c2_last_error.op = op;
    g_i2c2_last_error.step = step;
    g_i2c2_last_error.dev_addr = dev_addr;
    g_i2c2_last_error.reg = reg;
    g_i2c2_last_error.index = index;
    g_i2c2_last_error.scl_level = (uint8_t)((GPIOB->IDR >> 10) & 0x1U);
    g_i2c2_last_error.sda_level = (uint8_t)((GPIOB->IDR >> 11) & 0x1U);
    g_i2c2_last_error.sr1_seen = g_i2c2_sr1_seen;
    g_i2c2_last_error.sr1 = I2C2->SR1;
    g_i2c2_last_error.sr2 = I2C2->SR2;
}

static void I2C2_ClearError(void)
{
    g_i2c2_last_error.op = "none";
    g_i2c2_last_error.step = "none";
    g_i2c2_last_error.dev_addr = 0U;
    g_i2c2_last_error.reg = 0U;
    g_i2c2_last_error.index = 0xFFU;
    g_i2c2_last_error.scl_level = 0U;
    g_i2c2_last_error.sda_level = 0U;
    g_i2c2_last_error.sr1_seen = 0U;
    g_i2c2_last_error.sr1 = 0U;
    g_i2c2_last_error.sr2 = 0U;
    g_i2c2_sr1_seen = 0U;
}

static void I2C2_PrintFlag(const char *name)
{
    Serial_WriteChar(' ');
    Serial_WriteString(name);
}

static void I2C2_PrintBusState(const char *stage)
{
    Serial_WriteString("I2C RECOVER ");
    Serial_WriteString(stage);
    Serial_WriteString(" SCL=");
    Serial_WriteInt((GPIOB->IDR >> 10) & 0x1U);
    Serial_WriteString(" SDA=");
    Serial_WriteInt((GPIOB->IDR >> 11) & 0x1U);
    Serial_WriteString(" SR1=0x");
    Serial_WriteHex16((uint16_t)I2C2->SR1);
    Serial_WriteString(" SR2=0x");
    Serial_WriteHex16((uint16_t)I2C2->SR2);
    Serial_WriteString("\r\n");
}

static void I2C2_PrintDiagnosis(void)
{
    Serial_WriteString(" cause=");

    if (g_i2c2_last_error.scl_level == 0U || g_i2c2_last_error.sda_level == 0U) {
        Serial_WriteString("bus_line_stuck_low");
        return;
    }

    if ((g_i2c2_last_error.sr1 & I2C_SR1_AF) != 0U) {
        Serial_WriteString("no_ack_sensor_or_addr");
        return;
    }

    if ((g_i2c2_last_error.sr1 & I2C_SR1_ARLO) != 0U) {
        Serial_WriteString("arbitration_lost_or_bus_contention");
        return;
    }

    if ((g_i2c2_last_error.sr1 & I2C_SR1_BERR) != 0U) {
        Serial_WriteString("bus_protocol_error_or_noise");
        return;
    }

    if ((g_i2c2_last_error.sr2 & I2C_SR2_BUSY) != 0U) {
        Serial_WriteString("bus_busy_stuck");
        return;
    }

    Serial_WriteString("timeout_without_error_flag");
}

/* ---------------- I2C2 low-level ---------------- */

static int wait_set(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t timeout = 100000U;

    while (((*reg) & mask) == 0U) {
        if (--timeout == 0U) {
            return -1;
        }
    }
    return 0;
}

static int wait_clear(volatile uint32_t *reg, uint32_t mask)
{
    uint32_t timeout = 100000U;

    while (((*reg) & mask) != 0U) {
        if (--timeout == 0U) {
            return -1;
        }
    }
    return 0;
}

static void I2C2_GPIO_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
    RCC->APB1ENR |= RCC_APB1ENR_I2C2EN;

    /* PB10 = I2C2_SCL, PB11 = I2C2_SDA : AF Open-Drain 50MHz */
    GPIOB->CRH &= ~(0xFUL << 8);
    GPIOB->CRH |=  (0xFUL << 8);

    GPIOB->CRH &= ~(0xFUL << 12);
    GPIOB->CRH |=  (0xFUL << 12);
}

static void I2C2_InitHw(void)
{
    I2C2_GPIO_Init();

    I2C2->CR1 |= I2C_CR1_SWRST;     /*software reset*/
    I2C2->CR1 &= ~I2C_CR1_SWRST;

    I2C2->CR2 = 36;
    I2C2->CCR = 180;
    I2C2->TRISE = 37;

    I2C2->CR1 |= I2C_CR1_ACK;
    I2C2->CR1 |= I2C_CR1_PE;
}

void Sensor_I2C2_Init(void)
{
    I2C2_InitHw();
}

static void I2C2_DeInit(void)
{
    I2C2->CR1 &= ~I2C_CR1_PE;
    RCC->APB1RSTR |= RCC_APB1RSTR_I2C2RST;
    delay_loop(2000);
    RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C2RST;
    delay_loop(2000);
    I2C2->CR1 |= I2C_CR1_SWRST;
    delay_loop(2000);
    I2C2->CR1 &= ~I2C_CR1_SWRST;
}

static void I2C2_BusClear(void)
{
    int i;

    /* PB10=SCL, PB11=SDA 를 GPIO open-drain output으로 변경
       STM32F103 CRH:
       MODE=11 (50MHz output), CNF=01 (General purpose open-drain)
       => nibble = 0x7
    */
    GPIOB->CRH &= ~(0xFUL << 8);   /* PB10 */
    GPIOB->CRH |=  (0x7UL << 8);

    GPIOB->CRH &= ~(0xFUL << 12);  /* PB11 */
    GPIOB->CRH |=  (0x7UL << 12);

    /* 3) 둘 다 High로 release
       open-drain 출력에서는 ODR=1 이면 line을 놓고,
       pull-up에 의해 High가 됩니다.
    */
    GPIOB->BSRR = (1UL << 10) | (1UL << 11);
    delay_loop(2000);

    /* 4) SCL 9 pulse
       SDA는 건드리지 않고 release 상태 유지
    */
    for (i = 0; i < 9; i++) {
        /* SCL Low */
        GPIOB->BRR = (1UL << 10);
        delay_loop(2000);

        /* SCL High */
        GPIOB->BSRR = (1UL << 10);
        delay_loop(2000);
    }

    /* 5) STOP condition 강제 생성
       SCL High 상태에서 SDA Low -> High
       open-drain이라 SDA Low는 BRR, release는 BSRR
    */
    GPIOB->BSRR = (1UL << 10);   /* SCL High */
    delay_loop(2000);

    GPIOB->BRR  = (1UL << 11);   /* SDA Low */
    delay_loop(2000);

    GPIOB->BSRR = (1UL << 10);   /* SCL High 유지 */
    delay_loop(2000);

    GPIOB->BSRR = (1UL << 11);   /* SDA High -> STOP */
    delay_loop(2000);
}

static void I2C2_Recover(void)
{
    I2C2_PrintBusState("before");
    I2C2_DeInit();
    I2C2_PrintBusState("after_deinit");
    I2C2_BusClear();
    I2C2_PrintBusState("after_bus_clear");
    I2C2_InitHw();
    I2C2_PrintBusState("after_init");
    delay_loop(2000);
}



static int I2C2_Start(void)
{
    I2C2->CR1 |= I2C_CR1_START;
    return wait_set(&I2C2->SR1, I2C_SR1_SB);
}

static void I2C2_Stop(void)
{
    I2C2->CR1 |= I2C_CR1_STOP;
}

static int I2C2_SendAddress(uint8_t addr)
{
    volatile uint32_t temp;
    uint32_t sr1;
    uint32_t timeout = 100000U;

    g_i2c2_sr1_seen = 0U;

    I2C2->DR = addr;

    while (timeout-- != 0U) {
        sr1 = I2C2->SR1;
        g_i2c2_sr1_seen |= sr1 & (I2C_SR1_ADDR | I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO);

        if ((sr1 & I2C_SR1_ADDR) != 0U) {
            temp = I2C2->SR1;
            temp = I2C2->SR2;
            (void)temp;
            return 0;
        }

        if ((sr1 & (I2C_SR1_AF | I2C_SR1_BERR | I2C_SR1_ARLO)) != 0U) {
            return -1;
        }
    }

    return -1;
}

static int I2C2_WriteByte(uint8_t data)
{
    if (wait_set(&I2C2->SR1, I2C_SR1_TXE) < 0) {
        return -1;
    }

    I2C2->DR = data;
    return 0;
}

static int I2C2_ReadByte_Ack(uint8_t *data)
{
    if (wait_set(&I2C2->SR1, I2C_SR1_RXNE) < 0) {
        return -1;
    }

    *data = (uint8_t)I2C2->DR;
    return 0;
}

static int I2C2_ReadByte_Nack(uint8_t *data)
{
    I2C2->CR1 &= ~I2C_CR1_ACK;

    if (wait_set(&I2C2->SR1, I2C_SR1_RXNE) < 0) {
        return -1;
    }

    *data = (uint8_t)I2C2->DR;
    return 0;
}

static int I2C2_WriteReg(uint8_t dev_addr, uint8_t reg, uint8_t data)
{
    if (wait_clear(&I2C2->SR2, I2C_SR2_BUSY) < 0) {
        I2C2_SetError("write_reg", "wait_busy_clear", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_Start() < 0) {
        I2C2_SetError("write_reg", "start", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_SendAddress((uint8_t)((dev_addr << 1) | 0U)) < 0) {
        I2C2_SetError("write_reg", "send_addr_w", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_WriteByte(reg) < 0) {
        I2C2_SetError("write_reg", "write_reg_addr", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_WriteByte(data) < 0) {
        I2C2_SetError("write_reg", "write_data", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (wait_set(&I2C2->SR1, I2C_SR1_BTF) < 0) {
        I2C2_SetError("write_reg", "wait_btf", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    I2C2_Stop();
    I2C2_ClearError();
    return 0;
}

static int I2C2_ReadRegs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    volatile uint32_t temp;
    uint8_t i;

    if (wait_clear(&I2C2->SR2, I2C_SR2_BUSY) < 0) {
        I2C2_SetError("read_regs", "wait_busy_clear", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_Start() < 0) {
        I2C2_SetError("read_regs", "start_w", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_SendAddress((uint8_t)((dev_addr << 1) | 0U)) < 0) {
        I2C2_SetError("read_regs", "send_addr_w", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_WriteByte(reg) < 0) {
        I2C2_SetError("read_regs", "write_reg_addr", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (wait_set(&I2C2->SR1, I2C_SR1_BTF) < 0) {
        I2C2_SetError("read_regs", "wait_btf_before_restart", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (I2C2_Start() < 0) {
        I2C2_SetError("read_regs", "restart_r", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    I2C2->DR = (uint8_t)((dev_addr << 1) | 1U);

    if (wait_set(&I2C2->SR1, I2C_SR1_ADDR) < 0) {
        I2C2_SetError("read_regs", "send_addr_r", dev_addr, reg, 0xFFU);
        I2C2_Recover();
        return -1;
    }

    if (len == 1U) {
        I2C2->CR1 &= ~I2C_CR1_ACK;
        temp = I2C2->SR1;
        temp = I2C2->SR2;
        (void)temp;

        I2C2_Stop();

        if (I2C2_ReadByte_Nack(&buf[0]) < 0) {
            I2C2_SetError("read_regs", "read_last_byte", dev_addr, reg, 0U);
            I2C2_Recover();
            return -1;
        }
    } else {
        I2C2->CR1 |= I2C_CR1_ACK;
        temp = I2C2->SR1;
        temp = I2C2->SR2;
        (void)temp;

        for (i = 0; i < len; i++) {
            if (i == (uint8_t)(len - 1U)) {
                I2C2->CR1 &= ~I2C_CR1_ACK;
                I2C2_Stop();

                if (I2C2_ReadByte_Nack(&buf[i]) < 0) {
                    I2C2_SetError("read_regs", "read_last_byte", dev_addr, reg, i);
                    I2C2_Recover();
                    return -1;
                }
            } else {
                if (I2C2_ReadByte_Ack(&buf[i]) < 0) {
                    I2C2_SetError("read_regs", "read_byte", dev_addr, reg, i);
                    I2C2_Recover();
                    return -1;
                }
            }
        }
    }

    I2C2->CR1 |= I2C_CR1_ACK;
    I2C2_ClearError();
    return 0;
}

/* ---------------- MPU6050 ---------------- */

void Sensor_FilterReset(float pitch_init)
{
    (void)pitch_init;
}

int Sensor_MPU6050_Init(void)
{
    return I2C2_WriteReg(MPU6050_ADDR, MPU6050_PWR_MGMT_1, 0x00);
}

int Sensor_MPU6050_ReadRaw(MPU6050_RawData *raw)
{
    uint8_t data[14];

    if (I2C2_ReadRegs(MPU6050_ADDR, MPU6050_ACCEL_XOUT_H, data, 14) < 0) {
        return -1;
    }

    raw->ax = (int16_t)((data[0]  << 8) | data[1]);
    raw->ay = (int16_t)((data[2]  << 8) | data[3]);
    raw->az = (int16_t)((data[4]  << 8) | data[5]);

    raw->gx = (int16_t)((data[8]  << 8) | data[9]);
    raw->gy = (int16_t)((data[10] << 8) | data[11]);
    raw->gz = (int16_t)((data[12] << 8) | data[13]);

    return 0;
}

void Sensor_PrintLastI2CError(void)
{
    Serial_WriteString("I2C ERROR op=");
    Serial_WriteString(g_i2c2_last_error.op);
    Serial_WriteString(" step=");
    Serial_WriteString(g_i2c2_last_error.step);
    Serial_WriteString(" dev=0x");
    Serial_WriteHex8(g_i2c2_last_error.dev_addr);
    Serial_WriteString(" reg=0x");
    Serial_WriteHex8(g_i2c2_last_error.reg);

    if (g_i2c2_last_error.index != 0xFFU) {
        Serial_WriteString(" idx=");
        Serial_WriteInt(g_i2c2_last_error.index);
    }

    Serial_WriteString(" SCL=");
    Serial_WriteInt(g_i2c2_last_error.scl_level);
    Serial_WriteString(" SDA=");
    Serial_WriteInt(g_i2c2_last_error.sda_level);
    Serial_WriteString(" SR1=0x");
    Serial_WriteHex16((uint16_t)g_i2c2_last_error.sr1);
    Serial_WriteString(" SR2=0x");
    Serial_WriteHex16((uint16_t)g_i2c2_last_error.sr2);
    Serial_WriteString(" SR1_SEEN=0x");
    Serial_WriteHex16((uint16_t)g_i2c2_last_error.sr1_seen);

    Serial_WriteString(" flags=");
    if ((g_i2c2_last_error.sr1 & I2C_SR1_AF) != 0U) {
        I2C2_PrintFlag("AF");
    }
    if ((g_i2c2_last_error.sr1 & I2C_SR1_BERR) != 0U) {
        I2C2_PrintFlag("BERR");
    }
    if ((g_i2c2_last_error.sr1 & I2C_SR1_ARLO) != 0U) {
        I2C2_PrintFlag("ARLO");
    }
    if ((g_i2c2_last_error.sr2 & I2C_SR2_BUSY) != 0U) {
        I2C2_PrintFlag("BUSY");
    }
    if ((g_i2c2_last_error.sr1_seen & I2C_SR1_AF) != 0U &&
        (g_i2c2_last_error.sr1 & I2C_SR1_AF) == 0U) {
        I2C2_PrintFlag("AF_SEEN");
    }
    if ((g_i2c2_last_error.sr1_seen & I2C_SR1_BERR) != 0U &&
        (g_i2c2_last_error.sr1 & I2C_SR1_BERR) == 0U) {
        I2C2_PrintFlag("BERR_SEEN");
    }
    if ((g_i2c2_last_error.sr1_seen & I2C_SR1_ARLO) != 0U &&
        (g_i2c2_last_error.sr1 & I2C_SR1_ARLO) == 0U) {
        I2C2_PrintFlag("ARLO_SEEN");
    }

    I2C2_PrintDiagnosis();
    Serial_WriteString("\r\n");
}

/* ---------------- Filter / PID ---------------- */

float Sensor_GetPitchAccDeg(int16_t ax, int16_t az)
{
    return atan2f((float)(-ax), (float)(az)) * RAD_TO_DEG;
}

float Sensor_GetGyroYDegPerSec(int16_t gy_raw)
{
    return ((float)gy_raw) / GYRO_SENS_250DPS;
}

float Sensor_ComplementaryFilter(float pitch_prev, float gyro_rate, float pitch_acc, float dt)
{
    return 0.98f * (pitch_prev + gyro_rate * dt)
         + 0.02f * pitch_acc;
}

void Sensor_PID_Reset(void)
{
    integral = 0.0f;
}

float Sensor_PID_Control(float target_angle, float pitch, float pitch_rate, float dt)
{
  
    float error;
    float control;

    error = -target_angle-pitch;
    integral += error * dt;

    control = KP * error
            + KI * integral
            - KD * pitch_rate;

    if (control > PWM_MAX)  control = PWM_MAX;
    if (control < -PWM_MAX) control = -PWM_MAX;

    return control;
}

void Print_SensorLog(float pitch_acc, float pitch, float control){
    static uint32_t print_div = 0U;

    if (++print_div >= 20U) {
            print_div = 0U;

            Serial_WriteString("P_ACC:");
            Serial_WriteFloat2(pitch_acc);

            Serial_WriteString(" P_FUSED:");
            Serial_WriteFloat2(pitch);

            Serial_WriteString(" CTRL:");
            Serial_WriteFloat2(control);

            Serial_WriteString("\r\n");
        }
}
