#include "bsp_at24c02.h"
#include "i2c.h"
#include <stdio.h>
#include "ota.h"

//AT24C02A  256Byte 8Byte每页  一共32页

#define  write_cmd  0xA0
#define  read_cmd   0xA1
#define  page_size  8

uint8_t Bsp_At24c02_IsOnline(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 3, 100) == HAL_OK) {
        return 1;
    } 

    return 0;
}

static void Bsp_At24c02_Write_1Byte(uint16_t addr, uint8_t *data)
{
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
        I2C_MEMADD_SIZE_8BIT, data, 1, 100);    //将数据发送到 EEPROM 的页缓冲区
    if (status != HAL_OK)
        printf("write fail, status=%d\r\n", status);

        // 等待写周期完成
    while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
        ;   //等待内部写周期
   
}

static uint8_t Bsp_At24c02_Read_1Byte(uint16_t addr)
{
    uint8_t ret;
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, read_cmd, addr, 
        I2C_MEMADD_SIZE_8BIT, &ret, 1, 100);

    if (status != HAL_OK)
        printf("read fail, status=%d\r\n", status);
        
    return ret;
}

// void Bsp_At24c02_Write_Page(uint16_t addr, uint8_t *data, uint8_t size)
// {
//     uint8_t page_cnt, page_remainder;
//     page_cnt = size / page_size;    //执行完整页写入的个数
//     page_remainder = size % page_size;  //不足以执行完整页写入的个数
//     HAL_StatusTypeDef status;
//     uint8_t i;

//     //完整页写入
//     for(i = 0; i < page_cnt; i++) {
//         status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//             I2C_MEMADD_SIZE_8BIT, &data[i * page_size], page_size, 100);
//             addr += page_size;
//             printf("addr = %d\r\n", addr);
//             if (status != HAL_OK)
//                 printf("write fail, status=%d\r\n", status);

//             while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                 ;   //等待内部写周期
//     }

//     //剩余写入
//     if (page_remainder > 0) {
//         status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//             I2C_MEMADD_SIZE_8BIT, &data[page_size * page_cnt + 1], page_remainder, 100);
//             if (status != HAL_OK)
//                 printf("write fail, status=%d\r\n", status);

//             while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                 ;   //等待内部写周期
//     }
// }

// void Bsp_At24c02_Write_Page(uint16_t addr, uint8_t *data, uint8_t size)
// {
//     uint8_t scrape_8;
//     HAL_StatusTypeDef status;
//     uint8_t page_cnt, page_remainder;

//     if (size > page_size) {
//         scrape_8 = page_size - (addr % page_size);
//         status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//              I2C_MEMADD_SIZE_8BIT, &data[0], scrape_8, 100);    //内存凑满一页--8个字节
//              addr += scrape_8;
//              while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                  ;   //等待内部写周期
        
//         page_remainder = size - scrape_8;    //计算还要补发的个数;
//         while (page_remainder >= page_size) {   //补发个数大于page_size
//             page_cnt = page_remainder / page_size;  //完整的整页发送
//             for (uint8_t i = 0; i < page_cnt; i++) {
//                 status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//                     I2C_MEMADD_SIZE_8BIT, &data[scrape_8], page_size, 100);
//                     if (status != HAL_OK)   
//                         printf("write fail\r\n");

//                     while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                         ;   //等待内部写周期
//                     addr += page_size;
//                     page_remainder -= page_size;
//             }
//         }
        
//         if (page_remainder < page_size && page_remainder > 0) {
//             status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//                 I2C_MEMADD_SIZE_8BIT, &data[scrape_8 + page_size * page_cnt], page_remainder, 100);
//                 if (status != HAL_OK)   
//                         printf("write fail\r\n");

//                 while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                     ;   //等待内部写周期
//         }
//     } else {
//             uint8_t len = addr % page_size;
//             if (size > len) {
//                 status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//                 I2C_MEMADD_SIZE_8BIT, &data[0], len, 100);
//                 if (status != HAL_OK)   
//                         printf("write fail\r\n");
//                 while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                     ;   //等待内部写周期
//                 addr += len;

//                 status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//                     I2C_MEMADD_SIZE_8BIT, &data[len], size - len, 100);
//                 if (status != HAL_OK)   
//                         printf("write fail\r\n");
//                 while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                     ;   //等待内部写周期
                
//             } else {
//                 status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
//                     I2C_MEMADD_SIZE_8BIT, &data[0], size, 100);
//                 if (status != HAL_OK)   
//                         printf("write fail\r\n");
//                 while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
//                     ;   //等待内部写周期
//             }
//     }
// }

void Bsp_At24c02_Write_Page(uint16_t addr, uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status;
    uint8_t len;

    while (size > 0) {
        // 计算本次能写的最大字节数（不超过当前页剩余空间）
        len = page_size - (addr % page_size);
        if (len > size) {
            len = size;
        }

        // 执行页写入（len 范围 1~8）
        status = HAL_I2C_Mem_Write(&hi2c2, write_cmd, addr, 
            I2C_MEMADD_SIZE_8BIT, data, len, 100);
        if (status != HAL_OK) {
            printf("write fail at addr %d, status=%d\r\n", addr, status);
            return;  // 写入失败则退出
        }

        // 等待内部写周期完成
        while (HAL_I2C_IsDeviceReady(&hi2c2, write_cmd, 10, 100) != HAL_OK)
            ;

        // 更新地址、数据指针和剩余长度
        addr += len;
        data += len;
        size -= len;
    }
}

void Bsp_At24c02_Read_Page(uint16_t addr, uint8_t *data, uint16_t size)
{
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c2, read_cmd, addr, 
        I2C_MEMADD_SIZE_8BIT, data, size, 100);
    if (status != HAL_OK) {
        printf("read fail at addr %d, status=%d\r\n", addr, status);
    }
    
}

void Bsp_At24c02_Set_OTAFlag(void) 
{
    uint32_t flag = OTA_SET_FLAG;   //OTA_SET_FLAG = 0x11223344
    uint8_t data;
    for (uint8_t i = 0; i < 4; i++) {
        data = (flag >> (8 * i)) & 0xFF;
        Bsp_At24c02_Write_1Byte(0x00 + i, &data);   //保存在第一页
    }
}

void Bsp_At24c02_Reset_OTAFlag(void) 
{
    uint8_t data = 0;
    for (uint8_t i = 0; i < 4; i++) {
        Bsp_At24c02_Write_1Byte(0x00 + i, &data);
    }
}

uint32_t Bsp_At24c02_Read_OTAFlag(void) 
{
    uint32_t OTAFlag = 0;
    uint8_t data;

    for(uint8_t i = 0; i < 4; i++) {
        data = Bsp_At24c02_Read_1Byte(0x03 - i);  //每次读取一个字节
        OTAFlag = (OTAFlag << 8) | data;  // 左移8位腾出空间，再装入新字节
    }
    
    return OTAFlag;
}
