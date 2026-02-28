# HydroLab Water Sensor

## Opis projektu
Prosty system pomiarowy oparty na STM32:

- Pomiar temperatury (°C) i przewodności EC (µS/cm) w roztworach.  
- Kalibracja EC przy użyciu przycisku.  
- Wskaźnik błędów i stanów systemu przez LED.  
- Komunikacja przez UART.  

**Uwaga:** pH jeszcze nie jest implementowane (wartość zawsze 0.0).

---

## Ramka UART
Przykładowa ramka wysyłana co 1 sekundę:  
MEAS,T=24.34,EC=1423.06,pH=0.00
- `T` – temperatura w °C  
- `EC` – przewodność w µS/cm  
- `pH` – obecnie zawsze 0.00  

---

## Kalibracja EC
- Przytrzymanie przycisku PB12 przelicza nową wartość `k_val` na podstawie referencyjnej przewodności EC w funkcji temperatury.  
- Nowa wartość jest zapisywana w pamięci Flash.  

---

## LED i błędy
- LED na PC13 sygnalizuje błędy.  
- Flagi błędów (`error_flags`):
  - Bit 0 – temperatura poza zakresem (0°C … 150°C)  
  - Bit 1 – EC poza zakresem (≤300 lub >20000 µS/cm)  
  - Bit 2 – pH poza zakresem (0 ≤ pH ≤ 14)  
- LED mruga co 1 s, po sekwencji 2–3 s przerwy.  

---

## Schemat pinów STM32

| Pin MCU | Nazwa/Opis                | Funkcja w projekcie                |
|---------|---------------------------|------------------------------------|
| PC13    | LED                       | Wskaźnik błędów                    |
| PB12    | Przyciski kalibracyjne    | Kalibracja EC (przycisk)           |
| PA0     | ADC_CHANNEL_0             | Czujnik temperatury                |
| PA1     | ADC_CHANNEL_1             | Czujnik EC                         |
| PA2     | ADC_CHANNEL_2             | Czujnik pH                         |
| PA9     | USART1_TX                 | UART – transmisja                  |
| PA10    | USART1_RX                 | UART – odbiór                      |

**Uwaga:** pH nie jest jeszcze podłączone.

---

## Podłączenie
1. Czujnik temperatury → ADC0 (PA0)  
2. Czujnik EC → ADC1 (PA1)
3. Czujnik pH → ADC2 (PA2)  
4. Przycisk kalibracyjny → PB12 do GND (pullup do 3V3)
5. LED status → PC13 (wbudowany LED)  
6. UART → PA9 (TX), PA10 (RX)
