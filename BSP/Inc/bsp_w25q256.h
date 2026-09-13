#ifndef _BSP_W25Q256_H
#define _BSP_W25Q256_H

#include "main.h"
#include "gpio.h"

#define SPI1_CS_Port      GPIOA
#define SPI1_SCK_Port     GPIOA
#define SPI1_SO_Port      GPIOA
#define SPI1_SI_Port      GPIOA

#define SPI1_CS_PIN      GPIO_PIN_4
#define SPI1_SCK_PIN     GPIO_PIN_5
#define SPI1_SO_PIN      GPIO_PIN_6
#define SPI1_SI_PIN      GPIO_PIN_7

#define CS_LOW   HAL_GPIO_WritePin(SPI1_CS_Port, SPI1_CS_PIN, GPIO_PIN_RESET)
#define CS_HIGH  HAL_GPIO_WritePin(SPI1_CS_Port, SPI1_CS_PIN, GPIO_PIN_SET)

#define InterFlash_ADDR  0X00000000
#define BLOCK0_SIZE      0X00100000
#define BLOCK0_END          (InterFlash_ADDR + BLOCK0_SIZE - 1)
#define BACKUP_BLOCK_START  (BLOCK0_END + 1)
//896KB
#define BACKUP_BLOCK_SIZE       0x000E0000         

void Bsp_W25q256_Write_Page(uint32_t addr, uint8_t *pdata, uint16_t len);
void Bsp_W25q256_Read_page(uint32_t addr, uint8_t *pdata ,uint16_t len);
uint8_t Bsp_W25q256_Erasure_Sector(uint32_t addr);
void Bsp_W25q64_ReadId(void);
void Bsp_W25q256_Erasure_Sector_Multi(uint32_t addr, uint8_t cnt);

#endif
