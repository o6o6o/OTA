#ifndef _CIRCULAR_BUF_
#define _CIRCULAR_BUF_

#include "main.h"

typedef struct {
    uint16_t head;
    uint16_t tail;
    uint8_t *buf;
    uint16_t cir_buf_size;
}circular_buf_t;


void Circular_Buf_Init(circular_buf_t *cir_buf, uint8_t *buffer, uint16_t size);
static uint8_t Circular_Buf_IsFull(circular_buf_t *cir_buf);
static uint8_t Circular_Buf_IsEmpty(circular_buf_t *cir_buf);
uint8_t Circular_Buf_Put_1B(circular_buf_t *cir_buf, uint8_t data);
uint8_t Circular_Buf_Put(circular_buf_t *cir_buf, uint8_t *data, uint16_t size);
void Circular_Buf_Take(circular_buf_t *cir_buf, uint8_t *buf);

#endif
