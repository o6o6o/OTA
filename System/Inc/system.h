#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "stm32f767xx.h"   // 先包含设备头文件，定义 IRQn_Type、__NVIC_PRIO_BITS 等
#include "core_cm7.h"      // 再包含内核头文件

void DWT_Delay_Init(void);
void delay_us(uint32_t us);

#endif
