#ifndef __GPIO_STM32F1XX_H__
#define __GPIO_STM32F1XX_H__

typedef enum _Drv_Gpio_Port_s
{
    DRV_GPIO_PORT_A = 0U, 
    DRV_GPIO_PORT_B,
    DRV_GPIO_PORT_C,
    DRV_GPIO_PORT_D,
    DRV_GPIO_PORT_E,
    DRV_GPIO_PORT_NUM,
}Drv_Gpio_Port_e;

typedef enum _Drv_Gpio_Pin_s
{
    DRV_GPIO_PIN_0 = 0U, 
    DRV_GPIO_PIN_1,
    DRV_GPIO_PIN_2,
    DRV_GPIO_PIN_3,
    DRV_GPIO_PIN_4,
    DRV_GPIO_PIN_5,
    DRV_GPIO_PIN_6,
    DRV_GPIO_PIN_7,
    DRV_GPIO_PIN_8,
    DRV_GPIO_PIN_9,
    DRV_GPIO_PIN_10,
    DRV_GPIO_PIN_11,
    DRV_GPIO_PIN_12,
    DRV_GPIO_PIN_13,
    DRV_GPIO_PIN_14,
    DRV_GPIO_PIN_15,
    DRV_GPIO_PIN_NUM,
}Drv_Gpio_Pin_e;

#endif
