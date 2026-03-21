#include "flash.h"

// Zapis na pamiec flash wartosci K do liczenia EC
void Flash_Write_K(float k)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError;

    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.PageAddress = FLASH_K_ADDR;
    eraseInit.NbPages = 1;

    HAL_FLASHEx_Erase(&eraseInit, &pageError);

    uint32_t data;
    memcpy(&data, &k, sizeof(float));

    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_K_ADDR, data);

    HAL_FLASH_Lock();
}

// Odczyt wartosci K przy starcie urzadzenia
void Flash_Read_K(float *out, UART_HandleTypeDef *huart)
{
    uint32_t data = *(uint32_t*)FLASH_K_ADDR;
    float k;
    memcpy(&k, &data, sizeof(float));

    if (k < 0.5f || k > 1.5f)
        k = 1.0f;

    *out = k;

    // Wyświetlenie odczytanej wartości K przy starcie
    char uart_msg[64];
    snprintf(uart_msg, sizeof(uart_msg), "K value at startup: %.4f\r\n", *out);
    HAL_UART_Transmit(huart, (uint8_t*)uart_msg, strlen(uart_msg), HAL_MAX_DELAY);
}

// Zapis neutral i acid voltage do flasha
void Flash_Write_PH(float neutral, float acid)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError;

    eraseInit.TypeErase   = FLASH_TYPEERASE_PAGES;
    eraseInit.PageAddress = FLASH_PH_ADDR;
    eraseInit.NbPages     = 1;
    HAL_FLASHEx_Erase(&eraseInit, &pageError);

    uint32_t data;

    memcpy(&data, &neutral, sizeof(float));
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_PH_ADDR,     data);

    memcpy(&data, &acid, sizeof(float));
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, FLASH_PH_ADDR + 4, data);

    HAL_FLASH_Lock();
}

// Odczyt na starcie wartosci neutral i acid volt
void Flash_Read_PH(float *neutral_out, float *acid_out, UART_HandleTypeDef *huart)
{
    uint32_t data;
    float val;

    data = *(uint32_t*)FLASH_PH_ADDR;
    memcpy(&val, &data, sizeof(float));
    if (val >= 1322.0f && val <= 1678.0f)
    	*neutral_out = val;
    else
    	*neutral_out = 1500.0f;  // domyślna

    data = *(uint32_t*)(FLASH_PH_ADDR + 4);
    memcpy(&val, &data, sizeof(float));
    if (val >= 1854.0f && val <= 2210.0f)
    	*acid_out = val;
    else
    	*acid_out = 2032.44f;  // domyślna

    // Wyświetlenie odczytanej wartości przy starcie
    char uart_msg[64];
    snprintf(uart_msg, sizeof(uart_msg), "neutral val: %.4f acid val: %.4f\r\n", *neutral_out, *acid_out);
    HAL_UART_Transmit(huart, (uint8_t*)uart_msg, strlen(uart_msg), HAL_MAX_DELAY);
}
