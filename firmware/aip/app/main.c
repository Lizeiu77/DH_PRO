#include <stdint.h>
#include "data_type.h"

int32_t main(void)
{
	Drv_Platform_System_Init();
	Drv_Platform_Clock_Init(48000000);
	while(1);
}
