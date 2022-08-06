#include "drv_uart.h"

#include "data_type.h"
#include <stdint.h>
#include <string.h>

#include "fifo.h"

#include "stm32f103xb.h"
#include "stm32f1xx_ll_usart.h"
#include "stm32f1xx_ll_gpio.h"
#include "stm32f1xx_ll_bus.h"

typedef enum _Stm32f1xx_Uart_Direct_s
{
    STM32F1XX_UART_DIRECT_TX = 0U,
    STM32F1XX_UART_DIRECT_RX,
    STM32F1XX_UART_DIRECT_NUM,
}Stm32f1xx_Uart_Direct_e;

typedef struct _Stm32f1xx_Uart_Info_s
{
    uint8_t init;
    Drv_Uart_Config_t config;
}Stm32f1xx_Uart_Info_t;

void USART1_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Channel_1_IntHandler);
void USART2_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Channel_2_IntHandler);
void USART3_IRQHandler(void) __ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Channel_3_IntHandler);

int32_t Drv_Uart_Send(Drv_Uart_Channel_e channel, uint8_t *data, uint32_t size, uint32_t *act_size)
__ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Send);

int32_t Drv_Uart_Recv(Drv_Uart_Channel_e channel, uint32_t size, uint8_t *data, uint32_t *act_size)
__ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Recv);

int32_t Drv_Uart_IsrHook(Drv_Uart_Channel_e channel, Drv_Uart_Isr_f isr)
__ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_IsrHook);

int32_t Drv_Uart_Init(Drv_Uart_Config_t *config)
__ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Init);

int32_t Drv_Uart_Deinit(Drv_Uart_Channel_e channel)
__ATTRIBUTE_ALIAS__(Stm32f1xx_Uart_Deinit);

static USART_TypeDef *(s_uart_channel_instance_map)[DRV_UART_CHANNEL_NUM] = {
    [DRV_UART_CHANNEL_1] = USART1,
    [DRV_UART_CHANNEL_2] = USART2,
    [DRV_UART_CHANNEL_3] = USART3,
};

static Stm32f1xx_Uart_Info_t s_uart_info[DRV_UART_CHANNEL_NUM] = {0};
static Drv_Uart_Isr_f s_uart_isr_map[DRV_UART_CHANNEL_NUM] = { NULL };
static Common_Fifo_t s_uart_fifo[DRV_UART_CHANNEL_NUM][STM32F1XX_UART_DIRECT_NUM] = {0};

/**
 * @brief Check if the usartx error flag is set or not
 * 
 * @param USARTx 
 * @return Error status 
 */
static uint32_t Stm32f1xx_Uart_IsActiveFlag(USART_TypeDef *usart_x)
{
    uint32_t flag = 0;

    if(LL_USART_IsActiveFlag_FE(usart_x)) {
        //Check USART Framing Error
        ADD_FLAG(flag, DRV_UART_ERR_FRAME_ERR_01);
        LL_USART_ClearFlag_FE(usart_x);
    }

    if(LL_USART_IsActiveFlag_NE(usart_x)) {
        //Check USART Noise error detected
        ADD_FLAG(flag, DRV_UART_ERR_NOISE_ERR_02);
        LL_USART_ClearFlag_NE(usart_x);
    }

    if(LL_USART_IsActiveFlag_ORE(usart_x)) {
        //Check USART OverRun Error
        ADD_FLAG(flag, DRV_UART_ERR_OVERRUN_ERR_04);
        LL_USART_ClearFlag_ORE(usart_x);
    }

    if(LL_USART_IsActiveFlag_PE(usart_x)) {
        //Check USART Parity Error
        ADD_FLAG(flag, DRV_UART_ERR_PARITY_ERR_08);
        LL_USART_ClearFlag_PE(usart_x);
    }

    return flag;
}

/**
 * @brief Uart trasnmit in interrupt mode
 * 
 * @param channel 
 * @return int32_t 
 */
static int32_t Stm32f1xx_Uart_Int_Transmit(Drv_Uart_Channel_e channel)
{
    USART_TypeDef *usart_x = s_uart_channel_instance_map[channel];
    uint32_t act_size = 0;
    int32_t rval = 0;
    uint8_t ch = 0;

    //transmit 8bit or 9bit
    if(LL_USART_IsActiveFlag_TC(usart_x))
    {
        LL_USART_ClearFlag_TC(usart_x);
        rval = Common_Fifo_Read(&s_uart_fifo[channel][STM32F1XX_UART_DIRECT_TX], &ch, 1, &act_size);
        if(rval == ERR_OK) {
            LL_USART_TransmitData8(usart_x, ch);
        } else {
            rval = DRV_UART_ERR_TRANSMIT_COMPLETE_1000;
        }
    }

    return rval;
}

/**
 * @brief Uart receive in interrupt mode
 * 
 * @param channel 
 * @return int32_t 
 */
static int32_t Stm32f1xx_Uart_Int_Receive(Drv_Uart_Channel_e channel)
{
    USART_TypeDef *usart_x = s_uart_channel_instance_map[channel];
    int32_t rval = 0;
	uint32_t act_size = 0;
	uint8_t ch = 0;

    //receive 8bit or 9bit
    if(LL_USART_IsActiveFlag_RXNE(usart_x)) 
    {
        LL_USART_ClearFlag_RXNE(usart_x);
        ch = LL_USART_ReceiveData8(usart_x);
        rval = Common_Fifo_Write(&s_uart_fifo[channel][STM32F1XX_UART_DIRECT_RX], &ch, 1, &act_size);
        if(rval != ERR_OK) {
            rval = DRV_UART_ERR_FIFO_FULL_100;
        }
    }
    return rval;
}

/**
 * @brief Uart1 interrupt handler
 * 
 */
static void Stm32f1xx_Uart_Channel_1_IntHandler(void)
{
    Drv_Uart_Cb_Context_s cb_ctx = {0};
    int32_t rval = 0;

    if(NVIC_GetPendingIRQ(USART1_IRQn)) {
        NVIC_ClearPendingIRQ(USART1_IRQn);
    } else {
        return;
    }

    cb_ctx.err_status = Stm32f1xx_Uart_IsActiveFlag(USART1);

    //transmission
    rval = Stm32f1xx_Uart_Int_Transmit(DRV_UART_CHANNEL_1);
    if(rval == DRV_UART_ERR_TRANSMIT_COMPLETE_1000) {
        ADD_FLAG(cb_ctx.err_status, DRV_UART_ERR_TRANSMIT_COMPLETE_1000);
    }
    
    //reception
    rval = Stm32f1xx_Uart_Int_Receive(DRV_UART_CHANNEL_1);
    if(rval == DRV_UART_ERR_FIFO_FULL_100) {
        ADD_FLAG(cb_ctx.err_status, DRV_UART_ERR_FIFO_FULL_100);
    }

    //callback
    if(s_uart_isr_map[DRV_UART_CHANNEL_1] != NULL) {
        s_uart_isr_map[DRV_UART_CHANNEL_1](&cb_ctx);
    }
}

static void Stm32f1xx_Uart_Channel_2_IntHandler(void)
{
    Drv_Uart_Cb_Context_s cb_ctx = {0};
    int32_t rval = 0;

    if(NVIC_GetPendingIRQ(USART2_IRQn)) {
        NVIC_ClearPendingIRQ(USART2_IRQn);
    } else {
        return;
    }

    cb_ctx.err_status = Stm32f1xx_Uart_IsActiveFlag(USART2);

    rval = Stm32f1xx_Uart_Int_Transmit(DRV_UART_CHANNEL_2);
    if(rval == DRV_UART_ERR_TRANSMIT_COMPLETE_1000) {
        ADD_FLAG(cb_ctx.err_status, DRV_UART_ERR_TRANSMIT_COMPLETE_1000);
    }
    
    rval = Stm32f1xx_Uart_Int_Receive(DRV_UART_CHANNEL_2);
    if(rval == DRV_UART_ERR_FIFO_FULL_100) {
        ADD_FLAG(cb_ctx.err_status, DRV_UART_ERR_FIFO_FULL_100);
    }

    //callback
    if(s_uart_isr_map[DRV_UART_CHANNEL_2] != NULL) {
        s_uart_isr_map[DRV_UART_CHANNEL_2](&cb_ctx);
    }
}

static void Stm32f1xx_Uart_Channel_3_IntHandler(void)
{
    Drv_Uart_Cb_Context_s cb_ctx = {0};
    int32_t rval = 0;

    if(NVIC_GetPendingIRQ(USART3_IRQn)) {
        NVIC_ClearPendingIRQ(USART3_IRQn);
    } else {
        return;
    }

    cb_ctx.err_status = Stm32f1xx_Uart_IsActiveFlag(USART3);

    rval = Stm32f1xx_Uart_Int_Transmit(DRV_UART_CHANNEL_3);
    if(rval == DRV_UART_ERR_TRANSMIT_COMPLETE_1000) {
        ADD_FLAG(cb_ctx.err_status, DRV_UART_ERR_TRANSMIT_COMPLETE_1000);
    }
    
    rval = Stm32f1xx_Uart_Int_Receive(DRV_UART_CHANNEL_3);
    if(rval == DRV_UART_ERR_FIFO_FULL_100) {
        ADD_FLAG(cb_ctx.err_status, DRV_UART_ERR_FIFO_FULL_100);
    }

    //callback
    if(s_uart_isr_map[DRV_UART_CHANNEL_3] != NULL) {
        s_uart_isr_map[DRV_UART_CHANNEL_3](&cb_ctx);
    }
}

static int32_t Stm32f1xx_Uart_Transmit_Normal(Drv_Uart_Config_t *config, uint8_t *data, uint32_t size, uint32_t *act_size)
{
    USART_TypeDef *usart_x = s_uart_channel_instance_map[config->channel];
    int32_t rval = 0;

    for(uint32_t i=0; i<size; i++)
    {
        while(LL_USART_IsActiveFlag_TXE(usart_x) == 0)
        {
            //timeout
            if(act_size != NULL) {
                *act_size = i;
            }
            return ERR_NG;
        }
        LL_USART_TransmitData8(usart_x, data[i]);
    }

    if(act_size != NULL) {
        *act_size = size;
    }
    
    return rval;
}

static int32_t Stm32f1xx_Uart_Transmit_Interrupt(Drv_Uart_Config_t *config, uint8_t *data, uint32_t size, uint32_t *act_size)
{
    USART_TypeDef *usart_x = s_uart_channel_instance_map[config->channel];
    int32_t rval = 0;
    uint8_t ch = *data;

    rval = Common_Fifo_Write(&s_uart_fifo[config->channel][STM32F1XX_UART_DIRECT_TX], data + 1, size, act_size);
    if(rval != ERR_OK) {
        return rval;
    }

    LL_USART_TransmitData8(usart_x, ch);

    return ERR_OK;
}

__ATTRIBUTE_UNUSED__ static int32_t Stm32f1xx_Uart_Transmit_DMA()
{
    //
    return ERR_NG;
}

static int32_t Stm32f1xx_Uart_Receive_Normal(Drv_Uart_Config_t *config, uint32_t size, uint8_t *data, uint32_t *act_size)
{
    USART_TypeDef *usart_x = s_uart_channel_instance_map[config->channel];
    int32_t rval = 0;

    for(uint32_t i=0; i<size; i++)
    {
        while(LL_USART_IsActiveFlag_RXNE(usart_x))
        {
            //timeout
            if(act_size != NULL) {
                *act_size = i;
            }
            return ERR_NG;
        }
        data[i] = LL_USART_ReceiveData8(usart_x);
    }
    if(act_size != NULL) {
        *act_size = size;
    }
    return rval;
}

__ATTRIBUTE_UNUSED__ static int32_t Stm32f1xx_Uart_Receive_DMA()
{
    //
    return ERR_NG;
}

static int32_t Stm32f1xx_Uart_Receive_Interrupt(Drv_Uart_Config_t *config, uint32_t size, uint8_t *data, uint32_t *act_size)
{
    int32_t rval = 0;
    
    rval = Common_Fifo_Read(&s_uart_fifo[config->channel][STM32F1XX_UART_DIRECT_RX], data, size, act_size);
    return rval;
}

static void Stm32f1xx_Uart_Channel_1_Gpio_Init(void)
{
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    /**USART1 GPIO Configuration
     PA9   ------> USART1_TX
    PA10   ------> USART1_RX
    */
    gpio_init.Pin = LL_GPIO_PIN_9;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = LL_GPIO_PIN_10;
    gpio_init.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(GPIOA, &gpio_init);
}   

static void Stm32f1xx_Uart_Channel_2_Gpio_Init(void)
{
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
    /**USART1 GPIO Configuration
     PA9   ------> USART1_TX
    PA10   ------> USART1_RX
    */
    gpio_init.Pin = LL_GPIO_PIN_2;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = LL_GPIO_PIN_3;
    gpio_init.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(GPIOA, &gpio_init);
}

static void Stm32f1xx_Uart_Channel_3_Gpio_Init(void)
{
    LL_GPIO_InitTypeDef gpio_init = {0};
    
    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    /**USART1 GPIO Configuration
     PA9   ------> USART1_TX
    PA10   ------> USART1_RX
    */
    gpio_init.Pin = LL_GPIO_PIN_9;
    gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
    gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    gpio_init.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = LL_GPIO_PIN_10;
    gpio_init.Mode = LL_GPIO_MODE_FLOATING;
    LL_GPIO_Init(GPIOA, &gpio_init);
}

static void Stm32f1xx_Uart_Gpio_Init(Drv_Uart_Channel_e channel)
{
    switch(channel)
    {
        case DRV_UART_CHANNEL_1:
        {
            Stm32f1xx_Uart_Channel_1_Gpio_Init();
            break;
        }
        case DRV_UART_CHANNEL_2:
        {
            Stm32f1xx_Uart_Channel_2_Gpio_Init();
            break;
        }
        case DRV_UART_CHANNEL_3:
        {
            Stm32f1xx_Uart_Channel_3_Gpio_Init();
            break;
        }
        default: 
            break;
    }
}

static void Stm32f1xx_Uart_Clock_Init(Drv_Uart_Channel_e channel)
{
    switch(channel)
    {
        case DRV_UART_CHANNEL_1:
        {
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
            break;
        }
        case DRV_UART_CHANNEL_2:
        {
            LL_APB2_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
            break;
        }
        case DRV_UART_CHANNEL_3:
        {
            LL_APB2_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);
            break;
        }
        default: 
            break;
    }
}

static void Stm32f1xx_Uart_Clock_Deinit(Drv_Uart_Channel_e channel)
{
    switch(channel)
    {
        case DRV_UART_CHANNEL_1:
        {
            LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
            break;
        }
        case DRV_UART_CHANNEL_2:
        {
            LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART2);
            break;
        }
        case DRV_UART_CHANNEL_3:
        {
            LL_APB1_GRP1_DisableClock(LL_APB1_GRP1_PERIPH_USART3);
            break;
        }
        default: 
            break;
    }
}

static int32_t Stm32f1xx_Uart_Config_Init(Drv_Uart_Config_t *config)
{
    LL_USART_InitTypeDef usart_init = {0};
    USART_TypeDef *uart_def = NULL; 
    IRQn_Type irq;
    int32_t rval = 0;

    //baud config
    usart_init.BaudRate = config->baud;

    //data width config
    if(config->data_width == DRV_UART_DATA_WIDTH_8Bit) {
        usart_init.DataWidth = LL_USART_DATAWIDTH_8B;
    } else if(config->data_width == DRV_UART_DATA_WIDTH_9Bit) {
        usart_init.DataWidth = LL_USART_DATAWIDTH_9B;
    } else {
        return ERR_NG;
    }

    //stop bits config
    if(config->stop_bit == DRV_UART_DATA_STOP_1BIT) {
        usart_init.StopBits = LL_USART_STOPBITS_1;
    } else if(config->stop_bit == DRV_UART_DATA_STOP_1_5BIT) {
        usart_init.StopBits = LL_USART_STOPBITS_1_5;
    } else if(config->stop_bit == DRV_UART_DATA_STOP_2BIT) {
        usart_init.StopBits = LL_USART_STOPBITS_2;
    } else {
        return ERR_NG;
    }
    
    usart_init.Parity = LL_USART_PARITY_NONE;
    usart_init.TransferDirection = LL_USART_DIRECTION_TX_RX;

    //hardware flow ctrl
    if(config->hw_flow == DRV_UART_HARDWARE_FLOWCTRL_NONE) {
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    } else if(config->hw_flow == DRV_UART_HARDWARE_FLOWCTRL_RTS) {
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_RTS;
    } else if(config->hw_flow == DRV_UART_HARDWARE_FLOWCTRL_CTS) {
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_CTS;
    } else if(config->hw_flow == DRV_UART_HARDWARE_FLOWCTRL_RTS_CTS) {
        usart_init.HardwareFlowControl = LL_USART_HWCONTROL_RTS_CTS;
    } else {
        return ERR_NG;
    }
    
    usart_init.OverSampling = LL_USART_OVERSAMPLING_16;

    if(config->channel == DRV_UART_CHANNEL_1) {
        uart_def = USART1;
        irq = USART1_IRQn;
    } else if(config->channel == DRV_UART_CHANNEL_2) {
        uart_def = USART2;
        irq = USART2_IRQn;
    } else if(config->channel == DRV_UART_CHANNEL_3) {
        uart_def = USART3;
        irq = USART3_IRQn;
    }
    
    rval = LL_USART_Init(uart_def, &usart_init);
    if(rval != ERR_OK) {
        return rval;
    }
    LL_USART_ConfigAsyncMode(uart_def);
    LL_USART_Enable(uart_def);
    

    //interrupt mode
    if(config->mode == DRV_UART_INT) {
        LL_USART_EnableIT_TC(s_uart_channel_instance_map[config->channel]);
        LL_USART_EnableIT_RXNE(s_uart_channel_instance_map[config->channel]);
        NVIC_SetPriority(irq, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
        NVIC_EnableIRQ(irq);
    }

    return rval;
}

int32_t Stm32f1xx_Uart_Send(Drv_Uart_Channel_e channel, uint8_t *data, uint32_t size, uint32_t *act_size)
{
    uint32_t rval = 0;

    if(s_uart_info[channel].init == 0) {
        return ERR_NG;
    }

    if(data == NULL) {
        return ERR_NG;
    }

    switch(s_uart_info[channel].config.mode)
    {
        case DRV_UART_NORMAL:
            rval = Stm32f1xx_Uart_Transmit_Normal(&s_uart_info[channel].config, data, size, act_size);
            break;

        case DRV_UART_INT:
            rval = Stm32f1xx_Uart_Transmit_Interrupt(&s_uart_info[channel].config, data, size, act_size);
            break;

        case DRV_UART_DMA:
            break;

        default:
            rval = ERR_NG;
    }

    return rval;
}

int32_t Stm32f1xx_Uart_Recv(Drv_Uart_Channel_e channel, uint32_t size, uint8_t *data, uint32_t *act_size)
{
    uint32_t rval = 0;

    if(s_uart_info[channel].init == 0) {
        return ERR_NG;
    }

    if(data == NULL) {
        return ERR_NG;
    }

    switch(s_uart_info[channel].config.mode)
    {
        case DRV_UART_NORMAL:
            rval = Stm32f1xx_Uart_Receive_Normal(&s_uart_info[channel].config, size, data, act_size);
            break;

        case DRV_UART_INT:
            rval = Stm32f1xx_Uart_Receive_Interrupt(&s_uart_info[channel].config, size, data, act_size);
            break;

        case DRV_UART_DMA:
            break;

        default:
            rval = ERR_NG;
    }

    return rval;
}

int32_t Stm32f1xx_Uart_IsrHook(Drv_Uart_Channel_e channel, Drv_Uart_Isr_f isr)
{
    if(isr == NULL) {
        return ERR_NG;
    }
    s_uart_isr_map[channel] = isr;
    return ERR_OK;
}

int32_t Stm32f1xx_Uart_Init(Drv_Uart_Config_t *config)
{
    int32_t rval = 0;

    if(config->channel >= DRV_UART_CHANNEL_NUM) {
        return ERR_NG;
    }

    Stm32f1xx_Uart_Gpio_Init(config->channel);
    Stm32f1xx_Uart_Clock_Init(config->channel);

    rval = Stm32f1xx_Uart_Config_Init(config);
    if(rval != ERR_OK) { 
        return rval;
    }

    memcpy((void*)&s_uart_info[config->channel], (void*)config, sizeof(Drv_Uart_Config_t));
    s_uart_info[config->channel].init = 1;

    Common_Fifo_Init(&s_uart_fifo[config->channel][STM32F1XX_UART_DIRECT_TX], config->tx_buffer, config->buf_size);
    Common_Fifo_Init(&s_uart_fifo[config->channel][STM32F1XX_UART_DIRECT_RX], config->rx_buffer, config->buf_size);

    return rval;
}

int32_t Stm32f1xx_Uart_Deinit(Drv_Uart_Channel_e channel)
{
    IRQn_Type irq;

    if(channel >= DRV_UART_CHANNEL_NUM) {
        return ERR_NG;
    }

    if(s_uart_info[channel].init == 0) {
        return ERR_OK;
    }

    if(channel == DRV_UART_CHANNEL_1) {
        irq = USART1_IRQn;
    } else if(channel == DRV_UART_CHANNEL_2) {
        irq = USART2_IRQn;
    } else if(channel == DRV_UART_CHANNEL_3) {
        irq = USART3_IRQn;
    }

    if(s_uart_info[channel].config.mode == DRV_UART_INT) {
        LL_USART_DisableIT_TC(s_uart_channel_instance_map[channel]);
        LL_USART_DisableIT_TXE(s_uart_channel_instance_map[channel]);
        LL_USART_DisableIT_RXNE(s_uart_channel_instance_map[channel]);
        NVIC_DisableIRQ(irq);
    }

    LL_USART_Disable(s_uart_channel_instance_map[channel]);

    Stm32f1xx_Uart_Clock_Deinit(channel);
    memset((void*)&s_uart_info[channel], 0, sizeof(Stm32f1xx_Uart_Info_t));

    return ERR_OK;
}
