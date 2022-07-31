#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__

#include <stdint.h>

#ifdef PLATFORM_STM32F103X
    #include "gpio_stm32f1xx.h"
#else
typedef enum _Drv_Gpio_Port_s
{
    DRV_GPIO_PORT_A = 0U, 
    DRV_GPIO_PORT_B,
    DRV_GPIO_PORT_C,
    DRV_GPIO_PORT_D,
    DRV_GPIO_PORT_E,
    DRV_GPIO_PORT_F,
    DRV_GPIO_PORT_G,

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

    DRV_GPIO_PIN_NUM,
}Drv_Gpio_Pin_e;

#endif

typedef enum _Drv_Gpio_Mode_s
{
    DRV_GPIO_INPUT = 0U,
    DRV_GPIO_OUTPUT,
    DRV_GPIO_EXIT,          //external interrupt
    // DRV_GPIO_ALTERNATE,     //alternate function
    DRV_GPIO_MODE_NUM,
}Drv_Gpio_Mode_e;

typedef enum _Drv_Gpio_IntputMode_s
{
    DRV_GPIO_FLOATING = 0U,
    DRV_GPIO_PULLUP,
    DRV_GPIO_PULLDOWN,
    DRV_GPIO_INTPUTMODE_NUM,
}Drv_Gpio_IntputMode_e;

typedef enum _Drv_Gpio_OutputMode_s
{
    DRV_GPIO_PUSHUP = 0U,
    DRV_GPIO_OPENDRAIN,
    DRV_GPIO_OUTPUTMODE_NUM,
}Drv_Gpio_OutputMode_e;

typedef enum _Drv_Gpio_Level_s
{
    DRV_GPIO_LEVEL_LOW = 0U,
    DRV_GPIO_LEVEL_HIGH,
    DRV_GPIO_LEVEL_NUM,
}Drv_Gpio_Level_e;

typedef enum _Drv_Gpio_Trigger_s
{
    DRV_GPIO_RISING = 0U,
    DRV_GPIO_FALLING,
    DRV_GPIO_RISING_FALLING,
    DRV_GPIO_TRIGGER_NUM,
}Drv_Gpio_Trigger_e;

typedef void (*Drv_Gpio_IsrHook_f)(void);

typedef struct _Drv_Gpio_s
{
    Drv_Gpio_Port_e port;
    Drv_Gpio_Pin_e pin;
    Drv_Gpio_Mode_e mode;
    Drv_Gpio_IntputMode_e intput_mode;
    Drv_Gpio_OutputMode_e output_mode;
    Drv_Gpio_Level_e level;
    Drv_Gpio_Trigger_e trigger;
}Drv_Gpio_t;

int32_t Drv_Gpio_Init(Drv_Gpio_t gpio);
int32_t Drv_Gpio_Read(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e *level);
int32_t Drv_Gpio_Output(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e level);
int32_t Drv_Gpio_IsrHook(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_IsrHook_f isr);

#endif
