## LoRaģ��-get start����

## ���̼��

���ڱ����̿���ͨ��UART0����AT�������ò���/ģʽ��,ʵ�ָ���LoRa�豸���齨/�˳�ͨѶ����
SF,BW,CR,FRQΪLoRaͨѶ����,��һ�����޷�ͨѶ��PanID,SAddrΪ�������
ÿ���豸���԰�һ��Master,�ڰ�Masterʱ����Ҫ����ӻ�ģʽAT+NETOPEN2
ÿ���豸�����԰�MAX_CHILDERN��Slaver���ڰ�slaverʱ����Ҫ��������ģʽAT+NETOPEN1
�ӻ����󶨺���Զ����빤���ŵ�����������Ҫͨ������AT+NETOPEN0����

## FLASH����

0x08000000-0x0801FFFFΪASR6601CB��Flash��Χ��0x08003FFFFΪASR6601SE��Flash��Χ��ÿ4KBΪһҳ
�ù̼����ù̼���СΪ(128/2 - 12) = 52KB,��ҳ�ֱ����ڴ洢������Ϣ-���豸��Ϣ-�̼�OTA��Ϣ
BOOT����ΪFAC�̼������ᱻ����

### OTA

ʹ��YmodemЭ��LoRa���д��䣬���̽�����OTA��������������λ�����ͨ��LoRa����������

## ���ʹ������

�����������£�

AT+FACTORY ---> (��ѡ��AT+SF/AT+BW����ͨѶ������AT+PANID�����������) ---> AT+NETOPEN1

�ӻ��������£�

AT+FACTORY ---> AT+NETOPEN2

������ӡ����(����)

������AT+FACTORY
W (1030410) LoRaAtCmd: Factory
E (98) flashData: Unknow Error # ����¼��FACû����¼APP,FLASH����
I (98) flashData: NetState:0, NetMode:2, NetModeTimeout:0 
I (99) flashData: channel:0, PanID:fffe SelfAddr:0000, MasterAddr:0000 
I (99) flashData: SpreadingFactor:7, BandWidth:0 
I (707) main: app start
I (707) LoRa core: DAddr:fffe, CMD:01 data:7B FF FE 00 00 FF FE 00 01 08 A7 D7 1A 00 C3 02 32 01 EA 
I (3772) LoRa core: DAddr:fffe, CMD:01 data:7B FF FE 00 00 FF FE 01 01 08 A7 D7 1A 00 C3 02 32 01 EB 
������AT+NETOPEN1
I (140177) LoRaAtCmd: NetMode:1
I (140177) flashData: NetState:0, NetMode:1, NetModeTimeout:0 
I (140178) flashData: channel:0, PanID:0290 SelfAddr:0000, MasterAddr:0000 
I (140179) flashData: SpreadingFactor:7, BandWidth:0 

- ��ʱ�豸�Ѿ����У��Դӻ�����AT+FACTORY�Լ�AT+NETOPEN2��������ģʽ

I (188427) LoRa function: receive packet from PanID:FFFE, SAddr:030C, DAddr:0000, CMD:01, data:7B FF FE 03 0C 00 00 00 01 08 B7 9F 39 8C 04 BA 32 01 6C 
I (188429) LoRaCmd_Master: New Device Register, Mac:b7:9f:39:8c:04:ba:32:01
I (188429) LoRa core: DAddr:030c, CMD:02 data:7B 02 90 00 00 03 0C 00 02 06 05 9C 00 00 07 01 7D 
I (188572) LoRa function: receive packet from PanID:0290, SAddr:059C, DAddr:0000, CMD:03, data:7B 02 90 05 9C 00 00 00 03 08 B7 9F 39 8C 04 BA 32 01 6B 
I (188573) LoRa core: DAddr:059c, CMD:04 data:7B 02 90 00 00 05 9C 00 04 00 74 
I (188628) flashData: AddSlaver:0, SAddr:1436 

- ��ʱ�豸������ϣ���������ѡ��������豸��Ҳ����ѡ��ʹ������AT+NETOPEN0�ص������ŵ�

I (261629) LoRaAtCmd: NetMode:0
I (261629) flashData: NetState:0, NetMode:0, NetModeTimeout:0 
I (261630) flashData: channel:0, PanID:0290 SelfAddr:0000, MasterAddr:0000 
I (261631) flashData: SpreadingFactor:7, BandWidth:0 
I (261631) flashData: SynSlaver:0, SA:1436 
I (307325) LoRa function: receive packet from PanID:0290, SAddr:059C, DAddr:0000, CMD:20, data:7B 02 90 05 9C 00 00 03 20 04 7B 0C 00 FF DF 
I (307326) LoRaCmd_Master: HeartBeat, device saddr:059c, verion:12, rssi:255
������55 AA 80 05 9C 01 # 16���Ʒ���
I (312945) LoRa core: DAddr:059c, CMD:80 data:7B 02 90 00 00 05 9C 03 80 01 01 F3 
I (313065) LoRa function: receive packet from PanID:0290, SAddr:059C, DAddr:0000, CMD:81, data:7B 02 90 05 9C 00 00 03 81 02 FF C5 CA 
