#include <stdio.h>
#include <string.h>
#include "APP.h"

#define TAG "main"
static void IAP_Detect(void);      // 检测是否需要升级
static void State_Updata(void);    // 上报版本号
static void BSTimer_Init(void);    // 定时器初始化
static void uart_log_init(void);   // 串口初始化
static void watch_dog_init(void);  // 看门狗初始化
static void board_init(void);      // 硬件初始化

uint32_t Sys_ms;         // 系统时间
uart_state_t uart_state; // 串口状态

int main(void)
{
    board_init();      // 硬件初始化
#if Lora_Is_APP
    delay_ms(2000); // 等待Z9启动完成
#endif
    IAP_Check();                                 // Boot:检查是否有APP-是否需要升级 APP:检测标志位是否正常
    Lora_StateInit();                            // 状态初始	化
    delay_ms(((LoRaDevice.chip_ID) % 10) * 100); // 随机延迟
    lora_init();                                 // lora初始化
    wdg_reload();                                // 看门狗喂狗
    IAP_Detect();                                // 检测是否需要升级
    State_Updata();                              // 上报版本号
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
        /* 需要保证APP和BOOT 4kb对齐 */
        for (uint8_t i = 0; i < ((FLASH_MAX_SIZE - (APP_ADDR - Boot_ADDR)) / 4096) - 3; i++)
        {
            flash_erase_page(APP_ADDR + 0x1000 * i);
        }
        flash_program_bytes(OTA_ADDR + 16, &temp_data, 1); // 正在进行OTA标志写1
        ymodem_session.state = YMODEM_STATE_START;
    }
#endif
}

/**
 * BSTimer_Init
 * @brief 定时器初始化,1ms定时
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
 * @brief 定时器中断服务函数,1ms定时,用于计时和485接收超时判断,485接收超时后,将数据发送给处理函数
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
 * @brief ptos会调用,用于OS系统
 * @return 系统时间
 * @author Honokahqh
 * @date 2023-08-05
 */
unsigned int millis(void)
{
    return Sys_ms;
}

/**
 * UART_SendData
 * @brief 串口0发送数据
 * @param data 数据指针
 * @param len 数据长度
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
 * @brief 接收数据传入mbs缓冲区,并设置接收标志位,由timer0进行IDLE计时
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
 * @brief IO16\IO17初始化为UART0,工作模式:9600、Debug:1M
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
 * @brief 看门口初始化:10s
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
    /* 复位Lora模块 */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
    delay_ms(10);
    lora_init(); // lora初始化
}
/**
 * board_init
 * @brief 硬件初始化
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
    /* 复位Lora模块 */
    (*(volatile uint32_t *)(0x40000000 + 0x18)) &= ~RCC_RST0_LORA_RST_N_MASK;
    delay_ms(100);
    (*(volatile uint32_t *)(0x40000000 + 0x18)) |= RCC_RST0_LORA_RST_N_MASK;
}

// 上报版本号
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
