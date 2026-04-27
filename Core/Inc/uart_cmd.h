#ifndef INC_UART_CMD_H_
#define INC_UART_CMD_H_

#include "main.h"

void UartCmd_Init(UART_HandleTypeDef *huart);
void UartCmd_Task(void);

// Komendy
void UartCmd_SendReady(void);
void UartCmd_SendError(void);

// Callbacki
void UartCmd_RxCallback(void);
void UartCmd_OnTake(void);
void UartCmd_OnRelease(void);

#endif
