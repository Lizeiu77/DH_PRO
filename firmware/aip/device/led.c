#include "led.h"

#include <stdint.h>
#include "data_type.h"

#include "bsp.h"
#include "drv_gpio.h"

typedef struct _Dev_Led_Info_s
{
    uint32_t init;
    Dev_Led_Color_e color;
    Dev_Led_Polarity_e polarity;
}Dev_Led_Info_t;

static Dev_Led_Info_t s_led_info = {0};

static int32_t Led_Set_Color(Dev_Led_Color_e color, uint32_t level)
{
    switch(color)
    {
        case DEV_LED_RED:
            Drv_Gpio_Output(LED_RED_PORT, LED_RED_PIN, (Drv_Gpio_Level_e)level);
            break;

        case DEV_LED_COLOR_ALL:
        {
            Drv_Gpio_Output(LED_RED_PORT, LED_RED_PIN, (Drv_Gpio_Level_e)level);
            break;
        }
        
        default:
            break;
    }
    return ERR_OK;
}

int32_t Dev_Led_On(Dev_Led_Color_e color)
{
    uint32_t io_level = 0;

    if(s_led_info.init != 1) {
        return ERR_NG;
    }

    if(color >= DEV_LED_COLOR_NUM) {
        return ERR_NG;
    }

    io_level = s_led_info.polarity;
    Led_Set_Color(color, io_level);

    return ERR_OK;
}

int32_t Dev_Led_AllOff(void)
{
    uint32_t io_level = 0;

    if(s_led_info.init != 1) {
        return ERR_NG;
    }

    if(s_led_info.polarity == DEV_LED_POLARITY_LOW) 
    {
        io_level = (uint32_t)DRV_GPIO_LEVEL_HIGH;
    } 
    else if(s_led_info.polarity == DEV_LED_POLARITY_HIGH) 
    {
        io_level = (uint32_t)DRV_GPIO_LEVEL_LOW;
    }
    Led_Set_Color(DEV_LED_COLOR_ALL, io_level);
    return ERR_OK;
}

int32_t Dev_Led_Set_Polarity(Dev_Led_Polarity_e polarity)
{
    if(polarity >= DEV_LED_POLARITY_NUM) {
        return ERR_NG;
    }

    s_led_info.polarity = polarity;
    return ERR_OK;
}

int32_t Dev_Led_Init(void)
{
    Drv_Gpio_t gpio;

    gpio.port = LED_RED_PORT;
    gpio.pin = LED_RED_PIN;
    gpio.mode = DRV_GPIO_OUTPUT;
    gpio.output_mode = DRV_GPIO_PUSHUP;
    Drv_Gpio_Init(gpio);
    s_led_info.init = 1;
    return ERR_OK;
}

int32_t Dev_Led_Dinit(void)
{
    s_led_info.init = 0;
    return ERR_OK;
}
