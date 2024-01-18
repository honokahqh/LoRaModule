#include <stdio.h>
#include <string.h>
#include "APP.h"

#define TAG "main"
static void IAP_Detect(void);      // ����Ƿ���Ҫ����
static void State_Updata(void);    // �ϱ��汾��
static void BSTimer_Init(void);    // ��ʱ����ʼ��
static void uart_log_init(void);   // ���ڳ�ʼ��
static void watch_dog_init(void);  // ���Ź���ʼ��
static void board_init(void);      // Ӳ����ʼ��

uint32_t Sys_ms;         // ϵͳʱ��
uart_state_t uart_state; // ����״̬

int main(void)
{
    board_init();      // Ӳ����ʼ��
#if Lora_Is_APP
    delay_ms(2000); // �ȴ�Z9�������
#endif
    IAP_Check();                                 // Boot:����Ƿ���APP-�Ƿ���Ҫ���� APP:����־λ�Ƿ�����
    Lora_StateInit();                            // ״̬��ʼ	��
    delay_ms(((LoRaDevice.chip_ID) % 10) * 100); // ����ӳ�
    lora_init();                                 // lora��ʼ��
    wdg_reload();                                // ���Ź�ι��
    IAP_Detect();                                // ����Ƿ���Ҫ����
    State_Updata();                              // �ϱ��汾��
    LOG_I(TAG, "app start\r\n");
    System_Run();
}

static void IAP_Detect()
{
#if !Lora_Is_APP
    if (ymodem_session.state != YMODEM_STATE_IDLE)
    {
        uint8_t temp_data;
        temp_data = 0x01;
        /* ��Ҫ��֤APP��BOOT 4kb���� */
        for (uint8_t i = 0; i < ((FLASH_MAX_SIZE - (APP_ADDR - Boot_ADDR)) / 4096) - 3; i++)
        {
            flash_erase_page(APP_ADDR + 0x1000 * i);
        }
        flash_program_bytes(OTA_ADDR + 16, &temp_data, 1); // ���ڽ���OTA��־д1
        ymodem_session.state = YMODEM_STATE_START;
    }
#endif
}

/**
 * BSTimer_Init
 * @brief ��ʱ����ʼ��,1ms��ʱ
 * @author Honokahqh
 * @date 2023-08-05
 */
static void BSTimer_Init()
{
    bstimer_init_t bstimer_init_config;

    bstimer_init_config.bstimer_mms = BSTIMER_MMS_ENABLE;
    bstimer_init_config.period = 23;     // time period is ((1 / 2.4k) * (2399 + 1))
    bstimer_init_config.prescaler = 999; // sysclock defaults to 24M, is divided by (prescaler + 1) to 2.4k
    bstimer_init_config.autoreload_preload = true;
    bstimer_init(BSTIMER0, &bstimer_init_config);
    bstimer_config_overflow_update(BSTIMER0, ENABLE);
    bstimer_config_interrupt(BSTIMER0, ENABLE);
    bstimer_cmd(BSTIMER0, true);
    NVIC_EnableIRQ(BSTIMER0_IRQn);
    NVIC_SetPriority(BSTIMER0_IRQn, 2);
}

/**
 * BSTIMER0_IRQHandler
 * @brief ��ʱ���жϷ�����,1ms��ʱ,���ڼ�ʱ��485���ճ�ʱ�ж�,485���ճ�ʱ��,�����ݷ��͸�������
 * @author Honokahqh
 * @date 2023-08-05
 */
void BSTIMER0_IRQHandler(void)
{
    if (bstimer_get_status(BSTIMER0, BSTIMER_SR_UIF))
    {
        // UIF flag is active
        Sys_ms++;
        if (uart_state.IDLE)
        {
            uart_state.IDLE++;
            if (uart_state.IDLE >= UART_IDLE_Timeout)
            {
                uart_state.IDLE = 0;
                uart_state.has_data = true;
            }
        }
    }
}

/**
 * millis
 * @brief ptos�����,����OSϵͳ
 * @return ϵͳʱ��
 * @author Honokahqh
 * @date 2023-08-05
 */
unsigned int millis(void)
{
    return Sys_ms;
}

/**
 * UART_SendData
 * @brief ����0��������
 * @param data ����ָ��
 * @param len ���ݳ���
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART_SendData(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    for (i = 0; i < len; i++)
    {
        while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_FULL) == SET)
            ;
        UART0->DR = *data++;
    }
    while (uart_get_flag_status(UART0, UART_FLAG_TX_FIFO_EMPTY) == RESET)
        ;
}
/**
 * UART0_IRQHandler
 * @brief �������ݴ���mbs������,�����ý��ձ�־λ,��timer0����IDLE��ʱ
 * @author Honokahqh
 * @date 2023-08-05
 */
void UART0_IRQHandler(void)
{
    if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_DONE))
    {
        uart_clear_interrupt(UART0, UART_INTERRUPT_RX_DONE);
        uart_state.rx_buff[uart_state.rx_len++] = UART0->DR & 0xFF;
        uart_state.IDLE = 1;
    }
    if (uart_get_interrupt_status(UART0, UART_INTERRUPT_RX_TIMEOUT))
    {
        uart_clear_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT);
    }
}

/**
 * uart_log_init
 * @brief IO16\IO17��ʼ��ΪUART0,����ģʽ:9600��Debug:1M
 * @author Honokahqh
 * @date 2023-08-05
 */
static void uart_log_init(void)
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    gpio_set_iomux(UART_TX_PORT, UART_TX_PIN, 1); // TX
    gpio_set_iomux(UART_RX_PORT, UART_RX_PIN, 1); // RX

    /* uart config struct init */
    uart_config_t uart_config;
    uart_config_init(&uart_config);

    uart_config.baudrate = Uart_BAUD;

    uart_init(UART0, &uart_config);

    uart_config_interrupt(UART0, UART_INTERRUPT_RX_DONE, ENABLE);
    uart_config_interrupt(UART0, UART_INTERRUPT_RX_TIMEOUT, ENABLE);

    uart_cmd(UART0, ENABLE);
    NVIC_EnableIRQ(UART0_IRQn);
    NVIC_SetPriority(UART0_IRQn, 2);
}

/**
 * watch_dog_init
 * @brief ���ſڳ�ʼ��:10s
 * @author Honokahqh
 * @date 2023-08-05
 */
static void watch_dog_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_WDG, true);
    uint32_t timeout = 10000;
    uint32_t wdgclk_freq = rcc_get_clk_freq(RCC_PCLK0);
    uint32_t reload_value = timeout * (wdgclk_freq / 1000 / 2);

    // start wdg
    wdg_start(reload_value);
    NVIC_EnableIRQ(WDG_IRQn);
}

void LoraReInit()
{
    /* ��λLoraģ�� */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
    delay_ms(10);
    lora_init(); // lora��ʼ��
}
/**
 * board_init
 * @brief Ӳ����ʼ��
 * @author Honokahqh
 * @date 2023-08-05
 */
static void board_init()
{
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_AFEC, true);

    // enable the clk
    rcc_enable_oscillator(RCC_OSC_XO32K, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER1, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER0, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_TIMER1, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOC, true);
      rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    delay_ms(100);

    watch_dog_init();
    pwr_xo32k_lpm_cmd(true);
    uart_log_init();
    BSTimer_Init();
    /* ��λLoraģ�� */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
}

// �ϱ��汾��
static void State_Updata()
{
    if (LoRaDevice.Net_State == Net_JoinGateWay && ymodem_session.state == YMODEM_STATE_IDLE)
    {
        PCmd_HeartBeat();
    }
}
#ifdef USE_FULL_ASSERT
void assert_failed(void *file, uint32_t line)
{
    (void)file;
    (void)line;

    while (1)
    {
    }
}
#endif
