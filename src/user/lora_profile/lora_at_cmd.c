#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lora_core.h"

#define TAG "LoRaAtCmd"
// AT Process用于外部控制Lora模块，通过串口发送AT指令，Lora模块接收到指令后，会调用对应的处理函数
// 如果将协议集成到设备，可以直接调parseATCommand(const char *input)，也可以自己处理
// 处理函数原型
typedef void (*AT_Handler)(int parameter);

const char *AT_CommandList[] = {
    "AT+RST",     // 重启
    "AT+CHANNEL", // 设置信道              重启生效
    "AT+BW",      // 设置带宽              重启生效
    "AT+SF",      // 设置扩频因子          重启生效
    "AT+TXPOWER", // 设置发送功率          重启生效
    "AT+PANID",   // 设置PANID             重启生效
    "AT+SADDR",   // 设置短地址            重启生效
    "AT+NETOPEN", // 开启网络              重启生效
    "AT+LEAVE",   // Endpoint离开网络      直接生效
    "AT+DELETE",  // 删除设备X             直接生效
    "AT+FACTORY", // 恢复出厂设置          重启生效
    "AT+PRINT",   // 打印当前连接的设备短地址
};

typedef struct
{
    uint8_t index;
    int parameter;
} AT_Command;

AT_Command parseATCommand(char *input)
{
    AT_Command result = {0xFF, -1};

    for (int i = 0; i < sizeof(AT_CommandList) / sizeof(AT_CommandList[0]); i++)
    {
        if (strncmp(input, AT_CommandList[i], strlen(AT_CommandList[i])) == 0)
        {
            result.index = i;
            const char *param_str = input + strlen(AT_CommandList[i]);
            if (*param_str != '\0')
            {
                result.parameter = atoi(param_str);
            }
            break;
        }
    }
    return result;
}
extern uint16_t page1_offset;

void handleSend(uint8_t *data, uint8_t len)
{
    uint8_t cmd;
    uint16_t DAddr;
    cmd = data[0];
    DAddr = data[1] << 8 | data[2];
    CusProfile_Send(DAddr, cmd, &data[3], len - 3, 1);
}

void handleSleep(int parameter)
{
}

void handleRst(int parameter)
{
    uint8_t temp_data[8];
    memset(temp_data, 0xFF, 8);
    LoRa_NetPara_Save(Type_Lora_net);
    delay_ms(100);
    system_reset();
}

void handleChannel(int parameter)
{
    if (parameter < 0 || parameter > 100)
    {
        LOG_E(TAG, "Channel:%d is error\r\n", parameter);
        return;
    }
    LOG_I(TAG, "Channel:%d\r\n", parameter);
    LoRaBackup.channel = parameter;
}

void handleBW(int parameter)
{
    if (parameter > 2 || parameter < 0)
    {
        LOG_E(TAG, "BW:%d is error\r\n", parameter);
        return;
    }
    LOG_I(TAG, "BW:%d\r\n", parameter);
    LoRaBackup.BandWidth = parameter;
    LoRa_NetPara_Save(Type_Lora_net);
}

void handleSF(int parameter)
{
    if (parameter < 7 || parameter > 12)
    {
        LOG_E(TAG, "SF:%d is error\r\n", parameter);
        return;
    }
    LOG_I(TAG, "SF:%d\r\n", parameter);
    LoRaBackup.SpreadingFactor = parameter;
    LoRa_NetPara_Save(Type_Lora_net);
}

void handleTxpower(int parameter)
{
    if (parameter < 0 || parameter > 22)
    {
        LOG_E(TAG, "Txpower:%d is error\r\n", parameter);
        return;
    }
    LOG_I(TAG, "Txpower:%d\r\n", parameter);
}

void handlePanid(int parameter)
{
    // 0xFFFE为组播ID、0xFFFF为广播ID
    if (parameter < 1 || parameter > 0xFFFD)
    {
        LOG_E(TAG, "PanID:%04X is error\r\n", parameter);
        return;
    }
    LOG_I(TAG, "PanID:%04X\r\n", parameter);
    LoRaBackup.PanID = parameter;
    LoRa_NetPara_Save(Type_Lora_net);
}

void handleSaddr(int parameter)
{
    if (parameter < 0 || parameter > 0xFFFD)
    {
        LOG_E(TAG, "SAddr:%04X is error\r\n", parameter);
        return;
    }
    else if (parameter == 0)
        LOG_W(TAG, "SAddr:%04X is special\r\n", parameter);
    else
        LOG_I(TAG, "SAddr:%04X\r\n", parameter);
    LoRaBackup.SAddr = parameter;
    LoRa_NetPara_Save(Type_Lora_net);
}

void handleNetopen(int parameter)
{
    if (parameter == 1)
        LoRaDevice.NetMode = 1;
    else if (parameter == 2)
        LoRaDevice.NetMode = 2;
    else
        LoRaDevice.NetMode = 0;
    LOG_I(TAG, "NetMode:%d\r\n", LoRaDevice.NetMode);
    LoRaDevice.NetMode = LoRaDevice.NetMode;
    LoRa_NetPara_Save(Type_Lora_net);
    Reset_LoRa();
}

void handleLeave(int parameter)
{
    // 处理逻辑...
    PCmd_Slaver_Request_Leave();
}

void handleDelet(int parameter)
{
    if (parameter < 0 || parameter > MAX_CHILDREN)
    {
        LOG_E(TAG, "Delet:%d is error\r\n", parameter);
        return;
    }
    LOG_I(TAG, "Delet:%d\r\n", parameter);
    PCmd_Master_Request_Leave(parameter);
    memset(&LoRaDevice.Slaver[parameter], 0, sizeof(LoRa_Node_t));
}

void handleFactory(int parameter)
{
    LOG_W(TAG, "Factory\r\n");
    flash_erase_page(FlashData1_ADDR);
    flash_erase_page(FlashData2_ADDR);
    delay_ms(100);
    system_reset();
}
void handlePrint(int parameter)
{
    LOG_I(TAG, "NetState:%d, NetMode:%d, NetModeTimeout%d \r\n", LoRaDevice.Net_State, LoRaDevice.NetMode, LoRaDevice.NetModeTimeout);
    LOG_I(TAG, "channel:%d, PanID:%d SelfAddr:%d, MasterAddr:%d \r\n", LoRaDevice.channel, LoRaDevice.PanID, LoRaDevice.Self.shortAddress, LoRaDevice.Master.shortAddress);
    LOG_I(TAG, "SpreadingFactor:%d, BandWidth:%d \r\n", LoRaDevice.SpreadingFactor, LoRaDevice.BandWidth);
    for (uint8_t i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress)
            LOG_I(TAG, "ID:%d SAddr:%d MAC:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n", i, LoRaDevice.Slaver[i].shortAddress,
                  LoRaDevice.Slaver[i].Mac[0], LoRaDevice.Slaver[i].Mac[1], LoRaDevice.Slaver[i].Mac[2], LoRaDevice.Slaver[i].Mac[3],
                  LoRaDevice.Slaver[i].Mac[4], LoRaDevice.Slaver[i].Mac[5], LoRaDevice.Slaver[i].Mac[6], LoRaDevice.Slaver[i].Mac[7]);
    }
}

// 定义处理函数数组
AT_Handler AT_Handlers[] = {
    handleRst,
    handleChannel,
    handleBW,
    handleSF,
    handleTxpower,
    handlePanid,
    handleSaddr,
    handleNetopen,
    handleLeave,
    handleDelet,
    handleFactory,
    handlePrint,
};

void executeCommand(AT_Command parsed_command)
{
    if (parsed_command.index != 0xFF)
    {
        AT_Handlers[parsed_command.index](parsed_command.parameter);
    }
}

uint8_t processATCommand(char *input)
{
    AT_Command command = parseATCommand(input);
    if (command.index == 0xFF)
    {
        return 0;
    }
    executeCommand(command);
    return 1;
}