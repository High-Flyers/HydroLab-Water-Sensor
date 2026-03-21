#include "sensors.h"

float temp = 0;
float ec = 0;
float ph = 0;

// Usredniony odczyt z ADC (kanal, liczba probek do usrednienia)
uint32_t Read_ADC_Average(ADC_HandleTypeDef *hadc, uint32_t channel, uint16_t samples)
{
	// Konfiguracja kanalu
    ADC_ChannelConfTypeDef sConfigLocal = {0};
    sConfigLocal.Channel = channel;
    sConfigLocal.Rank = ADC_REGULAR_RANK_1;
    sConfigLocal.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

    HAL_ADC_ConfigChannel(hadc, &sConfigLocal);

    uint32_t sum = 0;

    // Odczyt "n" razy i wyciagniecie sredniej
    for(uint16_t i = 0; i < samples; i++)
    {
        HAL_ADC_Start(hadc);
        HAL_ADC_PollForConversion(hadc, HAL_MAX_DELAY);
        sum += HAL_ADC_GetValue(hadc);
        HAL_ADC_Stop(hadc);
    }

    return sum / samples;
}

// Pobierz temperature z kanalu 0
float get_temperature(ADC_HandleTypeDef *hadc)
{
	uint32_t adc0_val = Read_ADC_Average(hadc, ADC_CHANNEL_0, 32);//HAL_ADC_GetValue(&hadc1);

	float voltage = adc0_val * 3.3 / 4096.0;
	float Rpt1000 = (voltage/GDIFF+VR0)/I/G0;
	return (Rpt1000-1000)/3.85;
}

// pobierz EC z kanalu 1
float get_EC(ADC_HandleTypeDef *hadc)
{
	uint32_t adc1_val = Read_ADC_Average(hadc, ADC_CHANNEL_1, 32); //HAL_ADC_GetValue(&hadc1);

	float voltage = adc1_val * 3.3 / 4096.0;
	float ecvalueRaw = 1000 * 100000 * voltage / RES2 / ECREF * k_val;
	// w razie jakby nie bylo temperatury
	if (!(error_flags & 0x01))
		return ecvalueRaw / (1.0 + 0.02 * (temp - 25.0));
	else
		return ecvalueRaw;
}

float get_pH(ADC_HandleTypeDef *hadc)
{
	uint32_t adc2_val = Read_ADC_Average(hadc, ADC_CHANNEL_2, 32); //HAL_ADC_GetValue(&hadc1);

	float voltage_mv = adc2_val * 3300.0f / 4096.0f;  // w mV

	// Korekcja Nernsta względem temperatury (domyślnie 25°C jeśli błąd temp)
	float t = (error_flags & 0x01) ? 25.0f : temp;
	float nernst = (0.05916f * (273.15f + t) / 298.15f) * 1000.0f;  // mV/pH

	float slope     = (7.0f - 4.0f) / ((neutralVoltage - 1500.0f) / nernst
	                                      - (acidVoltage    - 1500.0f) / nernst);
	float intercept = 7.0f - slope * (neutralVoltage - 1500.0f) / nernst;

	return slope * (voltage_mv - 1500.0f) / nernst + intercept;
}

// Przesylanie pomiarow po UART
void send_measurements(UART_HandleTypeDef *huart){
	char uart_frame[64];
	snprintf(uart_frame, sizeof(uart_frame),
		     "MEAS,T=%.2f,EC=%.2f,pH=%.2f\r\n",
		      temp, ec, ph);
	// Transmit UART frame
	HAL_UART_Transmit(huart,
		              (uint8_t*)uart_frame,
		              strlen(uart_frame),
		              HAL_MAX_DELAY);
}
