#include "boot.h"
#include <stdio.h>
#include "usart.h"
#include "i2c.h"
#include "spi.h"
#include "ota.h"
#include <stdio.h>
#include <string.h>
#include "bsp_internal_flash.h"
#include "bsp_at24c02.h"
#include "bsp_w25q256.h"

#define SRAM_MEMORY_STARTADDR   0X20000000
#define SRAM_MEMORY_ENDADDR     0X2007FFFF
#define RX_Frame_Size      133
#define Valid_Data         128  

load_a load_A;
uint8_t Enter_CMD_FLAG = 0;

extern OTAInfoHandle_t OTAInfo;
extern UpdataAHandle_t UpdataA_CB;
extern circular_buf_t *Rx_Cir_BufHandle;  //串口接收环形缓冲区句柄
extern uint8_t volatile cmd_flag;
extern uint16_t volatile rx_len;
extern uint32_t BootStaFlag;
extern volatile uint32_t idle_cnt;

static uint8_t Cmd_Buf[8];  //接收指令数组
static uint8_t RX_UpdataABuf[RX_Frame_Size + 1];    //接收更新A区数据数组
static uint8_t RX_OTAVerBuf[OTA_VER_NUM_SIZE + 1];

void BootLoader_Branch(void) 
{
    if (!BootLoader_Enter_CMD(2)) {
        if(OTAInfo.OTA_FLAG == OTA_SET_FLAG) {
            printf("ota updata\r\n");
        } else {
            printf("jump to A section\r\n");
            LOAD_A(APP_START);
        }
    } else {
        Enter_CMD_FLAG = 1;
        BootLoader_CMD();   //进入BootLoader串口命令行
    }    
}

/**
 * 将栈顶指针MSP指向a区的起始地址
 * __attribute__((naked)) 告诉编译器不生成任何压栈（PUSH）和出栈（POP）代码
 * 遵循 AAPCS 调用标准，第一个参数 addr 由调用者放入 r0 寄存器。
 * 由于是 naked 函数，编译器不会将 r0 保存到栈上。
 */
__attribute__((naked)) void MSR_SP(uint32_t addr) {
    __asm volatile (
        "MSR MSP, r0\n"
        "BX LR\n"
    );
}

//void LOAD_A(uint32_t addr)
//{
//    //判断栈顶指针是否在SRAM的地址范围里
//    if((*(uint32_t *)addr >= SRAM_MEMORY_STARTADDR) && (*(uint32_t *)addr <= SRAM_MEMORY_ENDADDR)) {
//        MSR_SP(*(uint32_t *)addr);  //将栈顶指针SP指向A区起始地址0x08008000
//        load_A = (load_a)*(uint32_t *)(addr + 4);   //让load_A指向复位向量 
//        load_A();//将PC指针指向load_A间接指向复位向量 
//    }
//}

//void BootLoader_Clear_Register(void) 
//{
//    //复位GPIO
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);

//    HAL_UART_DMAStop(&huart1);//关闭DMA

//    //复位USART1
//    HAL_UART_DeInit(&huart1);

//    //复位I2C2
//    HAL_I2C_DeInit(&hi2c2);

//    //复位SPI1
//    HAL_SPI_DeInit(&hspi1);
//	
//	__disable_irq();              // 关闭所有中断，之后在A区要重新打开

//    HAL_SuspendTick();    //关闭SystemTick
//}

void LOAD_A(uint32_t addr)
{
    uint32_t stack_ptr = *(uint32_t *)addr;
    uint32_t reset_handler = *(uint32_t *)(addr + 4);

    if ((stack_ptr >= SRAM_MEMORY_STARTADDR) && (stack_ptr <= SRAM_MEMORY_ENDADDR)
        && (reset_handler != 0xFFFFFFFF) && (reset_handler != 0x00000000))
    {
        BootLoader_Clear_Register();          //复位用到的外设和寄存器
        MSR_SP(stack_ptr);                    //设置MSP
        load_A = (load_a)reset_handler;       //跳转到复位向量
        load_A();
    } else {
        printf("Failed to jump to Area A\r\n");
        BootLoader_CMD_SHOW();
        BootLoader_CMD();    //跳转A区失败自动进入BootLoader串口命令模式
    }
}

void BootLoader_Clear_Register(void) 
{
    //关闭全局中断，防止清理过程中被中断打断
    __disable_irq();

    //关闭 SysTick（避免跳转后SysTick中断触发）
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    //停止DMA传输
    HAL_UART_DMAStop(&huart1);

    //复位已初始化的外设
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_1);
    HAL_UART_DeInit(&huart1);
    HAL_I2C_DeInit(&hi2c2);
    HAL_SPI_DeInit(&hspi1);

    //清除所有外设中断的使能和挂起标志
    NVIC_DisableIRQ(USART1_IRQn);
    NVIC_ClearPendingIRQ(USART1_IRQn);
    
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_IDLE);
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_TC);
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_RXNE);
    // NVIC_DisableIRQ(I2C2_EV_IRQn);
    // NVIC_ClearPendingIRQ(I2C2_EV_IRQn);
    // NVIC_DisableIRQ(I2C2_ER_IRQn);
    // NVIC_ClearPendingIRQ(I2C2_ER_IRQn);
    // NVIC_DisableIRQ(SPI1_IRQn);
    // NVIC_ClearPendingIRQ(SPI1_IRQn);

    //清除DMA 中断
    NVIC_DisableIRQ(DMA2_Stream2_IRQn);
    NVIC_ClearPendingIRQ(DMA2_Stream2_IRQn);

    //设置中断向量表偏移到应用程序起始地址
    SCB->VTOR = APP_START;
}

static uint8_t BootLoader_Enter_CMD(uint8_t Tinmeout) 
{
    
    uint8_t TIME;
    printf("Enter 'w' within %ds to access the BootLoader serial command mode\r\n", Tinmeout);
    TIME = Tinmeout * 10;
    while (TIME) {
        if (cmd_flag) {
            Circular_Buf_Take(Rx_Cir_BufHandle, Cmd_Buf);
            cmd_flag = 0;
            if (!strcmp(Cmd_Buf, "w")) 
                return 1;
        }

        TIME --;
        HAL_Delay(100);
    }

    return 0;
}

static void BootLoader_CMD_SHOW(void) {
    printf("\r\n");
    printf("[1]Erase Area A\r\n");
    printf("[2]Serial port IAR download for Area A program\r\n");
    printf("[3]Set OTA version number\r\n");
    printf("[4]Query OTA version number\r\n");
    printf("[5]Download the program from the external Flash\r\n");
    printf("[6]Use the external Flash internal program\r\n");
    printf("[7]Restart\r\n");
}

void BootLoader_CMD(void) 
{
    static HAL_StatusTypeDef status;
    
    if (Enter_CMD_FLAG) {
        BootLoader_CMD_SHOW();
        Enter_CMD_FLAG = 0;
    }

    if (cmd_flag) {
        cmd_flag = 0;
        if (!BootStaFlag) {
            Circular_Buf_Take(Rx_Cir_BufHandle, Cmd_Buf);
            if (!strcmp(Cmd_Buf, "1")) {
                printf("Erasing...\r\n");
                status = Bsp_Internal_Flash_Erasure(FLASH_SECTOR_1, 7);  //把A区程序全部擦除
                if (status == HAL_OK) {
                    printf("The program in Area A has been erased\r\n");
                } else {
                    printf("Erasure failed\r\n");
                }
                BootLoader_CMD_SHOW();
            }
            else if (!strcmp(Cmd_Buf, "2")) {
                printf("Download the program for Area A via serial port IAP using Xmodem protocol,please use a bin format file.\r\n");
                printf("Erasing...\r\n");
                status = Bsp_Internal_Flash_Erasure(FLASH_SECTOR_1, 7);
                if (status == HAL_OK) {
                    printf("The program in Area A has been erased\r\n");
                } else {
                    printf("Erasure failed\r\n");
                }
                BootStaFlag |= (IAR_XMODEC_FLAG|IAR_XMODED_FLAG);     //将IAR发送字符‘C’标志位置1
                UpdataA_CB.XmodemTime = 0;           //开始计算发送字符‘C’的时间
                UpdataA_CB.XmodemNB = 0;
            }
            else if (!strcmp(Cmd_Buf, "3")) {
                printf("Please enter the OTA version number. The format should be as follows: VER-1.0.0-2026-10-01-12:0\r\n");
                BootStaFlag |= OTA_SET_VER_FLASG;
            }
            else if (!strcmp(Cmd_Buf, "4")) {
                uint8_t buf[OTA_VER_NUM_SIZE + 1] = {0};
                Bsp_At24c02_Read_Page(0x08, buf, OTA_VER_NUM_SIZE);
                buf[OTA_VER_NUM_SIZE] = '\0';
                printf("VER: [%s]\r\n", buf);

                BootLoader_CMD_SHOW();
            }
            else if (!strcmp(Cmd_Buf, "7")) {
                printf("Restart STM32\r\n");
                NVIC_SystemReset();
            }
        }
    
        /**********************启用Xmodem协议进行文件传输*****************************/
        else if (BootStaFlag & IAR_XMODED_FLAG) {
            Circular_Buf_Take(Rx_Cir_BufHandle, RX_UpdataABuf);    //从环形缓冲区拿串口接收到的一帧数据一共133字节,有效数据128字节
            uint8_t Serial_Number = RX_UpdataABuf[1];          //序号码
            uint8_t Complement_Number = RX_UpdataABuf[2];      //序号反码
            uint16_t CRC_Val = 0;
            static uint32_t blk;
            /**********************处理一帧数据帧头SOH(0X04)*****************************/
            if (rx_len == RX_Frame_Size && RX_UpdataABuf[0] == 0x01) {
                BootStaFlag &= ~IAR_XMODEC_FLAG;    // 收到第一包就停止发 'C'
                
                CRC_Val = BootLoade_CRC16_XMODE(&RX_UpdataABuf[3], Valid_Data);
                //判断序号码+序号反码是否等于0xFF
                if ((Serial_Number + Complement_Number) != 0xFF){
                    printf("\x15"); //NACK
                    return ;    //直接返回
                } 

                //进行CRC校验
                if (CRC_Val != (((uint16_t)RX_UpdataABuf[131] << 8) | RX_UpdataABuf[132])) {
                    printf("\x15"); //NACK
                    return ;    //直接返回
                } else {
                    printf("\x06"); //ACK
                }

                //将要更新的数据保存到Upadtabuf数组中凑足1K再写入内部Flash
                blk = UpdataA_CB.XmodemNB / (BUFSIZE / Valid_Data);        //blk--凑足了次满8数据
                UpdataA_CB.XmodemNB++;
                memcpy(&UpdataA_CB.Upadtabuf[((UpdataA_CB.XmodemNB - 1) % (BUFSIZE / Valid_Data)) * Valid_Data], 
                    &RX_UpdataABuf[3], Valid_Data);    //将每次接收的128字节数据保存起来

                //处理凑满8次的数据 1K
                if (UpdataA_CB.XmodemNB % (BUFSIZE / Valid_Data) == 0) {
                        Bsp_Internal_Flash_Write(APP_START + blk * BUFSIZE, 
                            &UpdataA_CB.Upadtabuf, BUFSIZE / 4);
                }
            }   
            /**********************处理EOT(0X04)*****************************/
            else if (rx_len == 1 && RX_UpdataABuf[0] == 0x04) {
                printf("\x06"); //ACK
                BootStaFlag &= ~IAR_XMODED_FLAG;    //清除IAR_XMODED_FLAG标志

                //处理凑不齐8次数据的尾巴
                if (UpdataA_CB.XmodemNB % (BUFSIZE / Valid_Data) != 0) {
                    Bsp_Internal_Flash_Write(APP_START + blk * BUFSIZE, 
                        &UpdataA_CB.Upadtabuf, (UpdataA_CB.XmodemNB % (BUFSIZE / Valid_Data) * Valid_Data) / 4);
                }
                
                HAL_Delay(100);
                printf("Restart STM32\r\n");
                NVIC_SystemReset();     //重启stm32
            }    
        }
        /**********************设置OTA版本号*****************************/
        else if (BootStaFlag & OTA_SET_VER_FLASG) {
            int temp;
            Circular_Buf_Take(Rx_Cir_BufHandle, &RX_OTAVerBuf);     //接收数据
            //判断长度是否正确
            if (rx_len == OTA_VER_NUM_SIZE) {
                BootStaFlag &= ~OTA_SET_VER_FLASG;  //清除标志位
                //用sscanf判断版本号格式是否正确
                if(sscanf((char *)RX_OTAVerBuf, "VER-%d.%d.%d-%d-%d-%d-%d:%d", 
                    &temp, &temp, &temp, &temp, &temp, &temp, &temp, &temp) == 8)   {
                            //写之前先擦除
                        Bsp_At24c02_Write_Page(0x08, RX_OTAVerBuf, OTA_VER_NUM_SIZE);    //将OTA版本号保存到24c02中
                        printf("OTA version number setting is successful\r\n");
                        BootLoader_CMD_SHOW();
                    } else {
                        printf("Incorrect format of OTA version number\r\n");
                        BootLoader_CMD_SHOW();
                        return ;
                    }
            } else {
                printf("Incorrect length of OTA version number\r\n");
                BootLoader_CMD_SHOW();
                return ;
            }
        }
    }
}

uint16_t BootLoade_CRC16_XMODE(uint8_t *data, uint16_t datalen) {

    uint8_t i;
    uint16_t Crcinit = 0x0000;
    uint16_t Crcipoly = 0x1021;

    while (datalen--) {
        Crcinit = (*data << 8) ^ Crcinit;
        for (i = 0; i < 8; i++) {
            if (Crcinit & 0x8000)
                Crcinit = (Crcinit << 1) ^ Crcipoly;
            else 
                Crcinit = (Crcinit << 1);
        }
        data++;
    }
    return Crcinit;
}
