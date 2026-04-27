#include "uart_cmd.h"
#include <string.h>

#define RX_BUF_SIZE 32

static UART_HandleTypeDef *_huart;
static uint8_t rx_byte;
static char rx_buf[RX_BUF_SIZE];
static uint8_t rx_idx = 0;
static uint8_t cmd_ready = 0;
static char cmd_buf[RX_BUF_SIZE];

void UartCmd_Init(UART_HandleTypeDef *huart)
{
    _huart = huart;
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
    else
    {
        send_response("error\r\n");
    }
}

// Akcje
// Nabierz wode do zbiornika
void UartCmd_OnTake(void)
{
    UartCmd_SendReady();
}

// Wypusc wode ze zbiornika
void UartCmd_OnRelease(void)
{
    UartCmd_SendReady();
}

//Callbacks
void UartCmd_SendReady(void)  { send_response("ready\r\n"); }
void UartCmd_SendError(void)  { send_response("error\r\n"); }
