#include "lora_core.h"

#define TAG "LoRa function"
/**
 * Get_IDLE_ID
 * @brief 获取最小空闲ID
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Get_IDLE_ID()
{
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress == 0)
            break;
    }
    if (i < MAX_CHILDREN) // 有空闲ID
        return i;
    return 0xFF;
}

/**
 * Compare_ShortAddr
 * @brief 短地址匹配
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_ShortAddr(uint16_t Short_Addr)
{
    uint8_t i;
    for (i = 0; i < MAX_CHILDREN; i++)
        if (LoRaDevice.Slaver[i].shortAddress == Short_Addr && LoRaDevice.Slaver[i].shortAddress != 0)
            break;
    if (i < MAX_CHILDREN)
        return i;
    return 0xFF;
}

/**
 * Compare_MAC
 * @brief MAC地址匹配
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t Compare_MAC(uint8_t *Mac)
{
    uint8_t i, j, count;
    for (i = 0; i < MAX_CHILDREN; i++)
    {
        count = 0;
        for (j = 0; j < MAC_Size; j++)
        {
            if (LoRaDevice.Slaver[i].Mac[j] == Mac[j])
                count++;
        }
        if (count == MAC_Size)
            return i;
    }
    return 0xFF;
}

/**
 * XOR_Calculate
 * @brief 异或校验计算
 * @author Honokahqh
 * @date 2023-08-05
 */
uint8_t XOR_Calculate(uint8_t *data, uint8_t len)
{
    uint8_t x_or = 0, i;
    for (i = 0; i < len; i++)
        x_or = x_or ^ *data++;
    return x_or;
}

/**
 * Lora_ReceiveData2State
 * @brief 接收数据存入各类参数
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_ReceiveData2State()
{
    LoRaPacket.Rx_DevType = LoRaPacket.Rx_Data[DevType_Addr];
    LoRaPacket.Rx_PanID = (LoRaPacket.Rx_Data[PanIDH_Addr] << 8) + LoRaPacket.Rx_Data[PanIDL_Addr];
    LoRaPacket.Rx_DAddr = (LoRaPacket.Rx_Data[DAddrH_Addr] << 8) + LoRaPacket.Rx_Data[DAddrL_Addr];
    LoRaPacket.Rx_SAddr = (LoRaPacket.Rx_Data[SAddrH_Addr] << 8) + LoRaPacket.Rx_Data[SAddrL_Addr];
    LoRaPacket.Rx_CMD = LoRaPacket.Rx_Data[Cmd_Addr];
    LoRaPacket.Rx_PID = LoRaPacket.Rx_Data[PackID_Addr];

    LOG_I(TAG, "receive packet from PanID:%04X, SAddr:%04X, DAddr:%04X, CMD:%02X, data:",
          LoRaPacket.Rx_PanID, LoRaPacket.Rx_SAddr, LoRaPacket.Rx_DAddr, LoRaPacket.Rx_CMD);
#if LOG_LEVEL >= LOG_I
    for (uint8_t i = 0; i < LoRaPacket.Rx_Len; i++)
    {
        printf("%02X ", LoRaPacket.Rx_Data[i]);
    }
    printf("\r\n");
#endif
}

// 函数比较两个 MAC 地址是否相同
int CompareMac(const uint8_t Mac1[8], const uint8_t Mac2[8])
{
    return memcmp(Mac1, Mac2, 8) == 0;
}

void Reset_LoRa()
{
    LoRaFlashdataSyn();
    LoraReInit();
    // system_reset();
}
