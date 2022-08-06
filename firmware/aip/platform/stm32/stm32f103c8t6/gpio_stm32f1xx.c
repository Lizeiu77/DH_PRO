#include "gpio_stm32f1xx.h"

#include <stdint.h>
#include <string.h>
#include "data_type.h"

#include "drv_gpio.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_exti.h"
#include "stm32f1xx_ll_bus.h"

int32_t Drv_Gpio_Read(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e *level) 
__ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Read);

int32_t Drv_Gpio_Output(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e level) 
__ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Output);

int32_t Drv_Gpio_IsrHook(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_IsrHook_f isr)
__ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_IsrHook);

int32_t Drv_Gpio_Init(Drv_Gpio_t gpio) 
__ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Init);

void EXTI0_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_0_Handler);
void EXTI1_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_1_Handler);
void EXTI2_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_2_Handler);
void EXTI3_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_3_Handler);
void EXTI4_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_4_Handler);
void EXTI9_5_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_5to9_Handler);
void EXTI15_10_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Gpio_Exti_10to15_Handler);

/** @defgroup EXTI_LL_EC_LINE LINE
  * @
  */
#define STM32F1XX_GPIO_EXIT_LINE(pin)    ((0x1U) << (pin))

/** @defgroup GPIO_LL_EC_EXTI_LINE GPIO EXTI LINE
  * @
  */
#define STM32F1XX_GPIO_AF_EXIT_LINE(pin)    (((0x000FU) << (((pin)%(4))*(4))) << (16U) | ((pin)>>(2)))

static Drv_Gpio_IsrHook_f s_gpio_isr_map[DRV_GPIO_PIN_NUM] = { NULL };      //interrupt service routine

static uint32_t s_gpio_port_apb2_map[DRV_GPIO_PORT_NUM] = {
    [DRV_GPIO_PORT_A] = LL_APB2_GRP1_PERIPH_GPIOA,
    [DRV_GPIO_PORT_B] = LL_APB2_GRP1_PERIPH_GPIOB,
    [DRV_GPIO_PORT_C] = LL_APB2_GRP1_PERIPH_GPIOC,
    [DRV_GPIO_PORT_D] = LL_APB2_GRP1_PERIPH_GPIOD,
    [DRV_GPIO_PORT_E] = LL_APB2_GRP1_PERIPH_GPIOE,
};

static GPIO_TypeDef *s_gpio_port_map[DRV_GPIO_PORT_NUM] = {
    [DRV_GPIO_PORT_A] = GPIOA,
    [DRV_GPIO_PORT_B] = GPIOB,
    [DRV_GPIO_PORT_C] = GPIOC,
    [DRV_GPIO_PORT_D] = GPIOD,
    [DRV_GPIO_PORT_E] = GPIOE,
};

static uint32_t s_gpio_pin_map[DRV_GPIO_PIN_NUM] = {
    [DRV_GPIO_PIN_0] = LL_GPIO_PIN_0,
    [DRV_GPIO_PIN_1] = LL_GPIO_PIN_1,
    [DRV_GPIO_PIN_2] = LL_GPIO_PIN_2,
    [DRV_GPIO_PIN_3] = LL_GPIO_PIN_3,
    [DRV_GPIO_PIN_4] = LL_GPIO_PIN_4,
    [DRV_GPIO_PIN_5] = LL_GPIO_PIN_5,
    [DRV_GPIO_PIN_6] = LL_GPIO_PIN_6,
    [DRV_GPIO_PIN_7] = LL_GPIO_PIN_7,
    [DRV_GPIO_PIN_8] = LL_GPIO_PIN_8,
    [DRV_GPIO_PIN_9] = LL_GPIO_PIN_9,
    [DRV_GPIO_PIN_10] = LL_GPIO_PIN_10,
    [DRV_GPIO_PIN_11] = LL_GPIO_PIN_11,
    [DRV_GPIO_PIN_12] = LL_GPIO_PIN_12,
    [DRV_GPIO_PIN_13] = LL_GPIO_PIN_13,
    [DRV_GPIO_PIN_14] = LL_GPIO_PIN_14,
    [DRV_GPIO_PIN_15] = LL_GPIO_PIN_15,
};

static IRQn_Type s_gpio_irq_map[DRV_GPIO_PIN_NUM] = {
    [DRV_GPIO_PIN_0] = EXTI0_IRQn,
    [DRV_GPIO_PIN_1] = EXTI1_IRQn,
    [DRV_GPIO_PIN_2] = EXTI2_IRQn,
    [DRV_GPIO_PIN_3] = EXTI3_IRQn,
    [DRV_GPIO_PIN_4] = EXTI4_IRQn,
    [DRV_GPIO_PIN_5] = EXTI9_5_IRQn,
    [DRV_GPIO_PIN_6] = EXTI9_5_IRQn,
    [DRV_GPIO_PIN_7] = EXTI9_5_IRQn,
    [DRV_GPIO_PIN_8] = EXTI9_5_IRQn,
    [DRV_GPIO_PIN_9] = EXTI9_5_IRQn,
    [DRV_GPIO_PIN_10] = EXTI15_10_IRQn,
    [DRV_GPIO_PIN_11] = EXTI15_10_IRQn,
    [DRV_GPIO_PIN_12] = EXTI15_10_IRQn,
    [DRV_GPIO_PIN_13] = EXTI15_10_IRQn,
    [DRV_GPIO_PIN_14] = EXTI15_10_IRQn,
    [DRV_GPIO_PIN_15] = EXTI15_10_IRQn,
};

int32_t Stm32f1xx_Gpio_Read(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e *level)
{
    if(port >= DRV_GPIO_PORT_NUM || pin >= DRV_GPIO_PIN_NUM)
    {
        return ERR_NG;
    }
    if(level == NULL) {
        return ERR_NG;
    }
    *level = (Drv_Gpio_Level_e)LL_GPIO_IsInputPinSet(s_gpio_port_map[port], s_gpio_pin_map[pin]);
    return ERR_OK;
}

int32_t Stm32f1xx_Gpio_Output(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_Level_e level)
{
    if(port >= DRV_GPIO_PORT_NUM || pin >= DRV_GPIO_PIN_NUM)
    {
        return ERR_NG;
    }
    if(level == DRV_GPIO_LEVEL_HIGH) {
        LL_GPIO_SetOutputPin(s_gpio_port_map[port], s_gpio_pin_map[pin]);
    } else {
        LL_GPIO_ResetOutputPin(s_gpio_port_map[port], s_gpio_pin_map[pin]);
    }
    return ERR_OK;
}

int32_t Stm32f1xx_Gpio_IsrHook(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin, Drv_Gpio_IsrHook_f isr)
{
    if(isr == NULL)
    {
        return ERR_NG;
    }
    s_gpio_isr_map[pin] = isr;
    return ERR_OK;
}

void Stm32f1xx_Gpio_Exti_0_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_0;
    
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

void Stm32f1xx_Gpio_Exti_1_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_1;

    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

void Stm32f1xx_Gpio_Exti_2_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_2;
    
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

void Stm32f1xx_Gpio_Exti_3_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_3;
    
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

void Stm32f1xx_Gpio_Exti_4_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_4;
    
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

void Stm32f1xx_Gpio_Exti_5to9_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_5;
    
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_6;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_7;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_8;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_9;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

void Stm32f1xx_Gpio_Exti_10to15_Handler(void)
{
    uint32_t pin = DRV_GPIO_PIN_10;
    
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_11;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_12;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_13;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_14;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }

    pin = DRV_GPIO_PIN_15;
    if (LL_EXTI_IsActiveFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin)) != RESET)
    {
        LL_EXTI_ClearFlag_0_31(STM32F1XX_GPIO_EXIT_LINE(pin));
        if(s_gpio_isr_map[pin] != NULL) {
            s_gpio_isr_map[pin]();
        }
    }
}

int32_t Stm32f1xx_Gpio_Init(Drv_Gpio_t gpio)
{
    int32_t rval = 0;
    LL_GPIO_InitTypeDef gpio_init = {0};
    uint32_t gpio_enable_clk = 0;

    if(gpio.port >= DRV_GPIO_PORT_NUM || gpio.pin >= DRV_GPIO_PIN_NUM)
    {
        return ERR_NG;
    }

    if(gpio.mode >= DRV_GPIO_MODE_NUM)
    {
        return ERR_NG;
    }

    gpio_enable_clk = s_gpio_port_apb2_map[gpio.port];

    LL_APB2_GRP1_EnableClock(gpio_enable_clk);

    gpio_init.Pin = s_gpio_pin_map[gpio.pin];
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_LOW;
    //set gpio pull of input mode
    if(gpio.intput_mode == DRV_GPIO_PULLUP) {
        gpio_init.Pull = LL_GPIO_PULL_UP;
    } else if(gpio.intput_mode == DRV_GPIO_PULLDOWN){
        gpio_init.Pull = LL_GPIO_PULL_DOWN;
    }

    switch(gpio.mode)
    {
        case DRV_GPIO_INPUT:
        {
            gpio_init.Mode = LL_GPIO_MODE_INPUT;
            break;
        }

        case DRV_GPIO_OUTPUT:
        {
            gpio_init.Mode = LL_GPIO_MODE_OUTPUT;
            if(gpio.output_mode >= DRV_GPIO_OUTPUTMODE_NUM) {
                return ERR_NG;
            }
            if(gpio.output_mode == DRV_GPIO_PUSHUP) {
                gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
            } else if(gpio.output_mode == DRV_GPIO_OPENDRAIN) {
                gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
            }
            break;
        }

        case DRV_GPIO_EXIT:
        {
            LL_EXTI_InitTypeDef exit_init = {0};

            LL_GPIO_AF_SetEXTISource((uint32_t)gpio.port, STM32F1XX_GPIO_AF_EXIT_LINE(gpio.pin));

            exit_init.Line_0_31 = STM32F1XX_GPIO_EXIT_LINE(gpio.pin);
            exit_init.LineCommand = ENABLE;
            exit_init.Mode = LL_EXTI_MODE_IT;

            //set edge direction of external interrupt mode
            if(gpio.trigger == DRV_GPIO_RISING) {
                exit_init.Trigger = LL_EXTI_TRIGGER_RISING;
            } else if(gpio.trigger == DRV_GPIO_FALLING) {
                exit_init.Trigger = LL_EXTI_TRIGGER_FALLING;
            } else if(gpio.trigger == DRV_GPIO_RISING_FALLING) {
                exit_init.Trigger = LL_EXTI_TRIGGER_RISING_FALLING;
            }

            //set gpio mode
            gpio_init.Mode = LL_GPIO_MODE_INPUT;

            NVIC_SetPriority(s_gpio_irq_map[gpio.pin], NVIC_EncodePriority(NVIC_GetPriorityGrouping(),2, 0));
            NVIC_EnableIRQ(s_gpio_irq_map[gpio.pin]);
            
            LL_EXTI_Init(&exit_init);
            break;
        }

        default:
            break;
    }

    rval = LL_GPIO_Init(s_gpio_port_map[gpio.port], &gpio_init);

    return rval;
}

int32_t Stm32f1xx_Gpio_Deinit(Drv_Gpio_Port_e port, Drv_Gpio_Pin_e pin)
{
    return ERR_OK;
}


