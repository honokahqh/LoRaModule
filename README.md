# LoRa-RoomCtrl
# 控制范围:
# 3路12~24v直流调光
# 4路可控硅调光
# 22路继电器
<!-- ASR6601 Lora中控
v1.0.0  by Honokahqh    2023.8.5
1.modbus slaver 用于中控寄存器控制,目前支持10个继电器+3路PWM调光
2.modbus master 用于读取传感器数据,目前支持红外遥控、蓝牙、房态灯、人体红外1、人体红外2、外部开关
3.OTA升级 ymodem协议 单包128字节
    3.1 支持动态速率 根据信号质量切换带宽-主机提供
4.flash存储 每页4KB,合计三页 page1:0x0800D000,存储网络信息 page2:0x0800E000,存储中控mbs信息 page3:0x0800F000,存储OTA状态
5.设备版本号Dev_Version -->


<!-- v1.0.1 by Honokahqh
新增大中控支持
新增LoRa/485通讯切换 -->

<!-- v1.0.2 by Honokahqh 2023.12.8
新增蓝牙模块支持
新增中控红外模块支持 400ms轮询一次红外遥控数据 10s轮询一次温度数据
心跳新增来自中控红外模块的温度数据
修复bug--在boot状态下心跳版本号不正确 --># LoRaModule
