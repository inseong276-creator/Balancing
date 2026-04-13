#include "stm32f103xb.h"
#include "motor.h"
#include "sensor.h"
#include "serial.h"
#include <stdint.h>

#define isPrintSensorLog 0
#define isPrintMotorLog 1

static void delay_loop(volatile uint32_t count)
{
    while (count--) {
        __asm volatile("nop");
    }
}

static void delay_ms(uint32_t ms)
{
    while (ms--) {
        delay_loop(8000);
    }
}

float Get_DT(){
    static uint16_t last_cnt = 0;

    uint16_t now = TIM1->CNT;
    uint16_t dt = now - last_cnt;
    last_cnt = now;

    return dt * 1e-6f;
}

void TIM1_INIT(void){
    RCC->APB2ENR |= (1 << 11); // TIM1EN

    TIM1->PSC = 72 - 1;
    TIM1->ARR = 0xFFFF;
    TIM1->CNT = 0;
    TIM1->CR1 |= (1U << 0); // TIM1 Enable

    Get_DT();
}

int main(void)
{
    MPU6050_RawData raw;
    float pitch_acc;
    float pitch_gyro_rate;
    float pitch = 0.0f;
    float control;
    uint32_t print_div = 0;

    SystemInit();
    TIM1_INIT();

    Serial_USART2_Init();
    Sensor_I2C2_Init();

    Motor_GPIO_Init();
    Motor_PWM_Init();
    Motor_SetStandby(1);
    
    Encoder_Init();

    Serial_WriteString("MPU6050 I2C2 + USART2 pitch test start\r\n");

    delay_ms(100);

    if (Sensor_MPU6050_Init() < 0) {
        Serial_WriteString("MPU6050 init fail\r\n");
        while (1) {
            Motor_SetSigned(0);
        }
    }

    Serial_WriteString("MPU6050 init done\r\n");

    if (Sensor_MPU6050_ReadRaw(&raw) < 0) {
        Serial_WriteString("MPU6050 first read fail\r\n");
        while (1) {
            Motor_SetSigned(0);
        }
    }

    pitch = Sensor_GetPitchAccDeg(raw.ax, raw.az);
    Sensor_FilterReset(pitch);
    Sensor_PID_Reset();

    while (1) {
        float dt = Get_DT();
        if (Sensor_MPU6050_ReadRaw(&raw) < 0) {
            Motor_SetSigned(0);
            Serial_WriteString("I2C ERROR\r\n");
            delay_ms(20);
            dt = Get_DT();
            continue;
        }

        float left_speed = Left_GetSpeed(dt);
        float right_speed = Right_GetSpeed(dt);
        float speed = (left_speed + right_speed) / 2;

        float target_angle = Motor_PID_Control(speed);

        pitch_acc = Sensor_GetPitchAccDeg(raw.ax, raw.az);
        pitch_gyro_rate = Sensor_GetGyroYDegPerSec(raw.gy);

        pitch = Sensor_ComplementaryFilter(pitch, pitch_gyro_rate, pitch_acc, dt);
        control = Sensor_PID_Control(target_angle, pitch, pitch_gyro_rate, dt);

        Motor_SetSigned((int16_t)control);

        if(isPrintSensorLog)    Print_SensorLog(pitch_acc, pitch, control);
        if(isPrintMotorLog)     Print_MotorLog(right_speed, left_speed, speed, dt, target_angle);

        delay_ms(10);
    }
}