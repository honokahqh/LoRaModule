

#include "Lora_core.h"

#define TAG "flashData"
uint16_t page1_offset, page2_offset;

void LoRa_NetPara_Save(uint8_t type)
{
    uint8_t temp_data[16];
    memset(temp_data, 0, 16);
    if (page1_offset >= 500)
    {
        page1_offset = 0;
        flash_erase_page(FlashData1_ADDR);
        LoRa_NetPara_Save(Type_Lora_net);
    }
    switch (type)
    {
    case Type_Lora_net:
        /* code */
        temp_data[0] = type;
        temp_data[1] = LoRaBackup.SpreadingFactor & 0x0F;
        temp_data[2] = LoRaBackup.BandWidth & 0x03;
        temp_data[3] = LoRaDevice.NetMode;
        temp_data[4] = LoRaDevice.Net_State;
        temp_data[5] = LoRaBackup.channel & 0x7F;
        temp_data[6] = LoRaBackup.SAddr >> 8;
        temp_data[7] = LoRaBackup.SAddr;
        temp_data[8] = LoRaDevice.Master.shortAddress >> 8;
        temp_data[9] = LoRaDevice.Master.shortAddress;
        temp_data[10] = LoRaBackup.PanID >> 8;
        temp_data[11] = LoRaBackup.PanID;
        temp_data[12] = LoRaDevice.NetModeTimeout;
        flash_program_bytes(FlashData1_ADDR + page1_offset * 8, temp_data, 16);
        page1_offset += (Type_Lora_net_len + 7) / 8;
        break;
    default:
        break;
    }
}

void LoRaNetParaSyn()
{
    uint32_t data;
    LoRaDevice.NetMode = SearchForMaster; // 默认为搜寻主机
    for (page1_offset = 0; page1_offset < 500; page1_offset++)
    { // flash 同步
        data = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8);
        if (data == Type_Lora_net)
        {
            LoRaBackup.SpreadingFactor = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 1) & 0x0F);
            LoRaBackup.BandWidth = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 2) & 0x03);
            LoRaDevice.NetMode = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 3);
            LoRaDevice.Net_State = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 4);
            LoRaBackup.channel = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 5) & 0x7F;
            LoRaBackup.SAddr = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 6) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 7));
            LoRaDevice.Master.shortAddress = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 8) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 9));
            LoRaBackup.PanID = (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 10) << 8) + (*(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 11));
            LoRaDevice.NetModeTimeout = *(uint8_t *)(FlashData1_ADDR + page1_offset * 8 + 12);
            page1_offset += ((Type_Lora_net_len + 7) / 8) - 1;
        }
        else
            break;
    }
    if (LoRaBackup.SpreadingFactor < 7 || LoRaBackup.SpreadingFactor > 12)
        LoRaBackup.SpreadingFactor = 7;
    if (LoRaBackup.BandWidth > 2)
        LoRaBackup.BandWidth = 2;
    if (LoRaBackup.channel > 100)
        LoRaBackup.channel = 0;
    if (LoRaDevice.Net_State > 1)
        LoRaDevice.Net_State = 0;
    if (LoRaDevice.NetMode > 2)
        LoRaDevice.NetMode = 0;
    if (LoRaBackup.SAddr >= 0xFFFE )
        LoRaBackup.SAddr = LoRaDevice.chip_ID;
    if (LoRaBackup.PanID == 0xFFFF || LoRaBackup.PanID == 0x0000)
        LoRaBackup.PanID = LoRaDevice.chip_ID;
    // 数据同步
    LoRaDevice.channel = LoRaBackup.channel;
    LoRaDevice.Self.shortAddress = LoRaBackup.SAddr;
    LoRaDevice.PanID = LoRaBackup.PanID;
    LoRaDevice.BandWidth = LoRaBackup.BandWidth;
    LoRaDevice.SpreadingFactor = LoRaBackup.SpreadingFactor;
    if (LoRaDevice.NetMode == SearchForMaster)
        LoRaDevice.PanID = BoardCast; // 只修改Device,不修改backup
    LOG_I(TAG, "NetState:%d, NetMode:%d, NetModeTimeout:%d \r\n", LoRaDevice.Net_State, LoRaDevice.NetMode, LoRaDevice.NetModeTimeout);
    LOG_I(TAG, "channel:%d, PanID:%04x SelfAddr:%04x, MasterAddr:%04x \r\n", LoRaDevice.channel, LoRaDevice.PanID, LoRaDevice.Self.shortAddress, LoRaDevice.Master.shortAddress);
    LOG_I(TAG, "SpreadingFactor:%d, BandWidth:%d \r\n", LoRaDevice.SpreadingFactor, LoRaDevice.BandWidth);
}
/**
 * Lora_AsData_Add
 * @brief 主机有效-将AS数组内的数据保存在Flash中
 * @param ID:AS数组index
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRaAddSlaver(uint8_t ID)
{
    // 8字节Mac + 2字节Saddr + 1字节ID + 1字节状态 + 4字节时间 + 16字节Name
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (ID >= MAX_CHILDREN)
        return;
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        flash_erase_page(FlashData2_ADDR);
        for (uint8_t i = 0; i < MAX_CHILDREN; i++)
        {
            if (LoRaDevice.Slaver[i].shortAddress)
            {
                LoRaAddSlaver(i);
            }
        }
        return;
    }
    temp_data[0] = Type_Lora_Slaver;
    temp_data[1] = ID;
    memcpy(&temp_data[2], LoRaDevice.Slaver[ID].Mac, 8);
    temp_data[10] = LoRaDevice.Slaver[ID].shortAddress >> 8;
    temp_data[11] = LoRaDevice.Slaver[ID].shortAddress;
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, ((Type_Lora_Slaver_len + 7) / 8) * 8);
    page2_offset += (Type_Lora_Slaver_len + 7) / 8;
    LOG_I(TAG, "AddSlaver:%d, SAddr:%d \r\n", ID, LoRaDevice.Slaver[ID].shortAddress);
}

/**
 * LoRaDelSlaver
 * @brief 主机有效-将某个ID数据删除
 * @param ID:AS数组index
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRaDelSlaver(uint8_t ID)
{
    uint8_t temp_data[32];
    memset(temp_data, 0, 32);
    if (ID >= MAX_CHILDREN)
        return;
    if (page2_offset >= 500)
    {
        page2_offset = 0;
        flash_erase_page(FlashData2_ADDR);
        for (uint8_t i = 0; i < MAX_CHILDREN; i++)
        {
            if (LoRaDevice.Slaver[i].shortAddress)
            {
                LoRaAddSlaver(i);
            }
        }
        return;
    }
    temp_data[0] = Type_Lora_Slaver;
    temp_data[1] = ID;
    flash_program_bytes(FlashData2_ADDR + page2_offset * 8, temp_data, ((Type_Lora_Slaver_len + 7) / 8) * 8);
    page2_offset += (Type_Lora_Slaver_len + 7) / 8;
}

/**
 * Lora_AsData_Syn
 * @brief 主机有效-从flash内读取AS数据
 * @author Honokahqh
 * @date 2023-08-05
 */
void LoRaSlaverSyn()
{
    uint8_t ID;
    // 主机连接的设备参数同步 每32个字节一组数据，根据第12个字节是否为0判断数据是否有效
    for (page2_offset = 0; page2_offset < 500; page2_offset++)
    {
        if (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8) == Type_Lora_Slaver && *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1) < MAX_CHILDREN)
        {
            ID = *(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 1);
            memcpy(LoRaDevice.Slaver[ID].Mac, (uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 2), 8);
            LoRaDevice.Slaver[ID].shortAddress = (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 10) << 8) + (*(uint8_t *)(FlashData2_ADDR + page2_offset * 8 + 11));
            page2_offset += ((Type_Lora_Slaver_len + 7) / 8) - 1;
        }
        else
            break;
    }
    for (uint8_t i = 0; i < MAX_CHILDREN; i++)
    {
        if (LoRaDevice.Slaver[i].shortAddress)
        {
            LOG_I(TAG, "SynSlaver:%d, SA:%d \r\n", i, LoRaDevice.Slaver[i].shortAddress);
        }
    }
}

void LoRaFlashdataSyn()
{
    LoRaNetParaSyn();
    LoRaSlaverSyn();
}
/**
 * jumpToApp
 * @brief 跳转至APP
 * @author Honokahqh
 * @date 2023-08-05
 */
void jumpToApp(int addr)
{
    __asm("LDR SP, [R0]");
    __asm("LDR PC, [R0, #4]");
}

/**
 * IAP_Check
 * @brief 程序跳转检查-标志位检测
 * @author Honokahqh
 * @date 2023-08-05
 */
void IAP_Check()
{
    uint8_t temp_data[8];
#if Lora_Is_APP
    if (*(uint8_t *)(OTA_ADDR + 24) != 0x01)
    {
        flash_erase_page(OTA_ADDR);
        temp_data[0] = 1;
        flash_program_bytes(OTA_ADDR + 24, temp_data, 1);
        LOG_I(TAG, "APP:OTA Success\r\n");
    }
    if (*(uint8_t *)(OTA_ADDR + 0) != 0x01 || *(uint8_t *)(OTA_ADDR + 8) != 0x01 || *(uint8_t *)(OTA_ADDR + 16) != 0x01 || *(uint8_t *)(OTA_ADDR + 24) != 0x01)
    {
        flash_erase_page(OTA_ADDR);
        temp_data[0] = 1;
        flash_program_bytes(OTA_ADDR + 0, temp_data, 1);
        flash_program_bytes(OTA_ADDR + 8, temp_data, 1);
        flash_program_bytes(OTA_ADDR + 16, temp_data, 1);
        flash_program_bytes(OTA_ADDR + 24, temp_data, 1);
    }
#else

    /* 设备第一次上电 OTA page内数据均为0xFF */
    if (*(uint8_t *)(OTA_ADDR + 0) == 0xFF)
    {
        LOG_I(TAG, "First Power On\r\n");
        temp_data[0] = 0x01;
        flash_program_bytes(OTA_ADDR + 0, temp_data, 8);
        boot_to_app(APP_ADDR);
    }
    /* 设备正常重启 */
    else if (*(uint8_t *)(OTA_ADDR + 24) == 0x01)
    {
        LOG_I(TAG, "Normal Reboot\r\n");
        boot_to_app(APP_ADDR);
    }
    /* 设备需要OTA */
    else if (*(uint8_t *)(OTA_ADDR + 8) == 0x01 && *(uint8_t *)(OTA_ADDR + 16) == 0xFF)
    {
        LOG_I(TAG, "Need OTA\r\n");
        ymodem_init();
    }
    /* 设备OTA失败 */
    else if (*(uint8_t *)(OTA_ADDR + 8) == 0x01 && *(uint8_t *)(OTA_ADDR + 16) != 0xFF)
    {
        LOG_E(TAG, "OTA Error\r\n");
    }
    else
    {
        LOG_E(TAG, "Unknow Error\r\n");
    }
#endif
}

/**
 * boot_to_app
 * @brief 修改中断偏移量后跳转
 * @author Honokahqh
 * @date 2023-08-05
 */
void boot_to_app(uint32_t addr)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    TREMO_REG_WR(CM4_IRQ_CLR, 0xFFFFFFFF); // close interrupts
    TREMO_REG_WR(CM4_IRQ_VECT_BASE, addr); // set VTOR
    jumpToApp(addr);
}