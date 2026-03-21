#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "main.h"
#include "sensors.h"
#include "flash.h"

// stala K i referencja - EC
extern float k_val;
// stale dla pH
extern float acidVoltage;
extern float neutralVoltage;

typedef enum {
    PH_CAL_IDLE = 0,
    PH_CAL_ENTER,
    PH_CAL_CAL,
    PH_CAL_EXIT
} PhCalState;

float EC_reference_temperature(float temp);
uint8_t EC_button_pressed(void);
void EC_calibration_task(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart);

void pH_calibration_task(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart);
uint8_t pH_button_pressed(void);

#endif
