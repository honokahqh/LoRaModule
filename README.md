## LoRa模组-get start例程

## 例程简介

基于本例程可以通过UART0发送AT数据配置参数/模式等,实现各个LoRa设备间组建/退出通讯网络
SF,BW,CR,FRQ为LoRa通讯参数,不一致则无法通讯。PanID,SAddr为网络参数
每个设备可以绑定一个Master,在绑定Master时，需要进入从机模式AT+NETOPEN2
每个设备最多可以绑定MAX_CHILDERN个Slaver，在绑定slaver时，需要进入主机模式AT+NETOPEN1
从机被绑定后会自动进入工作信道，而主机需要通过命令AT+NETOPEN0返回

## FLASH分区

0x08000000-0x0801FFFF为ASR6601CB的Flash范围，0x08003FFFF为ASR6601SE的Flash范围，每4KB为一页
该固件设置固件大小为(128/2 - 12) = 52KB,三页分别用于存储网络信息-子设备信息-固件OTA信息
BOOT可视为FAC固件，不会被擦除

### OTA

使用Ymodem协议LoRa进行传输，例程仅包含OTA升级，不包含上位机如何通过LoRa发送升级包

## 如何使用例程

主机流程如下：

AT+FACTORY ---> (可选：AT+SF/AT+BW配置通讯参数，AT+PANID配置网络参数) ---> AT+NETOPEN1

从机流程如下：

AT+FACTORY ---> AT+NETOPEN2

组网打印如下(主机)

发→◇AT+FACTORY
W (1030410) LoRaAtCmd: Factory
E (98) flashData: Unknow Error # 仅烧录了FAC没有烧录APP,FLASH报错
I (98) flashData: NetState:0, NetMode:2, NetModeTimeout:0 
I (99) flashData: channel:0, PanID:fffe SelfAddr:0000, MasterAddr:0000 
I (99) flashData: SpreadingFactor:7, BandWidth:0 
I (707) main: app start
I (707) LoRa core: DAddr:fffe, CMD:01 data:7B FF FE 00 00 FF FE 00 01 08 A7 D7 1A 00 C3 02 32 01 EA 
I (3772) LoRa core: DAddr:fffe, CMD:01 data:7B FF FE 00 00 FF FE 01 01 08 A7 D7 1A 00 C3 02 32 01 EB 
发→◇AT+NETOPEN1
I (140177) LoRaAtCmd: NetMode:1
I (140177) flashData: NetState:0, NetMode:1, NetModeTimeout:0 
I (140178) flashData: channel:0, PanID:0290 SelfAddr:0000, MasterAddr:0000 
I (140179) flashData: SpreadingFactor:7, BandWidth:0 

- 此时设备已经运行，对从机发送AT+FACTORY以及AT+NETOPEN2进入组网模式

I (188427) LoRa function: receive packet from PanID:FFFE, SAddr:030C, DAddr:0000, CMD:01, data:7B FF FE 03 0C 00 00 00 01 08 B7 9F 39 8C 04 BA 32 01 6C 
I (188429) LoRaCmd_Master: New Device Register, Mac:b7:9f:39:8c:04:ba:32:01
I (188429) LoRa core: DAddr:030c, CMD:02 data:7B 02 90 00 00 03 0C 00 02 06 05 9C 00 00 07 01 7D 
I (188572) LoRa function: receive packet from PanID:0290, SAddr:059C, DAddr:0000, CMD:03, data:7B 02 90 05 9C 00 00 00 03 08 B7 9F 39 8C 04 BA 32 01 6B 
I (188573) LoRa core: DAddr:059c, CMD:04 data:7B 02 90 00 00 05 9C 00 04 00 74 
I (188628) flashData: AddSlaver:0, SAddr:1436 

- 此时设备组网完毕，主机可以选择继续绑定设备，也可以选择使用命令AT+NETOPEN0回到工作信道

I (261629) LoRaAtCmd: NetMode:0
I (261629) flashData: NetState:0, NetMode:0, NetModeTimeout:0 
I (261630) flashData: channel:0, PanID:0290 SelfAddr:0000, MasterAddr:0000 
I (261631) flashData: SpreadingFactor:7, BandWidth:0 
I (261631) flashData: SynSlaver:0, SA:1436 
I (307325) LoRa function: receive packet from PanID:0290, SAddr:059C, DAddr:0000, CMD:20, data:7B 02 90 05 9C 00 00 03 20 04 7B 0C 00 FF DF 
I (307326) LoRaCmd_Master: HeartBeat, device saddr:059c, verion:12, rssi:255
发→◇55 AA 80 05 9C 01 # 16进制发送
I (312945) LoRa core: DAddr:059c, CMD:80 data:7B 02 90 00 00 05 9C 03 80 01 01 F3 
I (313065) LoRa function: receive packet from PanID:0290, SAddr:059C, DAddr:0000, CMD:81, data:7B 02 90 05 9C 00 00 03 81 02 FF C5 CA 
