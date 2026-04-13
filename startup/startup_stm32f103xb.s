.syntax unified
.cpu cortex-m3
.fpu softvfp
.thumb

.extern SystemInit
.extern main

.global _estack
.global Reset_Handler

.section .isr_vector,"a",%progbits
.type _isr_vector, %object
_isr_vector:
    .word _estack
    .word Reset_Handler
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    .word WWDG_IRQHandler
    .word PVD_IRQHandler
    .word TAMPER_IRQHandler
    .word RTC_IRQHandler
    .word FLASH_IRQHandler
    .word RCC_IRQHandler
    .word EXTI0_IRQHandler
    .word EXTI1_IRQHandler
    .word EXTI2_IRQHandler
    .word EXTI3_IRQHandler
    .word EXTI4_IRQHandler
    .word DMA1_Channel1_IRQHandler
    .word DMA1_Channel2_IRQHandler
    .word DMA1_Channel3_IRQHandler
    .word DMA1_Channel4_IRQHandler
    .word DMA1_Channel5_IRQHandler
    .word DMA1_Channel6_IRQHandler
    .word DMA1_Channel7_IRQHandler
    .word ADC1_2_IRQHandler
    .word USB_HP_CAN_TX_IRQHandler
    .word USB_LP_CAN_RX0_IRQHandler
    .word CAN_RX1_IRQHandler
    .word CAN_SCE_IRQHandler
    .word EXTI9_5_IRQHandler
    .word TIM1_BRK_IRQHandler
    .word TIM1_UP_IRQHandler
    .word TIM1_TRG_COM_IRQHandler
    .word TIM1_CC_IRQHandler
    .word TIM2_IRQHandler
    .word TIM3_IRQHandler
    .word TIM4_IRQHandler
    .word I2C1_EV_IRQHandler
    .word I2C1_ER_IRQHandler
    .word I2C2_EV_IRQHandler
    .word I2C2_ER_IRQHandler
    .word SPI1_IRQHandler
    .word SPI2_IRQHandler
    .word USART1_IRQHandler
    .word USART2_IRQHandler
    .word USART3_IRQHandler
    .word EXTI15_10_IRQHandler
    .word RTCAlarm_IRQHandler
    .word USBWakeUp_IRQHandler

.size _isr_vector, . - _isr_vector

.section .text.Reset_Handler,"ax",%progbits
.type Reset_Handler, %function
Reset_Handler:
    ldr r0, =_estack
    mov sp, r0

    ldr r0, =_sidata
    ldr r1, =_sdata
    ldr r2, =_edata
1:
    cmp r1, r2
    ittt lt
    ldrlt r3, [r0], #4
    strlt r3, [r1], #4
    blt 1b

    ldr r0, =_sbss
    ldr r1, =_ebss
2:
    cmp r0, r1
    itt lt
    movlt r2, #0
    strlt r2, [r0], #4
    blt 2b

    bl SystemInit
    bl main

LoopForever:
    b LoopForever

.size Reset_Handler, . - Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.type Default_Handler, %function
Default_Handler:
    b LoopForever

.size Default_Handler, . - Default_Handler

.macro WEAK_IRQ_HANDLER handler
    .weak \handler
    .type \handler, %function
\handler:
    b Default_Handler
.endm

WEAK_IRQ_HANDLER NMI_Handler
WEAK_IRQ_HANDLER HardFault_Handler
WEAK_IRQ_HANDLER MemManage_Handler
WEAK_IRQ_HANDLER BusFault_Handler
WEAK_IRQ_HANDLER UsageFault_Handler
WEAK_IRQ_HANDLER SVC_Handler
WEAK_IRQ_HANDLER DebugMon_Handler
WEAK_IRQ_HANDLER PendSV_Handler
WEAK_IRQ_HANDLER SysTick_Handler

WEAK_IRQ_HANDLER WWDG_IRQHandler
WEAK_IRQ_HANDLER PVD_IRQHandler
WEAK_IRQ_HANDLER TAMPER_IRQHandler
WEAK_IRQ_HANDLER RTC_IRQHandler
WEAK_IRQ_HANDLER FLASH_IRQHandler
WEAK_IRQ_HANDLER RCC_IRQHandler
WEAK_IRQ_HANDLER EXTI0_IRQHandler
WEAK_IRQ_HANDLER EXTI1_IRQHandler
WEAK_IRQ_HANDLER EXTI2_IRQHandler
WEAK_IRQ_HANDLER EXTI3_IRQHandler
WEAK_IRQ_HANDLER EXTI4_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel1_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel2_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel3_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel4_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel5_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel6_IRQHandler
WEAK_IRQ_HANDLER DMA1_Channel7_IRQHandler
WEAK_IRQ_HANDLER ADC1_2_IRQHandler
WEAK_IRQ_HANDLER USB_HP_CAN_TX_IRQHandler
WEAK_IRQ_HANDLER USB_LP_CAN_RX0_IRQHandler
WEAK_IRQ_HANDLER CAN_RX1_IRQHandler
WEAK_IRQ_HANDLER CAN_SCE_IRQHandler
WEAK_IRQ_HANDLER EXTI9_5_IRQHandler
WEAK_IRQ_HANDLER TIM1_BRK_IRQHandler
WEAK_IRQ_HANDLER TIM1_UP_IRQHandler
WEAK_IRQ_HANDLER TIM1_TRG_COM_IRQHandler
WEAK_IRQ_HANDLER TIM1_CC_IRQHandler
WEAK_IRQ_HANDLER TIM2_IRQHandler
WEAK_IRQ_HANDLER TIM3_IRQHandler
WEAK_IRQ_HANDLER TIM4_IRQHandler
WEAK_IRQ_HANDLER I2C1_EV_IRQHandler
WEAK_IRQ_HANDLER I2C1_ER_IRQHandler
WEAK_IRQ_HANDLER I2C2_EV_IRQHandler
WEAK_IRQ_HANDLER I2C2_ER_IRQHandler
WEAK_IRQ_HANDLER SPI1_IRQHandler
WEAK_IRQ_HANDLER SPI2_IRQHandler
WEAK_IRQ_HANDLER USART1_IRQHandler
WEAK_IRQ_HANDLER USART2_IRQHandler
WEAK_IRQ_HANDLER USART3_IRQHandler
WEAK_IRQ_HANDLER EXTI15_10_IRQHandler
WEAK_IRQ_HANDLER RTCAlarm_IRQHandler
WEAK_IRQ_HANDLER USBWakeUp_IRQHandler

.end
