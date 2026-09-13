#ifndef _OTA_H
#define _OTA_H

#include "main.h"
#include "usart.h"

#define FLASH_START         0x08000000
#define FLASH_SIZE          0x00100000   // 1MB

#define BOOT_SIZE           0x00008000   // 32KB（一个扇区）
#define BOOT_START          FLASH_START
#define BOOT_END            (BOOT_START + BOOT_SIZE - 1)

#define APP_START           (BOOT_END + 1)
#define APP_SIZE            (FLASH_SIZE - BOOT_SIZE)  // 剩余全部 992KB
#define APP_END             (APP_START + APP_SIZE - 1)

#define OTA_SET_FLAG        0x11223344
#define IAR_XMODEC_FLAG     0X00000001
#define IAR_XMODED_FLAG     0X00000002
#define OTA_SET_VER_FLASG   0X00000004
#define IAR_CMA5_FLAG       0X00000008

#define OTA_VER_NUM_SIZE    26

typedef struct{
    uint32_t OTA_FLAG;
    uint8_t OAT_VER[OTA_VER_NUM_SIZE];
}OTAInfoHandle_t;

typedef struct{
    uint8_t Upadtabuf[BUFSIZE]; //1K
    uint8_t UpdataNB;
    uint32_t XmodemNB;
    uint32_t XmodemTime;
    uint16_t XmodemCRC;
}UpdataAHandle_t;

extern OTAInfoHandle_t OTAInfo;
extern UpdataAHandle_t UpdataA_CB;

#endif
