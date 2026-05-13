#include "uart_cmd.h"
#include <string.h>

#define RX_BUF_SIZE 32
#define MOTOR_TIMEOUT_MS 10000

static UART_HandleTypeDef *_huart;
TIM_HandleTypeDef *_htim;

static uint8_t rx_byte;
static char rx_buf[RX_BUF_SIZE];
static uint8_t rx_idx = 0;
static uint8_t cmd_ready = 0;
static char cmd_buf[RX_BUF_SIZE];
static uint8_t meas_enabled = 1;

void UartCmd_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim)
{
	// Przypisz instancje
    _huart = huart;
    _htim = htim;
    // Uruchom odbiór bajt po bajcie przez przerwanie
    HAL_UART_Receive_IT(_huart, &rx_byte, 1);
}

// Wołane z HAL_UART_RxCpltCallback
void UartCmd_RxCallback(void)
{
    if (rx_byte == '\n' || rx_byte == '\r')
    {
        if (rx_idx > 0)
        {
            rx_buf[rx_idx] = '\0';
            memcpy(cmd_buf, rx_buf, rx_idx + 1);
            rx_idx = 0;
            cmd_ready = 1;
        }
    }
    else
    {
        if (rx_idx < RX_BUF_SIZE - 1)
            rx_buf[rx_idx++] = (char)rx_byte;
        else
            rx_idx = 0; // przepełnienie - reset
    }

    // Ponów nasłuchiwanie
    HAL_UART_Receive_IT(_huart, &rx_byte, 1);
}

// Prywatna f, do wysylania po uart
static void send_response(const char *resp)
{
    HAL_UART_Transmit(_huart, (uint8_t *)resp, strlen(resp), 100);
}

// Odczytuj komendy po uart
void UartCmd_Task(void)
{
    if (!cmd_ready) return;
    cmd_ready = 0;

    if (strcmp(cmd_buf, "take") == 0)
    {
        UartCmd_OnTake();
    }
    else if (strcmp(cmd_buf, "release") == 0)
    {
        UartCmd_OnRelease();
    }
    else if (strcmp(cmd_buf, "reset") == 0)
    {
        UartCmd_OnReset();
    }
    else if (strncmp(cmd_buf, "meas", 4) == 0)
    {
    	UartCmd_OnMeas();
    }
    else
    {
        send_response("error\r\n");
    }
}

// Akcje
// Nabierz wode do zbiornika
void UartCmd_OnTake(void)
{
    // Zbocze 0->1 na MOTOR_CMD
	HAL_GPIO_WritePin(MOTOR_CMD_GPIO_Port, MOTOR_CMD_Pin, GPIO_PIN_RESET);
	HAL_Delay (10);
    HAL_GPIO_WritePin(MOTOR_CMD_GPIO_Port, MOTOR_CMD_Pin, GPIO_PIN_SET);
    send_response("Taking water...\r\n");

    // Czekaj na zbocze 0->1 na MOTOR_FB
    uint32_t start = HAL_GetTick();
    while (HAL_GPIO_ReadPin(MOTOR_FB_GPIO_Port, MOTOR_FB_Pin) == GPIO_PIN_RESET)
    {
        if (HAL_GetTick() - start >= MOTOR_TIMEOUT_MS)
        {
            send_response("error\r\n");
            return;
        }
    }

    send_response("ready\r\n");
}

// Wypusc wode ze zbiornika
void UartCmd_OnRelease(void)
{
	__HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_2, 2000);
	HAL_Delay(5000);
	__HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_2, 1000);

    send_response("ready\r\n");
}

// reset stma
void UartCmd_OnReset(void)
{
	send_response("Resetting device...\r\n");
	HAL_Delay(10);
	NVIC_SystemReset();
}

// Pomiary on/off
void UartCmd_OnMeas(void)
{
	if (strcmp(cmd_buf, "meas on") == 0)
	{
	    meas_enabled = 1;
	    send_response("Measurements: ON\r\n");
	}
	else if (strcmp(cmd_buf, "meas off") == 0)
	{
	    meas_enabled = 0;
	    send_response("Measurements: OFF\r\n");
	}
	else
	{
	    send_response("error\r\n");
	}
}

uint8_t UartCmd_MeasEnabled(void) { return meas_enabled; }

//Callbacks
void UartCmd_SendReady(void)  { send_response("ready\r\n"); }
void UartCmd_SendError(void)  { send_response("error\r\n"); }
