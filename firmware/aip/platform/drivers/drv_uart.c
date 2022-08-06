#include "drv_uart.h"

#include "data_type.h"

__ATTRIBUTE_WEAK__ int32_t Drv_Uart_Send(Drv_Uart_Channel_e channel, uint8_t *data, uint32_t size, uint32_t *act_size)
{
    return ERR_NG;
}

__ATTRIBUTE_WEAK__ int32_t Drv_Uart_Recv(Drv_Uart_Channel_e channel, uint32_t size, uint8_t *data, uint32_t *act_size)
{
    return ERR_NG;
}

__ATTRIBUTE_WEAK__ int32_t Drv_Uart_IsrHook(Drv_Uart_Channel_e channel, Drv_Uart_Isr_f isr)
{
    return ERR_NG;
}

__ATTRIBUTE_WEAK__ int32_t Drv_Uart_Init(Drv_Uart_Config_t *config)
{
    return ERR_NG;
}

__ATTRIBUTE_WEAK__ int32_t Drv_Uart_Deinit(Drv_Uart_Channel_e channel)
{
    return ERR_NG;
}
