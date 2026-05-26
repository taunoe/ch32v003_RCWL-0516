/*
 *Hardware connection:
 *  PD5 -- Rx
 *  PD6 -- Tx
 *    -- LED
 *
 */

#include "debug.h"


/* Global define */
#define RADAR_PORT GPIOD
#define RADAR_PIN  GPIO_Pin_4
#define RADAR_RCC  RCC_APB2Periph_GPIOD

#define LED_PORT   GPIOC
#define LED_PIN    GPIO_Pin_0
#define LED_RCC    RCC_APB2Periph_GPIOC

void radar_init(void)
{
    RCC_APB2PeriphClockCmd(RADAR_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = RADAR_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING; // RCWL drives the line, no pull needed
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RADAR_PORT, &GPIO_InitStructure);
}

uint8_t radar_motion_detected(void)
{
    return GPIO_ReadInputDataBit(RADAR_PORT, RADAR_PIN) == Bit_SET;
}

void led_init(void)
{
    RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = LED_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;  // Push-Pull output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
}

void led_toggle(void)
{
    GPIO_WriteBit(LED_PORT, LED_PIN,
    GPIO_ReadOutputDataBit(LED_PORT, LED_PIN) ? Bit_RESET : Bit_SET);
}

/*********************************************************************
 * @fn      USARTx_CFG
 *
 * @brief   Initializes the USART2 & USART3 peripheral.
 *
 * @return  none
 */
void USARTx_CFG(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);
}

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */
int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    //USARTx_CFG();
    radar_init();
    led_init();
    led_toggle();

    printf("SystemClk: %d\r\n", SystemCoreClock);
    printf("ChipID: %08x\r\n", DBGMCU_GetCHIPID());
   
    while(1)
    {
        if (radar_motion_detected())
        {
            GPIO_WriteBit(LED_PORT, LED_PIN, Bit_SET);   // LED on
            printf("+\r\n");
        }
        else
        {
            GPIO_WriteBit(LED_PORT, LED_PIN, Bit_RESET);  // LED off
            printf("-\r\n");
        }
        Delay_Ms(200); // poll every 50 ms, sensor output holds ~2s by default
    }
}
