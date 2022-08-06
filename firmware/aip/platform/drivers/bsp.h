#ifndef __BSP_H__
#define __BSP_H__

#include "drv_gpio.h"

#ifdef PROJECT_DEMO

#define LED_RED_PORT     DRV_GPIO_PORT_C
#define LED_RED_PIN      DRV_GPIO_PIN_13

#define UART_DEBUG_TX_PORT    DRV_GPIO_PORT_A
#define UART_DEBUG_TX_PIN     DRV_GPIO_PIN_9

#define UART_DEBUG_RX_PORT    DRV_GPIO_PORT_A
#define UART_DEBUG_RX_PIN     DRV_GPIO_PIN_10

#endif


#endif
