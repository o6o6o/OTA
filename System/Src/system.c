#include "system.h"

#define DWT_LSR_Present_Msk (1UL << 0)

void DWT_Delay_Init(void)
{
    // 1. 使能跟踪与调试模块 (CoreDebug DEMCR TRCENA 位)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // 2. 对于 Cortex-M7，解锁 DWT 寄存器访问
    //    先检查 LAR 寄存器是否存在 (通过 DWT->LSR 的 Present 位)
    if (DWT->LSR & DWT_LSR_Present_Msk) {
        DWT->LAR = 0xC5ACCE55; // 写入解锁密钥
    }

    // 3. 复位周期计数器
    DWT->CYCCNT = 0;

    // 4. 使能周期计数器 (DWT CTRL CYCCNTENA 位)
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  微秒级延时
  * @param  us 要延时的微秒数
  */
void delay_us(uint32_t us)
{
    // 读取当前计数值
    uint32_t start = DWT->CYCCNT;
    // 计算目标周期数（SystemCoreClock 一般是 72MHz / 168MHz / etc）
    uint32_t ticks = us * (SystemCoreClock / 1000000);

    // 循环等待直到超过目标周期数
    while ((DWT->CYCCNT - start) < ticks) {
        // 空转
    }
}
