#ifndef __LORA_FLASH_H__
#define __LORA_FLASH_H__

#include "APP.h"
      
#define CM4_IRQ_VECT_BASE           0xE000ED08
#define CM4_IRQ_EN                  0xE000E100
#define CM4_IRQ_CLR                 0xE000E180

#define FLASH_START_ADDR            0x08000000
#define FLASH_MAX_SIZE              0x20000 //128KB

#define FlashData1_ADDR              0x0800C000
#define FlashData2_ADDR              0x0800D000 
#define OTA_ADDR                     0x0800E000

#define APP_ADDR                     0x08010000
#define Boot_ADDR                    0x08000000
// page 1 存储类型
#define Type_Lora_net 0x01
#define Type_Lora_net_len 16

// page 2
#define Type_Lora_Slaver        0x06
#define Type_Lora_Slaver_len    16

void LoRa_NetPara_Save(uint8_t type); // page1相关数据存储

void LoRaAddSlaver(uint8_t ID); // page2相关数据存储
void LoRaDelSlaver(uint8_t ID); // page2相关数据存储

void LoRaFlashdataSyn();

void IAP_Check();
void boot_to_app(uint32_t addr);
#endif

