#ifndef AT_PROCESS_H
#define AT_PROCESS_H
#include "lora_core.h"
#include "lora_user.h"

//需要保存的全部数据
//Lora硬件参数:频点,功率,带宽,扩频因子
//协议参数(通用):网络参数,设备类型,设备地址,设备PANID
//协议参数(主机):开网时间,连接设备列表(MAC+SADDR)
//协议参数(从机):是否连接

uint8_t processATCommand(char *input);
void handleSend(uint8_t *data, uint8_t len);
#endif //
