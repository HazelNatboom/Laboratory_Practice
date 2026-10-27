#include <stdint.h>
#include "init.h"
#include "main.h"

void System_Init(void) {
    // 1. Включаем тактирование GPIOB
    RCC_AHB1ENR |= RCC_GPIOB_EN;

    // 2. Сбрасываем и настраиваем MODER (PB2, PB5 - входы; PB6, PB8, PB9 - выходы)
    GPIOB_MODER &= ~(MODER_PB2_MASK | MODER_PB5_MASK | MODER_PB6_MASK | MODER_PB8_MASK | MODER_PB9_MASK);
    GPIOB_MODER |=  (MODER_PB6_OUT  | MODER_PB8_OUT  | MODER_PB9_OUT);

    // 3. Включаем подтяжку Pull-Up для PB2 и PB5
    GPIOB_PUPDR &= ~(PUPDR_PB2_MASK | PUPDR_PB5_MASK);
    GPIOB_PUPDR |=  (PUPDR_PB2_PU   | PUPDR_PB5_PU);

    // 4. Гасим все светодиоды при старте
    GPIOB_ODR &= ~(PIN_PB2 | LEDS_234_MASK);
}