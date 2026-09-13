#ifndef _BSP_INTERNAL_FLASH_H
#define _BSP_INTERNAL_FLASH_H

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_flash.h"    //编程与锁控制写入函数
#include "stm32f7xx_hal_flash_ex.h" //擦除与扇区定义

HAL_StatusTypeDef Bsp_Internal_Flash_Erasure(uint32_t addr, uint8_t erase_num);
HAL_StatusTypeDef Bsp_Internal_Flash_Write_4B(uint32_t addr, uint32_t data);
HAL_StatusTypeDef Bsp_Internal_Flash_Write(uint32_t addr, uint32_t *pdata, uint32_t size);
void Bsp_Internal_Flash_Read(uint32_t addr, uint32_t *pbuf, uint32_t size);

#endif
