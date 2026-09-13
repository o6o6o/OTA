#include "circular_buf.h"

/**
 * 初始化环形缓冲区
 */
void Circular_Buf_Init(circular_buf_t *cir_buf, uint8_t *buffer, uint16_t size)
{
    cir_buf->head = 0;
    cir_buf->tail = 0;
    cir_buf->buf = buffer;
    cir_buf->cir_buf_size = size;

}

/**
 * 判断缓冲区是否满
 */
static uint8_t Circular_Buf_IsFull(circular_buf_t *cir_buf)
{
    if((cir_buf->head + 1) % cir_buf->cir_buf_size == cir_buf->tail) 
        return 1;

    return 0;
}

/**
 * 判断缓冲区是否为空
 */
static uint8_t Circular_Buf_IsEmpty(circular_buf_t *cir_buf)
{
    if(cir_buf->head == cir_buf->tail)
        return 1;
    
    return 0;
}

/**
 * 写入1字节数据到环形缓冲区;
 */
uint8_t Circular_Buf_Put_1B(circular_buf_t *cir_buf, uint8_t data)
{
    if(Circular_Buf_IsFull(cir_buf)) return 0;

    cir_buf->buf[cir_buf->head] = data; //写入1字节数据
    cir_buf->head = (cir_buf->head + 1) % cir_buf->cir_buf_size;    //写指针加一

    return 1;
}

/**
 * 向缓冲区存入数据(操作head指针)
 */
uint8_t Circular_Buf_Put(circular_buf_t *cir_buf, uint8_t *data, uint16_t size)
{
    uint8_t full_flag = 0;
    for(uint16_t i = 0; i < size; i++) {
        full_flag = Circular_Buf_IsFull(cir_buf);   //判断缓冲区是否已满
        if(full_flag) return 0;    //若缓冲区已满还写入数据则旧的还没有处理的数据会被覆盖

        cir_buf->buf[cir_buf->head] = data[i];  //一个字节一个字节写入数据
        cir_buf->head =  (cir_buf->head + 1) % cir_buf->cir_buf_size;   //写指针加一
    }

    return 1;
}

/**
 * 向缓冲区读取数据(操作tail指针)
 */
void Circular_Buf_Take(circular_buf_t *cir_buf, uint8_t *buf)
{
    uint16_t count = 0;
    //如果环形缓冲区不为空就一直读取数据
    while(!Circular_Buf_IsEmpty(cir_buf)) {
        buf[count++] = cir_buf->buf[cir_buf->tail];
        cir_buf->tail = (cir_buf->tail + 1) % cir_buf->cir_buf_size;    //读指针加一
    }
}
