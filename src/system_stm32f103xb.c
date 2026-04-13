#include "stm32f103xb.h"

uint32_t SystemCoreClock = 8000000UL;

static void SetSysClock(void);

void SystemInit(void)
{
    RCC->CR |= RCC_CR_HSION;

    RCC->CFGR = 0x00000000UL;
    RCC->CR &= ~(RCC_CR_PLLON);

    SetSysClock();
}

void SystemCoreClockUpdate(void)
{
    uint32_t sysclk_source = (RCC->CFGR >> 2) & 0x3UL;

    switch (sysclk_source) {
    case 0x0: /* HSI */
        SystemCoreClock = 8000000UL;
        break;
    case 0x1: /* HSE */
        SystemCoreClock = 8000000UL;
        break;
    case 0x2: /* PLL */ {
        uint32_t pllmul = ((RCC->CFGR >> 18) & 0xFUL) + 2UL;
        uint32_t pll_src = (RCC->CFGR & RCC_CFGR_PLLSRC) ? 8000000UL : 4000000UL;
        if ((RCC->CFGR & RCC_CFGR_PLLXTPRE) && pll_src == 8000000UL) {
            pll_src /= 2UL;
        }
        SystemCoreClock = pll_src * pllmul;
        break;
    }
    default:
        SystemCoreClock = 8000000UL;
        break;
    }
}

static void SetSysClock(void)
{
    RCC->CR |= RCC_CR_HSEON;
    while ((RCC->CR & RCC_CR_HSERDY) == 0U) {
    }

    FLASH->ACR |= FLASH_ACR_PRFTBE;
    FLASH->ACR &= ~0x7UL;
    FLASH->ACR |= FLASH_ACR_LATENCY_2;

    RCC->CFGR &= ~((0xFUL << 4) | (0x7UL << 8) | (0x7UL << 11) | (0xFUL << 18));
    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;
    RCC->CFGR |= RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9;

    RCC->CR |= RCC_CR_PLLON;
    while ((RCC->CR & RCC_CR_PLLRDY) == 0U) {
    }

    RCC->CFGR &= ~0x3UL;
    RCC->CFGR |= RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & 0xCU) != RCC_CFGR_SWS_PLL) {
    }

#if defined(__FPU_PRESENT) && __FPU_PRESENT
    __asm volatile("DSB");
    __asm volatile("ISB");
#endif

    SystemCoreClock = 72000000UL;
}
