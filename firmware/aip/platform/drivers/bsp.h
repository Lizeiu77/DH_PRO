#ifndef __BSP_H__
#define __BSP_H__

#include "drv_gpio.h"

#if (PLATFORM == PLATFORM_STM32F103X)

#define LED_RED_PORT     DRV_GPIO_PORT_B
#define LED_RED_PIN      DRV_GPIO_PIN_2

#endif


#endif
