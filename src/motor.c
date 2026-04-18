#include "stm32f103xb.h"
#include "motor.h"
#include "serial.h"

extern float target_speed;
extern float turn_cmd;

void Motor_GPIO_Init(void)
{
    /* GPIOA, GPIOB, GPIOC, AFIO clock enable */
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN |
                    RCC_APB2ENR_IOPBEN |
                    RCC_APB2ENR_IOPCEN |
                    RCC_APB2ENR_AFIOEN;

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN |
                    RCC_APB1ENR_TIM3EN |
                    RCC_APB1ENR_TIM4EN;

    /* PA0 PA1 -> PWM */
    GPIOA->CRL &= ~((0xFUL << 0) | (0xFUL << 4));
    GPIOA->CRL |=  ((0xBUL << 0) | (0xBUL << 4));

    /* PB5 PB8 : AIN1,2 -> output */
    GPIOB->CRL &= ~(0xFUL << 20);
    GPIOB->CRL |=  (0x3UL << 20);
    GPIOB->CRH &= ~(0xFUL << 0);
    GPIOB->CRH |= (0x3UL << 0);

    /* PA9 PA10 : BIN1,2 -> output */
    GPIOA->CRH &= ~((0xFUL << 4) | (0xFUL << 8));
    GPIOA->CRH |=  ((0x3UL << 4) | (0x3UL << 8));

    /* PA8 -> STBY */
    GPIOA->CRH &= ~(0xFUL << 0);
    GPIOA->CRH |=  (0x3UL << 0);

    /* 기본값 LOW */
    GPIOB->BRR = AIN1_PIN | AIN2_PIN;
    GPIOA->BRR = BIN1_PIN | BIN2_PIN;
    GPIOA->BRR = STBY_PIN;

     /*
     * PA6, PA7 : Right Encoder A,B(TIM3 ch1,2) -> input floating
     * CNF=01, MODE=00 => 0b0100 = 0x4
     */
    GPIOA->CRL &= ~((0xFUL << 24) | (0xFUL << 28));
    GPIOA->CRL |=  ((0x4UL << 24) | (0x4UL << 28));

    /*
     * PB6, PB7 : Left Encoder A,B(TIM4 ch1,2) -> input floating
     */
    GPIOB->CRL &= ~((0xFUL << 24) | (0xFUL << 28));
    GPIOB->CRL |=  ((0x4UL << 24) | (0x4UL << 28));
}

static void Encoder_TIMx_Init(TIMx_TypeDef *TIMx)
{
    /*
     * Encoder mode 3:
     * count on both TI1 and TI2 edges
     * SMS = 011
     */
    TIMx->SMCR &= ~(0x7UL << 0);
    TIMx->SMCR |=  (0x3UL << 0);

    /*
     * CC1S = 01 -> CC1 mapped on TI1
     * CC2S = 01 -> CC2 mapped on TI2
     */
    TIMx->CCMR1 &= ~((0x3UL << 0) | (0x3UL << 8));
    TIMx->CCMR1 |=  ((0x1UL << 0) | (0x1UL << 8));

    /*
     * Optional input filter
     * IC1F = 0011
     * IC2F = 0011
     * 노이즈가 많으면 필터를 조금 넣기
    */
    TIMx->CCMR1 &= ~((0xFUL << 4) | (0xFUL << 12));
    TIMx->CCMR1 |=  ((0x3UL << 4) | (0x3UL << 12));

    /*
     * Rising edge polarity
     * CC1P = 0, CC2P = 0
    */
    TIMx->CCER &= ~((1UL << 1) | (1UL << 5));
    TIMx->CCER |= ((1UL << 0) | (1UL << 4));

    /*
     * Prescaler = 0
     * Auto reload max
    */
    TIMx->PSC = 0;
    TIMx->ARR = 0xFFFF;
    TIMx->CNT = 0;

    /* Update event */
    TIMx->EGR = 1;

    /* Counter enable */
    TIMx->CR1 |= TIM_CR1_CEN;
}

void Encoder_Init(void){
    Encoder_TIMx_Init(TIM3);
    Encoder_TIMx_Init(TIM4);
}

void Motor_PWM_Init(void)
{
    /*
      TIM2 clock:
      APB1 = 36MHz
      Timer clock = 72MHz (APB1 prescaler != 1)

      PWM frequency example:
      PSC = 71   -> 72MHz / 72 = 1MHz
      ARR = 999  -> 1MHz / 1000 = 1kHz
    */

    TIM2->PSC = 71;
    TIM2->ARR = 999;
    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;

    /*
      CH1: PWM mode 1
      OC1M = 110, OC1PE = 1
    */
    TIM2->CCMR1 &= ~((0x7UL << 4) | (1UL << 3));
    TIM2->CCMR1 |=  ((0x6UL << 4) | (1UL << 3));

    /*
      CH2: PWM mode 1
      OC2M = 110, OC2PE = 1
    */
    TIM2->CCMR1 &= ~((0x7UL << 12) | (1UL << 11));
    TIM2->CCMR1 |=  ((0x6UL << 12) | (1UL << 11));

    /* Enable CH1, CH2 output */
    TIM2->CCER |= TIM_CCER_CC1E;
    TIM2->CCER |= (1UL << 4);   /* CC2E */

    /* Auto-reload preload enable */
    TIM2->CR1 |= TIM_CR1_ARPE;

    /* Generate update event */
    TIM2->EGR = 1;

    /* Counter enable */
    TIM2->CR1 |= TIM_CR1_CEN;
}


void Motor_SetStandby(uint8_t enable)
{
    if (enable)
        GPIOA->BSRR = STBY_PIN;
    else
        GPIOA->BRR = STBY_PIN;
}

void Motor_Forward(void)
{
    GPIOB->BSRR = AIN1_PIN;
    GPIOB->BRR  = AIN2_PIN;

    GPIOA->BSRR = BIN1_PIN;
    GPIOA->BRR  = BIN2_PIN;
}

void Motor_Reverse(void)
{
    GPIOB->BSRR = AIN2_PIN;
    GPIOB->BRR  = AIN1_PIN;

    GPIOA->BSRR = BIN2_PIN;
    GPIOA->BRR  = BIN1_PIN;   
}

void Motor_Brake(void)
{
    GPIOB->BSRR = AIN1_PIN | AIN2_PIN;
    GPIOA->BSRR = BIN1_PIN | BIN2_PIN;

    TIM2->CCR1 = 0;
    TIM2->CCR2 = 0;
}

void Motor_SetSigned(int16_t pwm)
{
    int16_t mag;

    if (pwm > 1000) pwm = 1000;
    if (pwm < -1000) pwm = -1000;

    if (pwm == 0) {
        Motor_Brake();
        return;
    }

    mag = (pwm > 0) ? pwm : -pwm;

    if (pwm > 0)
        Motor_Forward();
    else
        Motor_Reverse();

    TIM2->CCR1 = 1.15 * mag;
    TIM2->CCR2 = 1.0 * mag;
}


float Right_GetSpeed(float dt)
{
    static uint16_t v_prev = 0;
    static int initialized = 0;

    uint16_t v_current = (uint16_t)TIM3->CNT;

    if (!initialized) {
        v_prev = v_current;
        initialized = 1;
        return 0.0f;
    }

    int16_t diff = (int16_t)(v_current - v_prev); // int16_t로 캐스팅 -> ARR overflow에서 
    v_prev = v_current;

    if (dt < 0.0005f) dt = 0.0005f;

    float speed = diff / dt;  // counts/sec

    return speed;
}

float Left_GetSpeed(float dt){
    static uint16_t v_prev = 0;
    static int initialized = 0;

    uint16_t v_current = (uint16_t)TIM4->CNT;

    if(!initialized){
        v_prev = v_current;
        initialized = 1;
        return 0.0f;
    }

    int16_t diff = (int16_t)(v_current - v_prev);
    v_prev = v_current;

    if(dt < 0.0005f) dt = 0.0005f;
    
    float speed = diff / dt;

    return speed;
}

float Motor_PID_Control(float target_speed, float speed){
    float Kv = 0.013f;
    float error = target_speed -speed;

    float target_angle = Kv * error;

    if(target_angle > 10) target_angle = 10;
    else if (target_angle < -10) target_angle = -10;

    return target_angle;
}


void PrintMotorLog(float rs, float ls, float s, float dt, float ta){
    static uint32_t print_div = 0;

    if(++print_div >= 20U){
        print_div = 0U;

        Serial_WriteString("Right speed : ");
        Serial_WriteFloat2(rs);
        Serial_WriteString("  Left speed : ");
        Serial_WriteFloat2(ls);
        Serial_WriteString("  speed : ");
        Serial_WriteFloat2(s);
        Serial_WriteString("  dt : ");
        Serial_WriteFloat2(dt);
        Serial_WriteString(" Target Angle : ");
        Serial_WriteFloat2(ta);
        Serial_WriteString("\r\n");

       }
}

void UART_CMD_Process(void){
    if (USART3->SR & USART_SR_RXNE) {

        char cmd = (char)USART3->DR;

        switch(cmd){
            case 'w':
                target_speed = 120.0f;
                turn_cmd = 0.0f;
                break;

            case 's':
                target_speed = -120.0f;
                turn_cmd = 0.0f;
                break;

            case 'a':
                turn_cmd = -30.0f;
                target_speed = 0.0f;
                break;

            case 'd':
                turn_cmd = 30.0f;
                target_speed = 0.0f;
                break;

            case 'q':
                target_speed = 0.0f;
                turn_cmd = 0.0f;
                break;

            default:
                break;
        }

        // ===== 출력 =====
        Serial_WriteString("CMD : ");
        Serial_WriteChar(cmd);
        Serial_WriteString("  Target Speed : ");
        Serial_WriteFloat2(target_speed);
        Serial_WriteString("\r\n");
    }
}