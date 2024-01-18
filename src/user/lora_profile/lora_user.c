#include "lora_core.h"
#include "lora_user.h"

#include "APP.h"
#include "ymodem.h"

/**
 * ChipID_Syn
 * @brief 芯片数据同步至MAC和chipID
 * @author Honokahqh
 * @date 2023-08-05
 */
void ChipID_Syn()
{
    uint32_t id[2];
    id[0] = EFC->SN_L;
    id[1] = EFC->SN_H;
    LoRaDevice.Self.Mac[0] = id[0] & 0xFF;
    LoRaDevice.Self.Mac[1] = (id[0] >> 8) & 0xFF;
    LoRaDevice.Self.Mac[2] = (id[0] >> 16) & 0xFF;
    LoRaDevice.Self.Mac[3] = (id[0] >> 24) & 0xFF;

    LoRaDevice.Self.Mac[4] = id[1] & 0xFF;
    LoRaDevice.Self.Mac[5] = (id[1] >> 8) & 0xFF;
    LoRaDevice.Self.Mac[6] = (id[1] >> 16) & 0xFF;
    LoRaDevice.Self.Mac[7] = (id[1] >> 24) & 0xFF;
    LoRaDevice.chip_ID = LoRaDevice.Self.Mac[0] + LoRaDevice.Self.Mac[1] + LoRaDevice.Self.Mac[2] + LoRaDevice.Self.Mac[3] +
                         LoRaDevice.Self.Mac[4] + LoRaDevice.Self.Mac[5] + LoRaDevice.Self.Mac[6] + LoRaDevice.Self.Mac[7];
}

/**
 * Lora_StateInit
 * @brief 设备初始化
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_StateInit()
{
    LoRaDevice.Self.DevcieType = Device_Type;
    ChipID_Syn();
    LoRaFlashdataSyn();
}

/**
 * User_Slaver_Cmd
 * @brief 自定义的指令
 * @author Honokahqh
 * @date 2023-08-05
 */
void User_Slaver_Cmd()
{
    switch (LoRaPacket.Rx_CMD)
    {

    default:
        break;
    }
}

/**
 * Random_Delay
 * @brief 0~10ms的随机延迟
 * @author Honokahqh
 * @date 2023-08-05
 */
void Random_Delay()
{
    static uint8_t i;
    uint16_t ms;
    ms = (LoRaDevice.Self.Mac[3] << 8) + LoRaDevice.Self.Mac[4];
    ms = (ms / (1 + i * 2)) % 10;
    i++;
    delay_ms(ms);
}

/**
 * Lora_Send_Data
 * @brief 数据发送
 * @param data 要发送的数据
 * @param len 数据的长度
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_Send_Data(uint8_t *data, uint8_t len)
{
    delay_ms(10);
    Radio.Send(data, len);
    Wait2TXEnd();
}

/**
 * Lora_Sleep
 * @brief Lora进入睡眠模式
 * @author Honokahqh
 * @date 2023-08-05
 */
void Lora_Sleep()
{
    Radio.Rx(0);
    Wait2RXEnd();
    Radio.Sleep();
    if (Radio.GetStatus() != RF_IDLE)
        return;
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, false);
    delay_ms(5);
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
    delay_ms(5);
}

void Cmd_Lora_SendData()
{
    if (LoRaDevice.Master.shortAddress == LoRaPacket.Rx_SAddr)
        LoRaDevice.Master.RSSI = LoRaPacket.Rx_RSSI;
    for (uint8_t i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress == LoRaPacket.Rx_SAddr)
        {
            LoRaDevice.Slaver[i].RSSI = LoRaPacket.Rx_RSSI;
            break;
        }
    }
    UART_SendData(&LoRaPacket.Rx_Data[Data_Addr], LoRaPacket.Rx_Data[Len_Addr]);
}