#include "drv_gpio.h"

#include "data_type.h"

__ATTRIBUTE_WEAK__ int32_t Drv_Gpio_Init(Drv_Gpio_t gpio)
{
    return ERR_NG;
} 

__ATTRIBUTE_WEAK__ int32_t Drv_Gpio_Read(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e *level)
{
    return ERR_NG;
} 

__ATTRIBUTE_WEAK__ int32_t Drv_Gpio_Output(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e level)
{
    return ERR_NG;
} 

__ATTRIBUTE_WEAK__ int32_t Drv_Gpio_IsrHook(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_IsrHook_f isr)
{
    return ERR_NG;
}
