#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include "main.h"
#include "sensors.h"

extern volatile uint8_t error_flags; // globalny licznik błędów

void check_errors();
void ErrorBlink_Task(void);

#endif
