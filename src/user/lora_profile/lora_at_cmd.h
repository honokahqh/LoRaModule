#ifndef AT_PROCESS_H
#define AT_PROCESS_H
#include "lora_core.h"
#include "lora_user.h"

//��Ҫ�����ȫ������
//LoraӲ������:Ƶ��,����,����,��Ƶ����
//Э�����(ͨ��):�������,�豸����,�豸��ַ,�豸PANID
//Э�����(����):����ʱ��,�����豸�б�(MAC+SADDR)
//Э�����(�ӻ�):�Ƿ�����

uint8_t processATCommand(char *input);
void handleSend(uint8_t *data, uint8_t len);
#endif //
