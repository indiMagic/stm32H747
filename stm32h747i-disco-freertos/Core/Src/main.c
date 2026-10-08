/**
 * STM32H747I-DISCO - Cortex-M7 FreeRTOS starter
 *
 *  - 400 MHz core clock from the 25 MHz HSE (PLL1), SMPS direct supply
 *  - I/D cache enabled
 *  - HAL tick on TIM6, FreeRTOS tick on SysTick
 *  - printf() routed to USART1 (ST-LINK virtual COM port, 115200 8N1)
 *  - Demo tasks: LED blinker, producer -> queue -> consumer
 */
#include <stdio.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

UART_HandleTypeDef huart1;

static QueueHandle_t xDemoQueue;

static void SystemClock_Config(void);
static void GPIO_Init(void);
static void USART1_Init(void);

/* ------------------------------------------------------------------ */
/* Tasks                                                               */
/* ------------------------------------------------------------------ */
static void vBlinkTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWake = xTaskGetTickCount();
    for (;;) {
        HAL_GPIO_TogglePin(LED_PORT, LD1_GREEN_PIN);
        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(500));
    }
}

static void vProducerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulCount = 0;
    for (;;) {
        ulCount++;
        if (xQueueSend(xDemoQueue, &ulCount, pdMS_TO_TICKS(10)) != pdPASS) {
            printf("producer: queue full\r\n");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void vConsumerTask(void *pvParameters)
{
    (void)pvParameters;
    uint32_t ulValue;
    for (;;) {
        if (xQueueReceive(xDemoQueue, &ulValue, portMAX_DELAY) == pdPASS) {
            HAL_GPIO_TogglePin(LED_PORT, LD2_ORANGE_PIN);
            printf("consumer: got %lu  (free heap %u B, min stack left %u words)\r\n",
                   (unsigned long)ulValue,
                   (unsigned)xPortGetFreeHeapSize(),
                   (unsigned)uxTaskGetStackHighWaterMark(NULL));
        }
    }
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
int main(void)
{
    SCB_EnableICache();
    SCB_EnableDCache();

    HAL_Init();                 /* sets NVIC priority grouping 4, starts TIM6 tick */
    SystemClock_Config();
    GPIO_Init();
    USART1_Init();

    printf("\r\nSTM32H747I-DISCO CM7 FreeRTOS %s starting, SYSCLK=%lu Hz\r\n",
           tskKERNEL_VERSION_NUMBER, (unsigned long)SystemCoreClock);

    xDemoQueue = xQueueCreate(8, sizeof(uint32_t));
    configASSERT(xDemoQueue != NULL);

    BaseType_t ok = pdPASS;
    ok &= xTaskCreate(vBlinkTask,    "blink",    configMINIMAL_STACK_SIZE,     NULL, 1, NULL);
    ok &= xTaskCreate(vProducerTask, "producer", configMINIMAL_STACK_SIZE,     NULL, 2, NULL);
    ok &= xTaskCreate(vConsumerTask, "consumer", configMINIMAL_STACK_SIZE * 2, NULL, 3, NULL);
    configASSERT(ok == pdPASS);

    vTaskStartScheduler();

    /* Only reached if there was not enough heap for the idle task */
    Error_Handler();
    for (;;) { }
}

/* ------------------------------------------------------------------ */
/* Clock: HSE 25 MHz -> PLL1 -> 400 MHz SYSCLK                         */
/*   VCO = 25/5*160 = 800 MHz; P = /2 = 400 MHz; Q = /4 = 200 MHz     */
/*   AHB 200 MHz, APB1/2/3/4 100 MHz                                   */
/* ------------------------------------------------------------------ */
static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    /* The H747I-DISCO board is powered through the internal SMPS (direct supply) */
    HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);
    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) { }

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState       = RCC_HSE_ON;
    osc.PLL.PLLState   = RCC_PLL_ON;
    osc.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM       = 5;
    osc.PLL.PLLN       = 160;
    osc.PLL.PLLP       = 2;
    osc.PLL.PLLQ       = 4;
    osc.PLL.PLLR       = 2;
    osc.PLL.PLLRGE     = RCC_PLL1VCIRANGE_2;
    osc.PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE;
    osc.PLL.PLLFRACN   = 0;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();

    clk.ClockType      = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 |
                         RCC_CLOCKTYPE_PCLK1  | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1;
    clk.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    clk.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    clk.AHBCLKDivider  = RCC_HCLK_DIV2;
    clk.APB3CLKDivider = RCC_APB3_DIV2;
    clk.APB1CLKDivider = RCC_APB1_DIV2;
    clk.APB2CLKDivider = RCC_APB2_DIV2;
    clk.APB4CLKDivider = RCC_APB4_DIV2;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_4) != HAL_OK) Error_Handler();
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef g = {0};
    __HAL_RCC_GPIOI_CLK_ENABLE();

    HAL_GPIO_WritePin(LED_PORT, LD1_GREEN_PIN | LD2_ORANGE_PIN | LD3_RED_PIN | LD4_BLUE_PIN, GPIO_PIN_RESET);
    g.Pin   = LD1_GREEN_PIN | LD2_ORANGE_PIN | LD3_RED_PIN | LD4_BLUE_PIN;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_PORT, &g);
}

static void USART1_Init(void)
{
    huart1.Instance                    = USART1;
    huart1.Init.BaudRate               = 115200;
    huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    huart1.Init.StopBits               = UART_STOPBITS_1;
    huart1.Init.Parity                 = UART_PARITY_NONE;
    huart1.Init.Mode                   = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

/* Low-level init called from HAL_UART_Init() */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        GPIO_InitTypeDef g = {0};
        RCC_PeriphCLKInitTypeDef pc = {0};

        pc.PeriphClockSelection = RCC_PERIPHCLK_USART16;
        pc.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
        if (HAL_RCCEx_PeriphCLKConfig(&pc) != HAL_OK) Error_Handler();

        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        g.Pin       = GPIO_PIN_9 | GPIO_PIN_10;   /* PA9 TX, PA10 RX -> ST-LINK VCP */
        g.Mode      = GPIO_MODE_AF_PP;
        g.Pull      = GPIO_PULLUP;
        g.Speed     = GPIO_SPEED_FREQ_LOW;
        g.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &g);
    }
}

/* ------------------------------------------------------------------ */
/* Error / FreeRTOS hooks                                              */
/* ------------------------------------------------------------------ */
void Error_Handler(void)
{
    __disable_irq();
    HAL_GPIO_WritePin(LED_PORT, LD3_RED_PIN, GPIO_PIN_SET);
    for (;;) { }
}

void vApplicationMallocFailedHook(void)
{
    Error_Handler();
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask; (void)pcTaskName;
    Error_Handler();
}
