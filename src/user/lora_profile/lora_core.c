#include "lora_core.h"

#define TAG "LoRa core"

CommunicationPacket_t LoRaPacket; // 通讯数据包
LoRaDevice_t LoRaDevice;          // 设备信息
LoRaRegister_t RegisterDevice;    // 待注册设备信息
LoRaBackup_t LoRaBackup;          // 备份信息
/**
 * CusProfile_Send
 * @brief Lora数据发送
 * @author Honokahqh
 * @date 2023-08-05
 */
void CusProfile_Send(uint16_t DAddr, uint8_t cmd, uint8_t *data, uint8_t len, uint8_t isAck)
{
    static uint8_t PackageID;
    uint8_t i;
    LoRaPacket.Tx_Data[DevType_Addr] = Device_Type;                      // 主机
    LoRaPacket.Tx_Data[PanIDH_Addr] = LoRaDevice.PanID >> 8;             // 所属网络
    LoRaPacket.Tx_Data[PanIDL_Addr] = LoRaDevice.PanID;                  // 所属网络
    LoRaPacket.Tx_Data[SAddrH_Addr] = LoRaDevice.Self.shortAddress >> 8; // 本机网络地址
    LoRaPacket.Tx_Data[SAddrL_Addr] = LoRaDevice.Self.shortAddress;      // 本机网络地址
    LoRaPacket.Tx_Data[DAddrH_Addr] = DAddr >> 8;                        // 目的网络地址
    LoRaPacket.Tx_Data[DAddrL_Addr] = DAddr;                             // 目的网络地址
    if (isAck)
    {
        LoRaPacket.Tx_Data[PackID_Addr] = LoRaPacket.Rx_PID;
    }
    else
    {
        LoRaPacket.Tx_Data[PackID_Addr] = PackageID;
        PackageID++;
    }
    LoRaPacket.Tx_Data[Cmd_Addr] = cmd;
    LoRaPacket.Tx_Data[Len_Addr] = len;
    for (i = 0; i < len; i++)
        LoRaPacket.Tx_Data[i + Data_Addr] = *data++;
    LoRaPacket.Tx_Data[len + Data_Addr] = XOR_Calculate(LoRaPacket.Tx_Data, len + Data_Addr);
    LoRaPacket.Tx_Len = len + Data_Addr + 1;
    LOG_I(TAG, "DAddr:%04x, CMD:%02x data:", DAddr, cmd);
#if LOG_LEVEL >= LOG_I
    for(uint8_t i = 0; i < LoRaPacket.Tx_Len; i++)
    {
        printf("%02X ", LoRaPacket.Tx_Data[i]);
    }
    printf("\r\n");
#endif
    Lora_Send_Data(LoRaPacket.Tx_Data, LoRaPacket.Tx_Len);
}

/**
 * CusProfile_Receive
 * @brief Lora数据接收处理-根据协议解析数据并调用相应函数处理数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void CusProfile_Receive()
{
    Lora_ReceiveData2State();
    /* 异或校验是否一致 */
    if (XOR_Calculate(LoRaPacket.Rx_Data, LoRaPacket.Rx_Len - 1) != LoRaPacket.Rx_Data[LoRaPacket.Rx_Len - 1])
    {
        return;
    }
    /* 从机收到Beacon且在注册模式时,跳过PanID验证 */
    if (LoRaPacket.Rx_CMD == Beacon && LoRaDevice.NetMode == SearchForMaster && LoRaPacket.Rx_DAddr == LoRaDevice.Self.shortAddress)
    {
        goto DataProcess;
    }
    /* 验证PanID */
    if (LoRaPacket.Rx_PanID != LoRaDevice.PanID && LoRaPacket.Rx_PanID != BoardCast && LoRaDevice.PanID != BoardCast)
    {
        return;
    }
    /* 主机收到BeaconRequest且在注册模式时，跳过短地址验证 step2 */
    if (LoRaPacket.Rx_CMD == BeaconRequest && LoRaDevice.NetMode == SearchForSlave)
    {
        goto DataProcess;
    }
    /* 验证目的短地址 为广播或自身地址 */
    if (LoRaPacket.Rx_DAddr != LoRaDevice.Self.shortAddress && LoRaPacket.Rx_DAddr != BoardCast)
    {
        return;
    }
    if (Compare_ShortAddr(LoRaPacket.Rx_SAddr) == 0xFF && LoRaPacket.Rx_SAddr != RegisterDevice.device.shortAddress)
    { // 短地址验证,在设备列表内/注册列表内
        return;
    }

DataProcess:
    switch (LoRaPacket.Rx_CMD)
    {
    case BeaconRequest:
        Cmd_BeaconRequest();
        break;
    case Beacon:
        Cmd_Beacon();
        break;
    case SlaverInNet:
        Cmd_SlaverInNet();
        break;
    case DeviceAnnonce:
        Cmd_DeviceAnnonce();
        break;
    case Master_Request_Leave:
        Cmd_Master_Request_Leave();
        break;
    case Slaver_Request_Leave:
        Cmd_Slaver_Request_Leave();
        break;
    case Lora_Change_Para:
        Cmd_Lora_Change_Para();
        break;
    case HeartBeat:
        Cmd_HeartBeat();
        break;
    default:
        User_Slaver_Cmd();
        break;
    }
}

static void LoRa_NetCheck();
static void LoRa_RegisterCheck();
/**
 * LoRa_Period_1s
 * @brief 从机每秒状态更新处理
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRa_Period_1s()
{
    LoRa_NetCheck();
    LoRa_RegisterCheck();
}

void LoRa_NetCheck()
{
    /* 未入网状态持续入网 */
    if (LoRaPacket.Wait_ACK == 0 && LoRaDevice.Net_State == Net_NotJoin && LoRaDevice.NetMode == SearchForMaster)
    {
        PCmd_BeaconRequest();
        return;
    }

    /* ACK超时 */
    if (LoRaPacket.AckTimeout)
    {
        LoRaPacket.AckTimeout--;
        if (LoRaPacket.Wait_ACK && LoRaPacket.AckTimeout == 0)
        {

            LoRaPacket.ErrTimes++;
            if (LoRaPacket.ErrTimes > 3)
            {
                switch (LoRaPacket.Wait_ACK)
                {
                case SlaverInNet: // 入网失败
                    memset(&LoRaDevice.Master, 0, sizeof(LoRa_Node_t));
                    LoRaDevice.Self.shortAddress = LoRaBackup.SAddr;
                    LoRaDevice.PanID = LoRaBackup.PanID;
                    LoRaDevice.Net_State = Net_NotJoin;
                    break;
                default:
                    LoRaPacket.Wait_ACK = 0;
                    LoRaPacket.ErrTimes = 0;
                    break;
                }
            }
            switch (LoRaPacket.Wait_ACK)
            {
            case BeaconRequest:
                PCmd_BeaconRequest();
                LoRaPacket.AckTimeout = 3;
                break;
            case SlaverInNet:
                PCmd_SlaverInNet();
                LoRaPacket.AckTimeout = 2;
                break;
            }
        }
    }
}
/**
 * Master_Period_1s
 * @brief 主机每秒状态更新处理
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRa_RegisterCheck()
{
    if (RegisterDevice.timeout)
        RegisterDevice.timeout--;
    if (RegisterDevice.timeout == 0)
        memset(&RegisterDevice, 0, sizeof(LoRaRegister_t));
    // 注册到第二阶段时数据保存在slaver内
    for (uint8_t i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].timeout)
        {
            LoRaDevice.Slaver[i].timeout--;
            if (LoRaDevice.Slaver[i].timeout == 0)
            {
                memset(&LoRaDevice.Slaver[i], 0, sizeof(LoRa_Node_t));
            }
        }
    }
}
