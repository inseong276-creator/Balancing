#ifndef STM32F103XB_H
#define STM32F103XB_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


#define PERIPH_BASE           (0x40000000UL)
#define APB1PERIPH_BASE       (PERIPH_BASE)
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000UL)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x00020000UL)

#define RCC_BASE              (AHBPERIPH_BASE + 0x00001000UL)
#define FLASH_R_BASE          (AHBPERIPH_BASE + 0x00002000UL)
#define GPIOA_BASE            (APB2PERIPH_BASE + 0x00000800UL)
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x00000C00UL)
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x00001000UL)
#define EXTI_BASE             (APB2PERIPH_BASE + 0x00000400UL)
#define TIM1_BASE             (APB2PERIPH_BASE + 0x00002C00UL)
#define TIM2_BASE             (APB1PERIPH_BASE + 0x00000000UL)
#define TIM3_BASE             (APB1PERIPH_BASE + 0x00000400UL)
#define TIM4_BASE             (APB1PERIPH_BASE + 0x00000800UL)
#define AFIO_BASE             (APB2PERIPH_BASE + 0x00000000UL)
#define SCB_AIRCR             (*(volatile uint32_t *)0xE000ED0CUL)
#define USART1_BASE           (APB2PERIPH_BASE + 0x00003800UL)
#define USART2_BASE           (APB1PERIPH_BASE + 0x00004400UL)
#define USART3_BASE           (APB1PERIPH_BASE + 0x00004800UL)
#define I2C1_BASE             (APB1PERIPH_BASE + 0x00005400UL)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x00005800UL)


#define RCC   ((RCC_TypeDef *) RCC_BASE)
#define FLASH ((FLASH_TypeDef *) FLASH_R_BASE)
#define GPIOC ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOA ((GPIO_TypeDef *) GPIOA_BASE)
#define GPIOB ((GPIO_TypeDef *) GPIOB_BASE)
#define EXTI ((EXTI_TypeDef *) EXTI_BASE)
#define AFIO ((AFIO_TypeDef *) AFIO_BASE)
#define TIM1 ((TIM1_TypeDef *) TIM1_BASE)
#define TIM2 ((TIMx_TypeDef *) TIM2_BASE)
#define TIM3 ((TIMx_TypeDef *) TIM3_BASE)
#define TIM4 ((TIMx_TypeDef *) TIM4_BASE)
#define USART1 ((USART_TypeDef *) USART1_BASE)
#define USART2 ((USART_TypeDef *) USART2_BASE)
#define USART3 ((USART_TypeDef *) USART3_BASE)

#define I2C1   ((I2C_TypeDef *) I2C1_BASE)
#define I2C2   ((I2C_TypeDef *) I2C2_BASE)


typedef struct {
    volatile uint32_t CR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t APB2RSTR;
    volatile uint32_t APB1RSTR;
    volatile uint32_t AHBENR;
    volatile uint32_t APB2ENR;
    volatile uint32_t APB1ENR;
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    volatile uint32_t AHBSTR;
    volatile uint32_t CFGR2;
} RCC_TypeDef;

typedef struct {
    volatile uint32_t ACR;
    volatile uint32_t KEYR;
    volatile uint32_t OPTKEYR;
    volatile uint32_t SR;
    volatile uint32_t CR;
    volatile uint32_t AR;
    volatile uint32_t RESERVED;
    volatile uint32_t OBR;
    volatile uint32_t WRPR;
} FLASH_TypeDef;

typedef struct {
    volatile uint32_t CRL;
    volatile uint32_t CRH;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t BRR;
    volatile uint32_t LCKR;
} GPIO_TypeDef;

typedef struct
{
  volatile uint32_t IMR;
  volatile uint32_t EMR;
  volatile uint32_t RTSR;
  volatile uint32_t FTSR;
  volatile uint32_t SWIER;
  volatile uint32_t PR;
} EXTI_TypeDef;

typedef struct
{
    volatile uint32_t CR1;      // 0x00
    volatile uint32_t CR2;      // 0x04
    volatile uint32_t SMCR;     // 0x08
    volatile uint32_t DIER;     // 0x0C
    volatile uint32_t SR;       // 0x10
    volatile uint32_t EGR;      // 0x14
    volatile uint32_t CCMR1;    // 0x18
    volatile uint32_t CCMR2;    // 0x1C
    volatile uint32_t CCER;     // 0x20
    volatile uint32_t CNT;      // 0x24
    volatile uint32_t PSC;      // 0x28
    volatile uint32_t ARR;      // 0x2C
    volatile uint32_t RCR;      // 0x30  (TIM1 전용)
    volatile uint32_t CCR1;     // 0x34
    volatile uint32_t CCR2;     // 0x38
    volatile uint32_t CCR3;     // 0x3C
    volatile uint32_t CCR4;     // 0x40
    volatile uint32_t BDTR;     // 0x44  (TIM1 전용)
    volatile uint32_t DCR;      // 0x48
    volatile uint32_t DMAR;     // 0x4C
} TIM1_TypeDef;

typedef struct
{
  volatile uint32_t CR1;      // 0x00
  volatile uint32_t CR2;      // 0x04
  volatile uint32_t SMCR;     // 0x08
  volatile uint32_t DIER;     // 0x0C
  volatile uint32_t SR;       // 0x10
  volatile uint32_t EGR;      // 0x14
  volatile uint32_t CCMR1;    // 0x18
  volatile uint32_t CCMR2;    // 0x1C
  volatile uint32_t CCER;     // 0x20
  volatile uint32_t CNT;      // 0x24
  volatile uint32_t PSC;      // 0x28
  volatile uint32_t ARR;      // 0x2C
  volatile uint32_t RESERVED; // 0x30 
  volatile uint32_t CCR1;     // 0x34
  volatile uint32_t CCR2;     // 0x38
  volatile uint32_t CCR3;     // 0x3C
  volatile uint32_t CCR4;     // 0x40
  volatile uint32_t DCR;      // 0x48
  volatile uint32_t DMAR;     // 0x4C
} TIMx_TypeDef;

typedef struct
{
  volatile uint32_t EVCR;
  volatile uint32_t MAPR;
  volatile uint32_t EXTICR[4];
  uint32_t RESERVED0;
  volatile uint32_t MAPR2;  
} AFIO_TypeDef;

typedef struct
{
  volatile uint32_t SR;    // 0x00
  volatile uint32_t DR;    // 0x04
  volatile uint32_t BRR;   // 0x08
  volatile uint32_t CR1;   // 0x0C
  volatile uint32_t CR2;   // 0x10
  volatile uint32_t CR3;   // 0x14
  volatile uint32_t GTPR;  // 0x18
} USART_TypeDef;

typedef struct
{
  volatile uint32_t CR1;    // 0x00
  volatile uint32_t CR2;    // 0x04
  volatile uint32_t OAR1;   // 0x08
  volatile uint32_t OAR2;   // 0x0C
  volatile uint32_t DR;     // 0x10
  volatile uint32_t SR1;    // 0x14
  volatile uint32_t SR2;    // 0x18
  volatile uint32_t CCR;    // 0x1C
  volatile uint32_t TRISE;  // 0x20
} I2C_TypeDef;


/*RCC Clock Config*/
#define RCC_CR_HSION          (1UL << 0)
#define RCC_CR_HSIRDY         (1UL << 1)
#define RCC_CR_HSEON          (1UL << 16)
#define RCC_CR_HSERDY         (1UL << 17)
#define RCC_CR_PLLON          (1UL << 24)
#define RCC_CR_PLLRDY         (1UL << 25)
#define RCC_CFGR_SW_HSI       (0x0UL)
#define RCC_CFGR_SW_HSE       (0x1UL)
#define RCC_CFGR_SW_PLL       (0x2UL)
#define RCC_CFGR_SWS_PLL      (0x8UL)
#define RCC_CFGR_PLLSRC       (1UL << 16)
#define RCC_CFGR_PLLXTPRE     (1UL << 17)
#define RCC_CFGR_PLLMULL9     (0x7UL << 18)
#define RCC_CFGR_HPRE_DIV1    (0x0UL << 4)
#define RCC_CFGR_PPRE1_DIV2   (0x4UL << 8)
#define RCC_CFGR_PPRE2_DIV1   (0x0UL << 11)

/*RCC GPIO Config*/
#define RCC_APB2ENR_IOPCEN    (1UL << 4)
#define RCC_APB2ENR_IOPAEN    (1UL << 2)
#define RCC_APB2ENR_IOPBEN    (1UL << 3)
#define RCC_APB2ENR_AFIOEN_Pos               (0U)                              
#define RCC_APB2ENR_AFIOEN_Msk               (0x1UL << RCC_APB2ENR_AFIOEN_Pos)  /*!< 0x00000001 */
#define RCC_APB2ENR_AFIOEN                   RCC_APB2ENR_AFIOEN_Msk            /*!< Alternate Function I/O clock enable */
#define GPIO_CRH_MODE13_0     (1UL << 20)
#define GPIO_CRH_MODE13_1     (1UL << 21)
#define GPIO_CRH_CNF13_MASK   (0x3UL << 22)

/*RCC Timer Config*/
#define RCC_APB1ENR_TIM4EN    (1UL << 2)
#define RCC_APB1ENR_TIM3EN    (1UL << 1)
#define RCC_APB1ENR_TIM2EN    (1UL << 0)  
#define TIM_CCER_CC1E         (1UL << 0)
#define TIM_CR1_ARPE          (1UL << 7)
#define TIM_CR1_CEN           (1UL << 0)

/*RCC Flash Config*/
#define FLASH_ACR_PRFTBE      (1UL << 4)
#define FLASH_ACR_LATENCY_0   (0x0UL)
#define FLASH_ACR_LATENCY_1   (0x1UL)
#define FLASH_ACR_LATENCY_2   (0x2UL)

/*RCC USART Config*/
#define RCC_APB1ENR_USART2EN   (1UL << 17)
#define RCC_APB1ENR_USART3EN   (1UL << 18)

/*RCC I2C Config*/
#define RCC_APB1ENR_I2C2EN   (1UL << 22)
#define RCC_APB1RSTR_I2C2RST (1UL << 22)

/* USART SR bits */
#define USART_SR_TXE          (1UL << 7)
#define USART_SR_TC           (1UL << 6)
#define USART_SR_RXNE         (1UL << 5)

/* USART CR1 bits */
#define USART_CR1_UE          (1UL << 13)
#define USART_CR1_M           (1UL << 12)
#define USART_CR1_PCE         (1UL << 10)
#define USART_CR1_TXEIE       (1UL << 7)
#define USART_CR1_TCIE        (1UL << 6)
#define USART_CR1_RXNEIE      (1UL << 5)
#define USART_CR1_TE          (1UL << 3)
#define USART_CR1_RE          (1UL << 2)

/* I2C CR1 bits */
#define I2C_CR1_PE            (1UL << 0)
#define I2C_CR1_START         (1UL << 8)
#define I2C_CR1_STOP          (1UL << 9)
#define I2C_CR1_ACK           (1UL << 10)
#define I2C_CR1_SWRST         (1UL << 15)

/* I2C CR2 bits */
#define I2C_CR2_FREQ_MASK     (0x3FUL)

/* I2C SR1 bits */
#define I2C_SR1_SB            (1UL << 0)
#define I2C_SR1_ADDR          (1UL << 1)
#define I2C_SR1_BTF           (1UL << 2)
#define I2C_SR1_RXNE          (1UL << 6)
#define I2C_SR1_TXE           (1UL << 7)

/* I2C SR2 bits */
#define I2C_SR2_BUSY          (1UL << 1)

/* I2C CCR mode bits */
#define I2C_CCR_FS            (1UL << 15)

/* GPIO configuration helpers */
#define GPIO_MODE_INPUT       (0x0UL)
#define GPIO_MODE_OUTPUT_10M  (0x1UL)
#define GPIO_MODE_OUTPUT_2M   (0x2UL)
#define GPIO_MODE_OUTPUT_50M  (0x3UL)

#define GPIO_CNF_GP_PP        (0x0UL)
#define GPIO_CNF_GP_OD        (0x1UL)
#define GPIO_CNF_AF_PP        (0x2UL)
#define GPIO_CNF_AF_OD        (0x3UL)
#define GPIO_CNF_INPUT_ANALOG (0x0UL)
#define GPIO_CNF_INPUT_FLOAT  (0x1UL)
#define GPIO_CNF_INPUT_PUPD   (0x2UL)

extern uint32_t SystemCoreClock;

void SystemInit(void);
void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* STM32F103XB_H */
