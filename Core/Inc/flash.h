#ifndef FLASH_H
#define FLASH_H

#include "main.h"
#include <stdio.h>

#define FLASH_PH_ADDR  0x0800F800
#define FLASH_K_ADDR 0x0800FC00

void Flash_Write_K(float k);
void Flash_Read_K(float *out, UART_HandleTypeDef *huart);

void Flash_Write_PH(float neutral, float acid);
void Flash_Read_PH(float *neutral_out, float *acid_out, UART_HandleTypeDef *huart);

#endif
