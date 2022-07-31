#ifndef __LED_H__
#define __LED_H__

#include <stdint.h>

typedef enum _Dev_Led_Color_s
{
    DEV_LED_COLOR_NONE = 0,
    DEV_LED_RED,
    DEV_LED_GREEN,
    DEV_LED_BLUE,
    DEV_LED_COLOR_ALL,
    DEV_LED_COLOR_NUM,
}Dev_Led_Color_e;

typedef enum _Dev_Led_Polarity_s
{
    DEV_LED_POLARITY_LOW = 0U,
    DEV_LED_POLARITY_HIGH,
    DEV_LED_POLARITY_NUM,
}Dev_Led_Polarity_e;

int32_t Dev_Led_On(Dev_Led_Color_e color);
int32_t Dev_Led_AllOff(void);
int32_t Dev_Led_Set_Polarity(Dev_Led_Polarity_e polarity);
int32_t Dev_Led_Init(void);
int32_t Dev_Led_Dinit(void);

#endif
