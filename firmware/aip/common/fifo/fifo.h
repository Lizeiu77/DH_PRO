#ifndef __COMMON_FIFO_H__
#define __COMMON_FIFO_H__

#include <stdint.h>

typedef struct _Common_Fifo_s
{
    uint32_t size;
    uint32_t w_index;
    uint32_t r_index;
    uint32_t full_flag;
    void *ringbuf;
}Common_Fifo_t;

int32_t Common_Fifo_Init(Common_Fifo_t *fifo, void *buffer, uint32_t size);
uint32_t Common_Fifo_Free(Common_Fifo_t *fifo);
int32_t Common_Fifo_Write(Common_Fifo_t *fifo, void *buf, uint32_t size, uint32_t *act_size);
int32_t Common_Fifo_Read(Common_Fifo_t *fifo, void *buf, uint32_t size, uint32_t *act_size);

#endif
