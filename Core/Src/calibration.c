#include "calibration.h"

float k_val = 1.0;
float acidVoltage = 2032.44;
float neutralVoltage = 1500.0;

static uint8_t ec_cal_state = 0;  // 0=IDLE, 1=ENTER
static PhCalState ph_cal_state = PH_CAL_IDLE;
static uint8_t    ph_cal_finish = 0;

// Tablica referencyjnych wartosci EC probki do kalibracji w zaleznosci od temperatury
float EC_reference_temperature(float temp){
    float T[] = {0,5,10,15,20,23,24,25,26,30};
    float EC[] = {776,896,1020,1147,1278,1359,1386,1413,1440,1548};
    int i;
    // Poniewaz mamy tylko wartosci co 5 stopni robimy interpolacje dla innych temeparatur
    for(i=0;i<9;i++){
        if(temp >= T[i] && temp <= T[i+1]){
            // liniowa interpolacja
            float k = (temp - T[i]) / (T[i+1]-T[i]);
            return EC[i] + k*(EC[i+1]-EC[i]);
        }
    }
    if(temp < T[0]) return EC[0];
    if(temp > T[9]) return EC[9];
    return EC[9]; // safety fallback
}

// Kalibracja stalej K do odczytu EC i zapis jej na FLASHu
// Pomocnicza: wykrywa wciśnięcie przycisku EC (analogicznie do pH)
uint8_t EC_button_pressed(void)
{
    static uint8_t  last_btn     = 1;
    static uint8_t  btn_handled  = 0;
    static uint32_t debounce_tick = 0;

    uint8_t btn = HAL_GPIO_ReadPin(GPIOB, EC_cal_Pin);
    uint32_t now = HAL_GetTick();

    if (btn != last_btn) {
        debounce_tick = now;
    }
    last_btn = btn;

    if ((now - debounce_tick) < 50) return 0;

    if (btn == 0 && !btn_handled) {
        btn_handled = 1;
        return 1;
    }
    if (btn == 1) {
        btn_handled = 0;
    }
    return 0;
}

// State machine kalibracji EC
void EC_calibration_task(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart)
{
    if (!EC_button_pressed()) return;

    char msg[64];

    switch (ec_cal_state)
    {
        // --- IDLE: pierwsze kliknięcie = wejdź w tryb, włóż do buforu ---
        case 0:
            ec_cal_state = 1;
            snprintf(msg, sizeof(msg), "CAL_EC: Enter mode. Put probe in 1413 uS/cm buffer.\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            break;

        // --- ENTER: drugie kliknięcie = oblicz K i zapisz ---
        case 1:
        {
            float ref_ec = EC_reference_temperature(temp);
            uint32_t adc_val = Read_ADC_Average(hadc, ADC_CHANNEL_1, 32);
            float voltage = adc_val * 3.3f / 4096.0f;

            float k_new = RES2 * ECREF * ref_ec / 100000.0f / voltage / 1000.0f;

            if (k_new >= 0.5f && k_new <= 1.5f) {
                k_val = k_new;
                Flash_Write_K(k_val);
                snprintf(msg, sizeof(msg), "CAL_EC: K saved: %.4f. Exit.\r\n", k_val);
            } else {
                snprintf(msg, sizeof(msg), "CAL_EC: ERROR - K out of range (%.4f). Exit.\r\n", k_new);
            }

            HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            ec_cal_state = 0;  // wróć do IDLE
            break;
        }

        default:
            ec_cal_state = 0;
            break;
    }
}

// Pomocnicza: wykrywa pojedyncze wciśnięcie przycisku (zbocze + debounce)
uint8_t pH_button_pressed(void)
{
    static uint8_t  last_btn     = 1;
    static uint8_t  btn_handled  = 0;
    static uint32_t debounce_tick = 0;

    uint8_t btn = HAL_GPIO_ReadPin(GPIOB, pH_cal_Pin); // pull-up: wciśnięty = 0
    uint32_t now = HAL_GetTick();

    if (btn != last_btn) {
        debounce_tick = now;   // reset debounce przy każdej zmianie
    }
    last_btn = btn;

    if ((now - debounce_tick) < 50) return 0;  // jeszcze bouncing

    if (btn == 0 && !btn_handled) {
        btn_handled = 1;
        return 1;   // świeże wciśnięcie!
    }
    if (btn == 1) {
        btn_handled = 0;  // zwolniony, gotowy na następny
    }
    return 0;
}

// Główna state machine kalibracji pH
void pH_calibration_task(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart)
{
    if (!pH_button_pressed()) return;  // nic do roboty

    char msg[64];

    switch (ph_cal_state)
    {
        // --- IDLE: pierwsze kliknięcie wchodzi w tryb kalibracji ---
        case PH_CAL_IDLE:
            ph_cal_finish = 0;
            ph_cal_state  = PH_CAL_ENTER;
            snprintf(msg, sizeof(msg), "CAL_PH: Enter mode. Put probe in 4.0 or 7.0 buffer.\r\n");
            HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            break;

        // --- ENTER: drugie kliknięcie = sprawdź napięcie i zapamiętaj ---
        case PH_CAL_ENTER:
        {
            // Odczyt aktualnego napięcia pH (mV)
            uint32_t adc_val = Read_ADC_Average(hadc, ADC_CHANNEL_2, 32);
            float voltage_mv = adc_val * 3300.0f / 4096.0f;  // w mV

            if (voltage_mv > 1322.0f && voltage_mv < 1678.0f) {
                neutralVoltage = voltage_mv;
                ph_cal_finish  = 1;
                snprintf(msg, sizeof(msg), "CAL_PH: Buffer 7.0 OK (%.1f mV). Click to save.\r\n", voltage_mv);
            } else if (voltage_mv > 1854.0f && voltage_mv < 2210.0f) {
                acidVoltage   = voltage_mv;
                ph_cal_finish = 1;
                snprintf(msg, sizeof(msg), "CAL_PH: Buffer 4.0 OK (%.1f mV). Click to save.\r\n", voltage_mv);
            } else {
                ph_cal_finish = 0;
                snprintf(msg, sizeof(msg), "CAL_PH: ERROR - bad voltage (%.1f mV). Try again.\r\n", voltage_mv);
            }

            HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            ph_cal_state = PH_CAL_CAL;
            break;
        }

        // --- CAL: trzecie kliknięcie = zapisz lub zgłoś błąd ---
        case PH_CAL_CAL:
            if (ph_cal_finish) {
            	Flash_Write_PH(neutralVoltage, acidVoltage);
                snprintf(msg, sizeof(msg), "CAL_PH: Calibration saved! Exit.\r\n");
            } else {
                snprintf(msg, sizeof(msg), "CAL_PH: Calibration FAILED. Exit.\r\n");
            }
            HAL_UART_Transmit(huart, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
            ph_cal_state = PH_CAL_IDLE;  // wróć do IDLE
            ph_cal_finish = 0;
            break;

        default:
            ph_cal_state = PH_CAL_IDLE;
            break;
    }
}
