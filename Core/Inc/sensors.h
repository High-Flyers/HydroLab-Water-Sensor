#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"
#include "calibration.h"
#include "error_handler.h"

#define RES2 820.0
#define ECREF 200.0
#define GDIFF (30/1.8)
#define VR0  0.223
#define G0  2
#define I  (1.24 / 10000)

// Wartosci do odczytu (moze struktura?)
extern float temp;
extern float ec;
extern float ph;

uint32_t Read_ADC_Average(ADC_HandleTypeDef *hadc, uint32_t channel, uint16_t samples);

float get_temperature(ADC_HandleTypeDef *hadc);
float get_EC(ADC_HandleTypeDef *hadc);
float get_pH(ADC_HandleTypeDef *hadc);

void send_measurements(UART_HandleTypeDef *huart);

#endif
