/* HAL time base on TIM6 (1 kHz) so FreeRTOS can keep SysTick for itself. */
#include "main.h"

TIM_HandleTypeDef htim6;

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t flashlatency;
    uint32_t pclk1;
    uint32_t tim_clk;

    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, TickPriority, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
    uwTickPrio = TickPriority;

    __HAL_RCC_TIM6_CLK_ENABLE();

    HAL_RCC_GetClockConfig(&clkconfig, &flashlatency);
    pclk1 = HAL_RCC_GetPCLK1Freq();
    /* Timer clock is 2x PCLK1 when the APB1 prescaler != 1 */
    tim_clk = (clkconfig.APB1CLKDivider == RCC_APB1_DIV1) ? pclk1 : (2U * pclk1);

    htim6.Instance               = TIM6;
    htim6.Init.Prescaler         = (tim_clk / 1000000U) - 1U;   /* 1 MHz counter */
    htim6.Init.Period            = (1000000U / 1000U) - 1U;     /* 1 kHz update  */
    htim6.Init.ClockDivision     = 0;
    htim6.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim6) != HAL_OK) return HAL_ERROR;
    return HAL_TIM_Base_Start_IT(&htim6);
}

void HAL_SuspendTick(void) { __HAL_TIM_DISABLE_IT(&htim6, TIM_IT_UPDATE); }
void HAL_ResumeTick(void)  { __HAL_TIM_ENABLE_IT(&htim6, TIM_IT_UPDATE); }

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        HAL_IncTick();
    }
}
