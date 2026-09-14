#include "bsp_w25q256.h"
#include "spi.h"
#include <string.h>
#include <stdio.h>
#include "system.h"

//64M--8192K 8M 32768page 1page = 256Byte 4page=1K

/**
 * @brief  页写入函数（写入长度 1~256 字节，不能跨页）
 * @param  addr  起始地址（24位，最大 0x7FFFFF）
 * @param  pdata 数据指针
 * @param  len   数据长度（1~256）
 */
void Bsp_W25q256_Write_Page(uint32_t addr, uint8_t *pdata, uint16_t len)
{

    if (len == 0 || len > 256 || addr + len > 0x7FFFFF) return; // 参数检查：长度非0且不超过256，地址不越界
 
    if ((addr & 0xFF) + len > 256) { // 检查是否跨页（页内偏移 + 长度 > 256 则跨页）
        printf("Error: Cross-page write, split your data!\r\n"); // 跨页则报错并退出
        return;
    }

    uint8_t write_enable = 0x06; // 写使能命令
    CS_LOW;                      // 拉低片选
    delay_us(1);                 // 短延时
    HAL_SPI_Transmit(&hspi1, &write_enable, 1, 100); // 发送写使能命令
    delay_us(1);                 // 短延时
    CS_HIGH;                     // 拉高片选，结束写使能
    HAL_Delay(1);                // 等待芯片内部处理

    uint8_t cmd[4] = {0x02, (addr>>16)&0xFF, (addr>>8)&0xFF, addr&0xFF}; // 页编程命令 0x02 + 3字节地址
    CS_LOW;                      // 拉低片选
    delay_us(1);                 // 短延时
    HAL_SPI_Transmit(&hspi1, cmd, 4, 200);          // 发送命令和地址
    HAL_SPI_Transmit(&hspi1, pdata, len, 200);      // 发送数据
    delay_us(1);                 // 短延时
    CS_HIGH;                     // 拉高片选，启动内部写入
    delay_us(1);                 // 短延时

   
    uint8_t tx_status[2] = {0x05, 0xFF}; // 读状态寄存器命令 0x05 + 哑字节
    uint8_t rx_status[2] = {0};          // 接收缓冲区
    uint32_t timeout = 10000;            // 超时计数（约10ms，因为每次循环有延时）
    do {                                 // 轮询等待写入完成
        CS_LOW;                          // 拉低片选
        delay_us(1);                     // 短延时
        HAL_SPI_TransmitReceive(&hspi1, tx_status, rx_status, 2, 100); // 读取状态寄存器
        delay_us(1);                     // 短延时
        CS_HIGH;                         // 拉高片选
        delay_us(1);                     // 短延时
        timeout--;                       // 超时递减
        if (timeout == 0) {              // 超时处理
            printf("Error: Write page timeout at addr 0x%06X\r\n", addr); // 打印超时信息
            return;
        }
    } while (rx_status[1] & 0x01);       // 检查 BUSY 位（bit0），为1则继续等待
}

/**
 * @brief  页读取函数（读取任意长度，不受页限制）
 * @param  addr  起始地址
 * @param  pdata 接收数据缓冲区
 * @param  len   读取长度
 */
void Bsp_W25q256_Read_page(uint32_t addr, uint8_t *pdata, uint16_t len)
{
    uint8_t cmd[4] = {0x03, (addr>>16)&0xFF, (addr>>8)&0xFF, addr&0xFF}; // 读数据命令 0x03 + 3字节地址
    CS_LOW;    //拉低片选
    delay_us(1);                             // 短延时
    HAL_SPI_Transmit(&hspi1, cmd, 4, 200);   // 发送命令和地址
    HAL_SPI_Receive(&hspi1, pdata, len, 200); // 接收数据
    delay_us(1);                             // 短延时
    CS_HIGH;                                 // 拉高片选
    delay_us(1);                             // 短延时
}


/**
 * @brief  扇区擦除(实际使用 0xD8 块擦除，擦除 64KB  0x20 块擦除， 擦除4KB)
 * @param  addr  擦除起始地址
 * @retval 1 成功，0 失败
 */
uint8_t Bsp_W25q256_Erasure_Sector(uint32_t addr)
{
   
    uint8_t wren = 0x06; // 写使能命令
    CS_LOW;              // 拉低片选
    delay_us(1);         // 短延时
    if (HAL_SPI_Transmit(&hspi1, &wren, 1, 100) != HAL_OK) { // 发送写使能，失败则退出
        CS_HIGH;         // 拉高片选
        delay_us(1);     // 短延时
        return 0;
    }
    CS_HIGH;             // 拉高片选，结束写使能
    HAL_Delay(1);        // 等待芯片内部处理

    uint8_t cmd[4] = {0x20, (addr>>16)&0xFF, (addr>>8)&0xFF, addr&0xFF}; // 块擦除命令 0x20 + 3字节地址
    CS_LOW;              // 拉低片选
    delay_us(1);         // 短延时
    if (HAL_SPI_Transmit(&hspi1, cmd, 4, 100) != HAL_OK) { // 发送擦除命令，失败则退出
        CS_HIGH;         // 拉高片选
        delay_us(1);     // 短延时
        return 0;
    }
    CS_HIGH;             // 拉高片选，启动擦除
    delay_us(1);         // 短延时

    
    uint8_t tx_status[2] = {0x05, 0xFF}; // 读状态寄存器命令
    uint8_t rx_status[2] = {0};          // 接收缓冲区
    uint32_t timeout = 20000;            // 超时计数（约20ms）
    do {                                 // 轮询等待擦除完成
        CS_LOW;                          // 拉低片选
        delay_us(1);                     // 短延时
        HAL_SPI_TransmitReceive(&hspi1, tx_status, rx_status, 2, 100); // 读取状态寄存器
        delay_us(1);                     // 短延时
        CS_HIGH;                         // 拉高片选
        HAL_Delay(1);                    // 较长延时
        timeout--;                       // 超时递减
        if (timeout == 0) {              // 超时处理
            printf("Sector erase timeout at addr 0x%06X\r\n", addr); // 打印超时信息
            return 0;
        }
    } while (rx_status[1] & 0x01);       // 检查 BUSY 位

    return 1; // 成功
}

/**
 * @brief  连续擦除多个扇区（每次擦除 64KB 块，地址间隔 0x1000 实际是 4KB，调用时注意）
 * @param  addr  起始地址
 * @param  cnt   擦除次数
 */
void Bsp_W25q256_Erasure_Sector_Multi(uint32_t addr, uint8_t cnt) {
    for (uint8_t i = 0; i < cnt; i++) { // 循环擦除
        Bsp_W25q256_Erasure_Sector(addr + (i * 0x1000)); // 每次地址增加 4KB
    }
}

/**
 * @brief  读取 JEDEC ID（制造商 ID、存储类型、容量）
 */
void Bsp_W25q64_ReadId(void)
{
    uint8_t tx_data[4] = {0x9F, 0xFF, 0xFF, 0xFF}; // 读 ID 命令 0x9F + 3 个哑字节
    uint8_t rx_data[4] = {0};                      // 接收缓冲区

    CS_HIGH;              // 先拉高片选
    delay_us(2);          // 短延时
    CS_LOW;               // 拉低片选                        
    delay_us(2);          // 短延时                   
    HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, 4, 100); // 发送命令并接收 4 字节
    delay_us(2);          // 短延时
    CS_HIGH;              // 拉高片选                       
    delay_us(2);          // 短延时

    printf("JEDEC ID: 0x%02X, 0x%02X, 0x%02X\r\n", // 打印 ID（有效字节为 rx_data[1]~[3]）
           rx_data[1], rx_data[2], rx_data[3]);
}
