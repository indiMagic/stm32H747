#ifndef MAIN_H
#define MAIN_H

#include "stm32h7xx_hal.h"

/* STM32H747I-DISCO user LEDs (active low is NOT used here: LEDs are active-high on PI12..PI15) */
#define LD1_GREEN_PIN    GPIO_PIN_12   /* PI12 */
#define LD2_ORANGE_PIN   GPIO_PIN_13   /* PI13 */
#define LD3_RED_PIN      GPIO_PIN_14   /* PI14 */
#define LD4_BLUE_PIN     GPIO_PIN_15   /* PI15 */
#define LED_PORT         GPIOI

extern UART_HandleTypeDef huart1;   /* USART1 -> ST-LINK virtual COM port (PA9/PA10) */

void Error_Handler(void);

#endif /* MAIN_H */
