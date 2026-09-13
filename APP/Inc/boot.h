#ifndef _BOOT_H
#define _BOOT_H

#include "main.h"
#include "ota.h"

typedef void (*load_a)(void);   //º¯ÊýÖ¸Õë

void BootLoader_Branch(void);
void MSR_SP(uint32_t addr);
void LOAD_A(uint32_t addr);
void BootLoader_Clear_Register(void);
static void BootLoader_CMD_SHOW(void);
static uint8_t BootLoader_Enter_CMD(uint8_t Tinmeout);
void BootLoader_CMD(void);
uint16_t BootLoade_CRC16_XMODE(uint8_t *data, uint16_t datalen);

#endif
