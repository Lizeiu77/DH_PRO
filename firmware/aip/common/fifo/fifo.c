#include "fifo.h"

#include "data_type.h"
#include <stdint.h>
#include <string.h>

int32_t Common_Fifo_Init(Common_Fifo_t *fifo, void *buffer, uint32_t size)
{
    if(fifo == NULL || buffer == NULL || size <= 0) {
        return ERR_NG;
    }

    memset((void*)fifo, 0, sizeof(Common_Fifo_t));

    fifo->size = size;
    fifo->ringbuf = buffer;
    return ERR_OK;
}

uint8_t Common_Fifo_IsFull(Common_Fifo_t *fifo)
{
    return fifo->w_index == fifo->r_index;
}

uint8_t Common_Fifo_IsEmpty(Common_Fifo_t *fifo)
{
    return (fifo->r_index == fifo->w_index) && (fifo->full_flag == 0);
}

uint32_t Common_Fifo_Free(Common_Fifo_t *fifo)
{
    int32_t free = 0;

    if(fifo->full_flag == 1)
    {
        return free;
    }

    free = fifo->r_index - fifo->w_index;

    if(free <= 0)
    {
        free = -free;
        return (fifo->size - free);
    }

    return free;
}

int32_t Common_Fifo_Write(Common_Fifo_t *fifo, void *buf, uint32_t size, uint32_t *act_size)
{
    uint32_t free = 0;
    uint32_t right_instance = 0;
    uint32_t remain = 0;

    if(fifo->ringbuf == NULL) {
        return ERR_NG;
    }

    if(fifo->full_flag == 1) {
        return ERR_NG;
    }

    free = Common_Fifo_Free(fifo);
    if(free < size)
    {
        size = free;
    }

    //instance of right edge
    right_instance = fifo->size - fifo->w_index;

    if(right_instance >= size)
    {
        memcpy((void*)((uint8_t*)fifo->ringbuf + fifo->w_index), buf, size);
    }
    else
    {
        remain = size - right_instance;
        memcpy((void*)((uint8_t*)fifo->ringbuf + fifo->w_index), buf, right_instance);
        memcpy(fifo->ringbuf, (void*)((uint8_t*)buf + right_instance), remain);
    }
    fifo->w_index += size;
    fifo->w_index %= fifo->size;
    if(act_size != NULL)
    {
        *act_size = size;
    }

    if(Common_Fifo_IsFull(fifo))
    {
        fifo->full_flag = 1;
    }

	return ERR_OK;
}

int32_t Common_Fifo_Read(Common_Fifo_t *fifo, void *buf, uint32_t size, uint32_t *act_size)
{
    uint32_t free = 0;
    uint32_t right_instance = 0;
    uint32_t remain = 0;

    if(fifo->ringbuf == NULL) {
        return ERR_NG;
    }

    if(Common_Fifo_IsEmpty(fifo))
    {
        return ERR_NG;
    }

    free = fifo->size - Common_Fifo_Free(fifo);
    if(free < size)
    {
        size = free;
    }

    right_instance = fifo->size - fifo->r_index;

    if(right_instance >= size)
    {
        memcpy(buf, (void*)((uint8_t*)fifo->ringbuf + fifo->r_index), size);
    }
    else
    {
        remain = size - right_instance;
        memcpy(buf, (void*)((uint8_t*)fifo->ringbuf + fifo->r_index), right_instance);
        memcpy((void*)((uint8_t*)buf + right_instance), fifo->ringbuf, remain);
    }
    fifo->r_index += size;
    fifo->r_index %= fifo->size;
    fifo->full_flag = 0;
    if(act_size != NULL)
    {
        *act_size = size;
    }

    return ERR_OK;
}
