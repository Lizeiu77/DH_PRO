#ifndef __DRV_UART_H__
#define __DRV_UART_H__

#include <stdint.h>

#ifdef PLATFORM_STM32F103X
    #include "uart_stm32f1xx.h"
#else
typedef enum _Drv_Uart_Channel_s
{
    DRV_UART_CHANNEL_0 = 0U,
    DRV_UART_CHANNEL_1,
    DRV_UART_CHANNEL_2,
    DRV_UART_CHANNEL_NUM,
}Drv_Uart_Channel_e;

typedef enum _Drv_Uart_Baud_s
{
    DRV_UART_BAUD_9600 = 9600U,
    DRV_UART_BAUD_115200 = 115200U,
    DRV_UART_BAUD_NUM,
}Drv_Uart_Baud_e;

typedef enum _Drv_Uart_DataWidth_s
{
    DRV_UART_DATA_WIDTH_8Bit = 0U,
    DRV_UART_DATA_WIDTH_9Bit,
    DRV_UART_DATA_WIDTH_NUM,
}Drv_Uart_DataWidth_e;

typedef enum _Drv_Uart_StopBit_s
{
    DRV_UART_DATA_STOP_1BIT = 0U,
    DRV_UART_DATA_STOP_1_5BIT,
    DRV_UART_DATA_STOP_2BIT,
}Drv_Uart_StopBit_e;

typedef enum _Drv_Uart_HardwareFlowCtrl_s 
{
    DRV_UART_HARDWARE_FLOWCTRL_NONE = 0U,
    DRV_UART_HARDWARE_FLOWCTRL_RTS,
    DRV_UART_HARDWARE_FLOWCTRL_CTS,
    DRV_UART_HARDWARE_FLOWCTRL_RTS_CTS,
    DRV_UART_HARDWARE_FLOWCTRL_NUM,
}Drv_Uart_HardwareFlowCtrl_e;

typedef enum _Drv_Uart_Mode_s
{
    DRV_UART_NORMAL = 0U,
    DRV_UART_INT,
    DRV_UART_DMA,
}Drv_Uart_Mode_e;

typedef struct _Drv_Uart_Cb_Context_s
{
    uint32_t err_status;
}Drv_Uart_Cb_Context_s;

#endif

typedef struct _Drv_Uart_Config_s
{
    Drv_Uart_Channel_e channel;
    Drv_Uart_Baud_e baud;
    Drv_Uart_DataWidth_e data_width;
    Drv_Uart_StopBit_e stop_bit;
    Drv_Uart_HardwareFlowCtrl_e hw_flow;
    Drv_Uart_Mode_e mode;
    uint8_t *tx_buffer;
    uint8_t *rx_buffer;
    uint8_t buf_size;
    uint32_t timeout;
}Drv_Uart_Config_t;

typedef void(*Drv_Uart_Isr_f)(void *p_arg);

int32_t Drv_Uart_Send(Drv_Uart_Channel_e channel, uint8_t *data, uint32_t size, uint32_t *act_size);
int32_t Drv_Uart_Recv(Drv_Uart_Channel_e channel, uint32_t size, uint8_t *data, uint32_t *act_size);
int32_t Drv_Uart_IsrHook(Drv_Uart_Channel_e channel, Drv_Uart_Isr_f isr);
int32_t Drv_Uart_Init(Drv_Uart_Config_t *config);
int32_t Drv_Uart_Deinit(Drv_Uart_Channel_e channel);

#endif
