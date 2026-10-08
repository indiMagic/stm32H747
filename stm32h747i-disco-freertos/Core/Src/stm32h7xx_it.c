#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

extern TIM_HandleTypeDef htim6;
extern void xPortSysTickHandler(void);

void NMI_Handler(void)        { while (1) { } }
void HardFault_Handler(void)  { while (1) { __asm volatile ("bkpt #0"); } }
void MemManage_Handler(void)  { while (1) { __asm volatile ("bkpt #0"); } }
void BusFault_Handler(void)   { while (1) { __asm volatile ("bkpt #0"); } }
void UsageFault_Handler(void) { while (1) { __asm volatile ("bkpt #0"); } }
void DebugMon_Handler(void)   { }

/* SVC_Handler and PendSV_Handler are provided by the FreeRTOS port
 * (mapped in FreeRTOSConfig.h). */

/* FreeRTOS owns SysTick; the HAL tick runs on TIM6. */
void SysTick_Handler(void)
{
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
        xPortSysTickHandler();
    }
}

void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim6);
}
