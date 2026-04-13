#include "sensor.h"
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

static float integral = 0.0f;

static void delay_loop(volatile uint32_t count)
{
    while (count--) {
        __asm volatile("nop");
    }
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

void Sensor_I2C2_Init(void)
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

/*
static void I2C2_Recover(void)
{
    I2C2->CR1 |= I2C_CR1_SWRST;
    delay_loop(5000);
    I2C2->CR1 &= ~I2C_CR1_SWRST;

    I2C2->CR2 = 36;
    I2C2->CCR = 180;
    I2C2->TRISE = 37;
    I2C2->CR1 |= I2C_CR1_ACK;
    I2C2->CR1 |= I2C_CR1_PE;
}
*/

static void I2C2_Recover(void)
{
    int i;

    /* 1) I2C peripheral disable */
    I2C2->CR1 &= ~I2C_CR1_PE;

    /* 2) PB10=SCL, PB11=SDA 를 GPIO open-drain output으로 변경
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

    /* 6) 혹시 남은 내부 상태가 있으면 SWRST도 한 번 수행 */
    I2C2->CR1 |= I2C_CR1_SWRST;
    delay_loop(2000);
    I2C2->CR1 &= ~I2C_CR1_SWRST;

    /* 7) 핀을 다시 I2C AF Open-Drain 50MHz로 복구
       MODE=11, CNF=11 => 0xF
    */
    GPIOB->CRH &= ~(0xFUL << 8);
    GPIOB->CRH |=  (0xFUL << 8);

    GPIOB->CRH &= ~(0xFUL << 12);
    GPIOB->CRH |=  (0xFUL << 12);

    delay_loop(2000);

    /* 8) I2C 재초기화 */
    I2C2->CR1 = 0x0000;
    I2C2->CR2 = 36;
    I2C2->CCR = 180;
    I2C2->TRISE = 37;
    I2C2->CR1 |= I2C_CR1_ACK;
    I2C2->CR1 |= I2C_CR1_PE;

    /* 필요하면 STOP 비트 한번 더 정리 */
    I2C2->CR1 |= I2C_CR1_STOP;
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

    I2C2->DR = addr;

    if (wait_set(&I2C2->SR1, I2C_SR1_ADDR) < 0) {
        return -1;
    }

    temp = I2C2->SR1;
    temp = I2C2->SR2;
    (void)temp;

    return 0;
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
        I2C2_Recover();
        return -1;
    }

    if (I2C2_Start() < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_SendAddress((uint8_t)((dev_addr << 1) | 0U)) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_WriteByte(reg) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_WriteByte(data) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (wait_set(&I2C2->SR1, I2C_SR1_BTF) < 0) {
        I2C2_Recover();
        return -1;
    }

    I2C2_Stop();
    return 0;
}

static int I2C2_ReadRegs(uint8_t dev_addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    volatile uint32_t temp;
    uint8_t i;

    if (wait_clear(&I2C2->SR2, I2C_SR2_BUSY) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_Start() < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_SendAddress((uint8_t)((dev_addr << 1) | 0U)) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_WriteByte(reg) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (wait_set(&I2C2->SR1, I2C_SR1_BTF) < 0) {
        I2C2_Recover();
        return -1;
    }

    if (I2C2_Start() < 0) {
        I2C2_Recover();
        return -1;
    }

    I2C2->DR = (uint8_t)((dev_addr << 1) | 1U);

    if (wait_set(&I2C2->SR1, I2C_SR1_ADDR) < 0) {
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
                    I2C2_Recover();
                    return -1;
                }
            } else {
                if (I2C2_ReadByte_Ack(&buf[i]) < 0) {
                    I2C2_Recover();
                    return -1;
                }
            }
        }
    }

    I2C2->CR1 |= I2C_CR1_ACK;
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
    static int print_div = 0;

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