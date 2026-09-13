#include "bsp_internal_flash.h"
#include <stdio.h>

FLASH_EraseInitTypeDef EraseInitStruct;

HAL_StatusTypeDef Bsp_Internal_Flash_Erasure(uint32_t addr, uint8_t erase_num) {

    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;

    HAL_FLASH_Unlock();  // 必须解锁

    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.Sector = addr;
    EraseInitStruct.NbSectors = erase_num;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

    HAL_FLASH_Lock();  //操作完成后上锁

    if (status != HAL_OK) {
        // 擦除失败，SectorError 指示出错位置
        printf("Erase failed at sector %lu\r\n", SectorError);
    }

    HAL_FLASH_Lock();    //上锁

    return status;
}

HAL_StatusTypeDef Bsp_Internal_Flash_Write_4B(uint32_t addr, uint32_t data) 
{
    HAL_FLASH_Unlock();  //必须解锁

    HAL_StatusTypeDef status;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, data);

    if (status != HAL_OK) {
        // 擦除失败
        printf("Erase failed\r\n");
    }

    HAL_FLASH_Lock();    //上锁

    return status;
}

HAL_StatusTypeDef Bsp_Internal_Flash_Write(uint32_t addr, uint32_t *pdata, uint32_t size) 
{
    HAL_FLASH_Unlock();  //必须解锁
    HAL_StatusTypeDef status;

    for (uint32_t i = 0; i < size; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *pdata);   //每次写入4字节，调用的时候要注意处理字节长度
        addr += 4;
        pdata += 1;

        if (status != HAL_OK) {
            printf("write failed\r\n");
        }
    }

    HAL_FLASH_Lock();    //上锁

    return status;
}

void Bsp_Internal_Flash_Read(uint32_t addr, uint32_t *pbuf, uint32_t size) 
{
    //uint32_t *pdata = &addr;这样是错误的因为addr是函数形参，存储在栈上。&addr获取的是栈地址
    uint32_t *pdata = (uint32_t *)addr;  // 指向 Flash 地址
    for (uint32_t i = 0; i < size; i++) {
        pbuf[i] = pdata[i]; 
    }
}
