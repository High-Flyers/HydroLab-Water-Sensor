# STM32 EC & Temperature Measurement System

## Opis projektu
Prosty system pomiarowy oparty na STM32:

- Pomiar temperatury (°C), przewodności EC (µS/cm) oraz pH w roztworach.  
- Kalibracja EC i pH przy użyciu przycisków.  
- Wskaźnik błędów i stanów systemu przez LED.  
- Komunikacja z komputerem przez UART.  

System zapisuje dane kalibracyjne w pamięci Flash, dzięki czemu nie są tracone po restarcie.

---

## Ramka UART
Przykładowa ramka wysyłana co 1 sekundę:  
MEAS,T=24.34,EC=1423.06,pH=4.21

- `T` – temperatura w °C  
- `EC` – przewodność w µS/cm  
- `pH` – pH roztworu  

---
## Kalibracja

### Kalibracja EC
Przytrzymanie przycisku `PB12` przelicza nową wartość `k_val` na podstawie referencyjnej przewodności EC w funkcji temperatury.  
1. Klik → wejście w tryb kalibracji
2. Klik → przeliczenie stałej `k_val` i zapisanie jej do pamięci flash.

### Kalibracja pH
Kalibracja odbywa się za pomocą przycisku `PB13` jako 3-etapowy proces wykonany najpierw dla próbki 7 pH, a następnie 4 pH:

1. Klik → wejście w tryb kalibracji  
2. Klik → odczyt napięcia i rozpoznanie bufora:
   - ~1500 mV → pH 7 (`neutralVoltage`)
   - ~2000 mV → pH 4 (`acidVoltage`)
3. Klik → zapis danych do Flash  

---

## LED i błędy
- LED na PC13 sygnalizuje błędy.  
- Flagi błędów (`error_flags`):
  - Bit 0 – temperatura poza zakresem (0°C … 50°C)  
  - Bit 1 – EC poza zakresem (≤300 lub >20000 µS/cm)  
  - Bit 2 – pH poza zakresem (0 < pH ≤ 14)  
- LED mruga co 100 ms, po sekwencji 2–3 s przerwy.  

---

## Pamięć Flash
Dane kalibracyjne są zapisane na pamięć flash, aby zachować je po restarcie urządzenia. Dane do sondy przewodności znajdują się na innym sektorze flash (1kB) niż dane z sondy pH.
| Parametr | Adres |
|---------|------|
| K (EC) | 0x0800FC00 |
| pH neutral | 0x0800F800 |
| pH acid | 0x0800F804 |

---

## Schemat pinów STM32

| Pin MCU | Nazwa/Opis   | Funkcja w projekcie        |
|--------|-------------|----------------------------|
| PC13   | LED         | Wskaźnik błędów / status   |
| PB12   | EC_cal_Pin  | Kalibracja EC              |
| PB13    | pH_cal_Pin  | Kalibracja pH              |
| PA0    | ADC_CHANNEL_0 | Czujnik temperatury     |
| PA1    | ADC_CHANNEL_1 | Czujnik EC              |
| PA2    | ADC_CHANNEL_2 | Czujnik pH              |
| PA9    | USART1_TX   | UART – transmisja    |
| PA10   | USART1_RX   | UART – odbiór              |

---

## Podłączenie
1. Czujnik temperatury → PA0  
2. Czujnik EC → PA1  
3. Czujnik pH → PA2  
4. Przycisk EC → PB12 do GND  
5. Przycisk pH → PB13 do GND  
6. UART → PA9 (TX), PA10 (RX)
   
---

## Uwagi dotyczące sondy pH 
- Nie dopuścić do wyschnięcia sondy  
- Przechowywać w:
  - roztworze KCl  
  - lub buforze pH 4
  - lub wodzie z niewielką ilością soli (awaryjnie)  
- Nie dotykać szklanej końcówki  
- Utrzymywać złącze BNC suche i czyste  
