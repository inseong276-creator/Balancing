#include "stm32f103xb.h"
#include "serial.h"
#include <stdint.h>

void Serial_UASRT3_Init(void){
    /*
        Partial Remap
        PC10 : USART3 TX
        PC11 : USART3 RX
    */
    AFIO->MAPR &= (0x3 << 4);
    AFIO->MAPR |= (0x1 << 4);

    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART3EN;

    /*PC10 = USART3_TX : AF PP 50MHz*/
    GPIOC->CRH &= (0xFUL << 8);
    GPIOC->CRH |= (0xBUL << 8);

    /*PC11 = USART3_RX : Input floating*/
    GPIOC->CRH &= (0xFUL << 12);
    GPIOC->CRH |= (0x4UL << 12);

    USART3->BRR = (234 << 4) | 6;
    USART3->CR1 |= USART_CR1_UE | USART_CR1_TE
                |  USART_CR1_RE;
}

void Serial_USART2_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* PA2 = USART2_TX : AF PP 50MHz */
    GPIOA->CRL &= ~(0xFUL << 8);
    GPIOA->CRL |=  (0xBUL << 8);

    /* PA3 = USART2_RX : Input floating */
    GPIOA->CRL &= ~(0xFUL << 12);
    GPIOA->CRL |=  (0x4UL << 12);

    /* APB1 = 36MHz, 115200 baud -> BRR = 0x0138 */
    USART2->BRR = 0x0138;

    USART2->CR1 = USART_CR1_TE | USART_CR1_RE;
    USART2->CR1 |= USART_CR1_UE;
}

void Serial_WriteChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = (uint8_t)c;
}

void Serial_WriteString(const char *s)
{
    while (*s) {
        Serial_WriteChar(*s++);
    }
}

void Serial_WriteInt(int32_t value)
{
    char buf[16];
    int i = 0;
    int j;
    int32_t temp = value;

    if (temp == 0) {
        Serial_WriteChar('0');
        return;
    }

    if (temp < 0) {
        Serial_WriteChar('-');
        temp = -temp;
    }

    while (temp > 0) {
        buf[i++] = (char)('0' + (temp % 10));
        temp /= 10;
    }

    for (j = i - 1; j >= 0; j--) {
        Serial_WriteChar(buf[j]);
    }
}

void Serial_WriteFloat2(float value)
{
    int32_t scaled;
    int32_t int_part;
    int32_t frac_part;

    if (value < 0.0f) {
        Serial_WriteChar('-');
        value = -value;
    }

    scaled = (int32_t)(value * 100.0f + 0.5f);
    int_part = scaled / 100;
    frac_part = scaled % 100;

    Serial_WriteInt(int_part);
    Serial_WriteChar('.');
    Serial_WriteChar((char)('0' + (frac_part / 10)));
    Serial_WriteChar((char)('0' + (frac_part % 10)));
}