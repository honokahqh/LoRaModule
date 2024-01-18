#include "lora_core.h"

#define TAG "LoRaCmd_Master"
/**
 * Cmd_BeaconRequest
 * @brief Uses the local chipID as an offset and the subdevice's chipID as a short address to send to the subdevice.
 *        Registers in the connected device array and sets the network stage to Net_Joining.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_BeaconRequest()
{
    uint8_t Temp_Data[6], i;
    uint16_t short_Addr;
    if (LoRaDevice.NetMode != SearchForSlave)
        return;
    i = Compare_MAC(&LoRaPacket.Rx_Data[Data_Addr]);
    if (i != 0xFF)
    { // 设备重复注册,直接许可
        memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
        goto RegisterStart;
    }

    if (CompareMac(&LoRaPacket.Rx_Data[Data_Addr], RegisterDevice.device.Mac) == 0)
    {
        // 新设备
        LOG_I(TAG, "New Device Register, Mac:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
              LoRaPacket.Rx_Data[Data_Addr + 0], LoRaPacket.Rx_Data[Data_Addr + 1], LoRaPacket.Rx_Data[Data_Addr + 2], LoRaPacket.Rx_Data[Data_Addr + 3],
              LoRaPacket.Rx_Data[Data_Addr + 4], LoRaPacket.Rx_Data[Data_Addr + 5], LoRaPacket.Rx_Data[Data_Addr + 6], LoRaPacket.Rx_Data[Data_Addr + 7]);
#if Lora_Register_Enable
        // 通知上位机,由上位机改写RegisterDevice
        return;
#endif
    }
	RegisterStart:
    i = Get_IDLE_ID();
    if (i == 0xFF)
        return;
    memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
    LoRaDevice.Slaver[i].DevcieType = LoRaPacket.Rx_DevType;
    LoRaDevice.Slaver[i].RSSI = LoRaPacket.Rx_RSSI;
    LoRaDevice.Slaver[i].timeout = Register_Timeout;
    memcpy(LoRaDevice.Slaver[i].Mac, &LoRaPacket.Rx_Data[Data_Addr], 8);
    short_Addr = LoRaPacket.Rx_SAddr + LoRaDevice.PanID;
    while (Compare_ShortAddr(short_Addr) != 0xFF || short_Addr == 0 || short_Addr > 0xFFFD)
    { // 短地址非0且该地址未被注册
        short_Addr++;
    }
    LoRaDevice.Slaver[i].shortAddress = short_Addr;
    Temp_Data[0] = LoRaDevice.Slaver[i].shortAddress >> 8;
    Temp_Data[1] = LoRaDevice.Slaver[i].shortAddress & 0xFF;
    Temp_Data[2] = LoRaDevice.channel;
    Temp_Data[3] = LoRaDevice.BandWidth;
    Temp_Data[4] = LoRaDevice.SpreadingFactor;
    Temp_Data[5] = 1;
    CusProfile_Send(LoRaPacket.Rx_SAddr, Beacon, Temp_Data, 6, TRUE);
}

/**
 * Cmd_SlaverInNet
 * @brief Handles the notification of a sub-device joining the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_SlaverInNet()
{
    uint8_t i;
    if (LoRaDevice.NetMode != SearchForSlave)
        return;
    i = Compare_ShortAddr(LoRaPacket.Rx_SAddr);
    if (i == 0xFF)
    { // 没有发现匹配的设备
        CusProfile_Send(LoRaPacket.Rx_SAddr, Master_Request_Leave, NULL, 0, FALSE);
        return;
    }
    LoRaDevice.Slaver[i].timeout = 0;
    LoRaDevice.Slaver[i].RSSI = LoRaPacket.Rx_RSSI;
    CusProfile_Send(LoRaPacket.Rx_SAddr, DeviceAnnonce, NULL, 0, TRUE);
    LoRaAddSlaver(i);
}

/**
 * Cmd_Slaver_Request_Leave
 * @brief Handles a sub-device's request to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Slaver_Request_Leave()
{
    uint8_t i;
    i = Compare_ShortAddr(LoRaPacket.Rx_SAddr);
    CusProfile_Send(LoRaDevice.Slaver[i].shortAddress, Master_Leave_ACK, NULL, 0, TRUE);
    memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
    LoRaDelSlaver(i);
}

/**
 * Cmd_HeartBeat
 * @brief Reserved for a future function related to maintaining network connection (typically used for sending periodic signals to ensure active connections).
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_HeartBeat()
{
    if (LoRaPacket.Rx_Data[Len_Addr] < 4)
        LOG_E(TAG, "HeartBeat format error\r\n");
    LOG_I(TAG, "HeartBeat, device saddr:%04x, verion:%d, rssi:%d\r\n", LoRaPacket.Rx_SAddr, LoRaPacket.Rx_Data[Data_Addr + 1],
          (short)((LoRaPacket.Rx_Data[Data_Addr + 2] << 8) + LoRaPacket.Rx_Data[Data_Addr + 3]));
}

/**
 * PCmd_Master_Request_Leave
 * @brief Function used by the master device to request a sub-device to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Master_Request_Leave(uint8_t ID)
{
    CusProfile_Send(LoRaDevice.Slaver[ID].shortAddress, Master_Request_Leave, NULL, 0, FALSE);
    memset(&LoRaDevice.Slaver[ID], 0, sizeof(LoRa_Node_t)); // 将数据全部清除，之后接收到该短地址的数据会直接要求离网
    LoRaDelSlaver(ID);
}
