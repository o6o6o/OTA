#ifndef _BSP_AT24C02_H
#define _BSP_AT24C02_H

#include "main.h"

#define PAGE_1

uint8_t Bsp_At24c02_IsOnline(void);
static void Bsp_At24c02_Write_1Byte(uint16_t addr, uint8_t *data);
static uint8_t Bsp_At24c02_Read_1Byte(uint16_t addr);
void Bsp_At24c02_Write_Page(uint16_t addr, uint8_t *data, uint16_t size);
void Bsp_At24c02_Read_Page(uint16_t addr, uint8_t *data, uint16_t size);
uint32_t Bsp_At24c02_Read_OTAFlag(void);
void Bsp_At24c02_Set_OTAFlag(void);
void Bsp_At24c02_Reset_OTAFlag(void);
#endif