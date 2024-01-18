#include "APP.h"

#define TAG "app"
static struct pt Lora_Rx;
static struct pt Period_1s;
static struct pt uart_process;

static int Task1_Lora_Rx(struct pt *pt);
static int Task2_1s_Period(struct pt *pt);
static int Task3_uart_process(struct pt *pt);
static void Ymodem_timeout_process(void);
/**
 * System_Run
 * @brief PTOS运行
 * @author Honokahqh
 * @date 2023-08-05
 */
void System_Run()
{
	PT_INIT(&Lora_Rx);
	PT_INIT(&Period_1s);
	PT_INIT(&uart_process);
	while (1)
	{
		Task1_Lora_Rx(&Lora_Rx);
		Task2_1s_Period(&Period_1s);
		Task3_uart_process(&uart_process);
	}
}

/**
 * Task1_Lora_Rx
 * @brief Lora中断处理、Rst按键处理
 * @author Honokahqh
 * @date 2023-08-05
 */
extern uint16_t GPIO_RST_Flag;
static int Task1_Lora_Rx(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		Lora_IRQ_Rrocess();
		PT_TIMER_DELAY(pt, 5);
	}
	PT_END(pt);
}

/**
 * Task2_1s_Period
 * @brief 1秒周期运行-lora slaver状态更新-复位检测-led闪烁-喂狗
 * @author Honokahqh
 * @date 2023-08-05
 */
static int Task2_1s_Period(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		LoRa_Period_1s();
		Ymodem_timeout_process(); // IAP通讯超时处理
		wdg_reload();			  // 喂狗
		PT_TIMER_DELAY(pt, 1000);
	}
	PT_END(pt);
}

/**
 * Task3_MBS_CMD_CTRL
 * @brief mbs主机轮询-中控外设控制-数据保存
 * @author Honokahqh
 * @date 2023-08-05
 */
static int Task3_uart_process(struct pt *pt)
{
	PT_BEGIN(pt);
	while (1)
	{
		if (uart_state.rx_buff[0] == 0x55 && uart_state.rx_buff[1] == 0xAA && uart_state.rx_len > 5)
		{
			handleSend(&uart_state.rx_buff[2], uart_state.rx_len - 2);
		}
		processATCommand((char *)uart_state.rx_buff);
		memset(uart_state.rx_buff, 0, sizeof(uart_state.rx_buff));
		uart_state.rx_len = 0;
		uart_state.has_data = 0;
		PT_WAIT_UNTIL(pt, uart_state.has_data); // 等待串口数据
	}
	PT_END(pt);
}

/**
 * Ymodem_timeout_process
 * @brief IAP通讯超时处理
 * @author Honokahqh
 * @date 2023-10-07
 */
static void Ymodem_timeout_process()
{
#if !Lora_Is_APP
	if (ymodem_session.state != YMODEM_STATE_IDLE)
	{ // 非lora模式下,OTA超时检测在此处而非Slaver_Period_1s
		uint8_t temp[2];
		ymodem_session.timeout++;
		if (ymodem_session.timeout > 3)
		{
			ymodem_session.error_count++;
			temp[0] = NAK;
			LOG_E(TAG, "OTA timeout\r\n");
			CusProfile_Send(0, OTA_Device_ACK, temp, 1, true);
		}
		if (ymodem_session.error_count > 5)
		{
			memset(&ymodem_session, 0, sizeof(ymodem_session_t));
			temp[0] = CAN;
			temp[1] = CAN;
			LOG_E(TAG, "OTA timeout, Err\r\n");
			CusProfile_Send(0x0000, OTA_Device_ACK, temp, 2, true);
			system_reset();
		}
	}
#endif
}
