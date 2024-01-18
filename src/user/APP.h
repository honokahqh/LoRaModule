	#ifndef __APP_H
#define __APP_H

#include <stdio.h>

#include "lora_core.h"
#include "delay.h"
#include "timer.h"
#include "radio.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "tremo_delay.h"
#include "tremo_bstimer.h"
#include "tremo_system.h"
#include "tremo_flash.h"
#include "tremo_adc.h"
#include "tremo_timer.h"
#include "tremo_lptimer.h"
#include "tremo_regs.h"
#include "tremo_wdg.h"
#include "tremo_dac.h"
#include "pt.h"
#include "log.h"
// ���п� 4���� + 4�ɿع� + 10�̵��� + 8 595�̵��� + 3 595LED + 3PWM
// С�п� 10�̵��� + 3 PWM
#define ROOM_PLUS	1
#define ROOM_MINI	0

#define Dev_Version     	100

#define UART_RX_PORT 		GPIOB	
#define UART_RX_PIN 		GPIO_PIN_0
#define UART_TX_PORT		GPIOB
#define UART_TX_PIN 		GPIO_PIN_1

#define Uart_BAUD 1000000

/* uart״̬ */
#define UART_IDLE_Timeout 5
typedef struct
{
	uint8_t IDLE;	  // ���ڿ���-0:����
	uint8_t has_data; // ����һ֡���ݽ������

	uint8_t rx_buff[256];
	uint8_t rx_len;
} uart_state_t;
extern uart_state_t uart_state;

void System_Run(void);
void lora_init(void);
void Lora_IRQ_Rrocess(void);

void Flash_Data_Syn();
void LoraState_Save();
void UART_SendData(const uint8_t *data, uint16_t len);

void LoraReInit();

extern uint32_t Sys_ms;

#endif
