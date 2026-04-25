#include <stdint.h>

#define AIN1_PIN (1UL << 5)     /*PB5*/
#define AIN2_PIN (1UL << 8)     /*PB8*/
#define BIN1_PIN (1UL << 9)     /*PA9*/
#define BIN2_PIN (1UL << 10)    /*PA10*/
#define STBY_PIN (1UL << 8)     /*PA8*/

//#define DT_SEC 0.01f


/*
    Right_Encoder_A = TIM3_CH1
    Right_Encoder_B = TIM3_CH2
    Left_Encoder_A  = TIM4_CH1
    Left_Encoder_B  = TIM4_CH2
*/

void Motor_GPIO_Init(void);
void Motor_SetStandby(uint8_t enable);
void Motor_Forward(void);
void Motor_Reverse(void);
void Motor_Break(void);
void Motor_SetSigned(int16_t pwm);
void Motor_SetDifferentialSigned(int16_t left_pwm, int16_t right_pwm);
void Motor_PWM_Init(void);
void Encoder_Init(void);

float Left_GetSpeed(float dt);
float Right_GetSpeed(float dt);
float Motor_PID_Control(float target_speed, float speed);
void Motor_UpdateTargetSpeed(float dt);
void PrintMotorLog(float rs, float ls, float s, float dt, float ta);
void UART_CMD_Process(void);
