#include "error_handler.h"

volatile uint8_t error_flags = 0;

// Sprawdzenie errorow i ewentualne ustawienie flagi errora
void check_errors(){
	  // ustawianie error_flags po pomiarach
	  error_flags = 0;
	  if(temp <= 0 || temp > 50) error_flags |= 0x01;
	  if(ec <= 300 || ec > 20000) error_flags |= 0x02;
	  if(ph <= 0 || ph > 14) error_flags |= 0x04;
}

// Task do mrugania dioda w przypadku errorow
void ErrorBlink_Task(void){
    static uint8_t state = 0;         // aktualny blink w sekwencji
    static uint32_t last_tick = 0;
    static uint32_t pause_until = 0;  // moment do rozpoczęcia nowej sekwencji

    uint8_t blinks = 0;
    if(error_flags & 0x01) blinks += 1; // temp
    if(error_flags & 0x02) blinks += 2; // EC
    if(error_flags & 0x04) blinks += 4; // pH

    uint32_t now = HAL_GetTick();

    // jeśli jesteśmy w przerwie, nic nie robimy
    if(now < pause_until){
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
        return;
    }

    if(blinks == 0){
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // wyłącz LED
        state = 0;  // reset sekwencji
        return;
    }

    // szybkie mruganie co 100ms
    if(now - last_tick >= 100){
        last_tick = now;

        if(state < blinks*2){ // *2 bo on+off
            if(state % 2 == 0)
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            else
                HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            state++;
        } else {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // wyłącz LED po sekwencji
            state = 0;
            pause_until = now + 2500; // pauza 2,5 s przed kolejną sekwencją
        }
    }
}
