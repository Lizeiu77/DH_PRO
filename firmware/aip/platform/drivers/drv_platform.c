#include "drv_platform.h"

#include "data_type.h"

__ATTRIBUTE_WEAK__ int32_t Drv_Platform_Clock_Init(uint32_t freq)
{
    return ERR_NG;
}

__ATTRIBUTE_WEAK__ int32_t Drv_Platform_System_Init(void)
{
    return ERR_NG;
}
