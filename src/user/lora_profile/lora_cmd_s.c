#include "lora_core.h"

#define TAG "LoRaCmd_Slaver"
/**
 * Cmd_Beacon
 * @brief Sub-device received network access permission information
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Beacon()
{
    // channel Bw sf panid selfaddr会被临时修改 需要有注册失败时的恢复机制
    if (LoRaPacket.Rx_Data[Len_Addr] == 6)
    {
        LoRaDevice.Master.shortAddress = (LoRaPacket.Rx_Data[SAddrH_Addr] << 8) + LoRaPacket.Rx_Data[SAddrL_Addr];
        LoRaDevice.Self.shortAddress = (LoRaPacket.Rx_Data[Data_Addr] << 8) + LoRaPacket.Rx_Data[Data_Addr + 1];
        LoRaDevice.PanID = LoRaPacket.Rx_PanID;
        LoRaDevice.channel = LoRaPacket.Rx_Data[Data_Addr + 2];
        LoRaDevice.BandWidth = LoRaPacket.Rx_Data[Data_Addr + 3];
        LoRaDevice.SpreadingFactor = LoRaPacket.Rx_Data[Data_Addr + 4];
        LoRaDevice.Net_State = 2; // 未入网 已建立连接
        LoRaPacket.Wait_ACK = SlaverInNet;
        PCmd_SlaverInNet();
        LOG_I(TAG, "Beacon, SF:%d, BW:%d, Channel:%d, PanID:%04X, SAddr:%04X\r\n",
              LoRaDevice.SpreadingFactor, LoRaDevice.BandWidth, LoRaDevice.channel, LoRaDevice.PanID, LoRaDevice.Self.shortAddress);
    }
    else
        LOG_E(TAG, "Beacon Format Error\r\n");
}

/**
 * Cmd_DeviceAnnonce
 * @brief Sub-device declaration after receiving the host's network reply.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_DeviceAnnonce()
{
    LoRaDevice.Net_State = Net_JoinGateWay;
    LoRaPacket.Wait_ACK = 0;
    LoRaBackup.channel = LoRaDevice.channel;
    LoRaBackup.SAddr = LoRaDevice.Self.shortAddress;
    LoRaBackup.PanID = LoRaDevice.PanID;
    LoRaBackup.BandWidth = LoRaDevice.BandWidth;
    LoRaBackup.SpreadingFactor = LoRaDevice.SpreadingFactor;
    LoRaDevice.Net_State = Net_JoinGateWay;
    LoRaDevice.NetMode = Normal;
    LoRa_NetPara_Save(Type_Lora_net);
    LOG_I(TAG, "DeviceAnnonce\r\n");
    Reset_LoRa();
}

/**
 * Cmd_Master_Request_Leave
 * @brief Sub-device receives a request to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Master_Request_Leave()
{
    LoRaBackup.SAddr = LoRaDevice.chip_ID;
    LoRaBackup.PanID = LoRaDevice.chip_ID;
    LoRaDevice.Net_State = Net_NotJoin;
    LoRaDevice.NetMode = Normal;
    memset(&LoRaDevice.Master, 0, sizeof(LoRa_Node_t));
    LoRa_NetPara_Save(Type_Lora_net);
    Reset_LoRa();
}

/**
 * Cmd_Lora_Change_Para
 * @brief change lora parameter, such as channel, spreading factor, bandwidth
 * @author Honokahqh
 * @date 2023-10-07
 */
void Cmd_Lora_Change_Para()
{
    LoRaBackup.SAddr = (LoRaPacket.Rx_Data[Data_Addr] << 8) + LoRaPacket.Rx_Data[Data_Addr + 1];
    LoRaBackup.PanID = (LoRaPacket.Rx_Data[Data_Addr + 2] << 8) + LoRaPacket.Rx_Data[Data_Addr + 3];
    LoRaBackup.channel = LoRaPacket.Rx_Data[Data_Addr + 4];
    LoRaBackup.BandWidth = LoRaPacket.Rx_Data[Data_Addr + 5];
    LoRaBackup.SpreadingFactor = LoRaPacket.Rx_Data[Data_Addr + 6];
    LoRaDevice.Master.shortAddress = (LoRaPacket.Rx_Data[Data_Addr + 8] << 8) + LoRaPacket.Rx_Data[Data_Addr + 9];
    LoRa_NetPara_Save(Type_Lora_net);
    LOG_I(TAG, "Lora_Change_Para, SF:%d, BW:%d, Channel:%d, PanID:%04X, SAddr:%04X\r\n",
          LoRaDevice.SpreadingFactor, LoRaDevice.BandWidth, LoRaDevice.channel, LoRaDevice.PanID, LoRaDevice.Self.shortAddress);
    Reset_LoRa();
}

/**
 * Cmd_Query_Rssi
 * @brief Cmd_Query_Rssi
 * @author Honokahqh
 * @date 2023-08-05
 */
void Cmd_Query_Rssi()
{
    uint8_t temp[2];
    temp[0] = LoRaPacket.Rx_RSSI >> 8;
    temp[1] = LoRaPacket.Rx_RSSI;
    CusProfile_Send(LoRaPacket.Rx_SAddr, Lora_Query_RSSI_ACK, temp, 2, TRUE);
}

/**
 * PCmd_BeaconRequest
 * @brief Sub-device actively requests to join the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_BeaconRequest()
{
    if (LoRaDevice.Net_State == Net_NotJoin && LoRaDevice.NetMode == SearchForMaster)
    {
        CusProfile_Send(BoardCast, BeaconRequest, LoRaDevice.Self.Mac, 8, FALSE);
        LoRaPacket.Wait_ACK = BeaconRequest;
        LoRaPacket.AckTimeout = 3;
    }
}

/**
 * PCmd_SlaverInNet
 * @brief Sub-device actively requests to join the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_SlaverInNet()
{
    CusProfile_Send(BoardCast, SlaverInNet, LoRaDevice.Self.Mac, 8, FALSE);
}

/**
 * PCmd_Slaver_Request_Leave
 * @brief Sub-device actively requests to leave the network.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_Slaver_Request_Leave()
{
    CusProfile_Send(LoRaDevice.Master.shortAddress, Slaver_Request_Leave, NULL, 0, FALSE);
    memset(&LoRaDevice.Master, 0, sizeof(LoRa_Node_t));
    LoRaDevice.Net_State = Net_NotJoin;
    LoRa_NetPara_Save(Type_Lora_net);
    Reset_LoRa();
}

/**
 * PCmd_HeartBeat
 * @brief Sub-device sends a heartbeat.
 * @author Honokahqh
 * @date 2023-08-05
 */
void PCmd_HeartBeat()
{
    uint8_t temp[20];
    if (LoRaDevice.Net_State != Net_JoinGateWay)
        return;
    temp[0] = Device_Type;
    temp[1] = Dev_Version;
    temp[2] = LoRaDevice.Master.RSSI >> 8;
    temp[3] = LoRaDevice.Master.RSSI;
    CusProfile_Send(LoRaDevice.Master.shortAddress, HeartBeat, temp, 4, FALSE);
}
