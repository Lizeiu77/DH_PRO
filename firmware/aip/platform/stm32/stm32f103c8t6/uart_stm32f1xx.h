#ifndef __UART_STM32F1XX_H__
#define __UART_STM32F1XX_H__

#include <stdint.h>

#define DRV_UART_ERR_FRAME_ERR_01           (0x01)
#define DRV_UART_ERR_NOISE_ERR_02           (0x02)
#define DRV_UART_ERR_OVERRUN_ERR_04         (0x04)
#define DRV_UART_ERR_PARITY_ERR_08          (0x08)
#define DRV_UART_ERR_FIFO_FULL_100          (0x100)
#define DRV_UART_ERR_TRANSMIT_COMPLETE_1000 (0x1000)

typedef enum _Drv_Uart_Channel_s
{
    DRV_UART_CHANNEL_1 = 0U,
    DRV_UART_CHANNEL_2,
    DRV_UART_CHANNEL_3,
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
    DRV_UART_DATA_STOP_1_5BIT = 0U,
    DRV_UART_DATA_STOP_2BIT = 0U,
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
