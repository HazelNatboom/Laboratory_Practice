#include <stdint.h>
#include <init.h>
#include "main.h"

void delay(volatile uint32_t count) {
    while (count--) {
        __NOP();
    }
}

// Константы времени (в тиках главного цикла)
#define PWM_PERIOD        100     // Период ШИМ (100 шагов)
#define DEBOUNCE_TICKS    800     // Порог антидребезга (~25-30 мс)
#define LONG_PRESS_TICKS  18000   // Порог длинного нажатия (~0.6-0.8 сек)

int main(void) {
    System_Init();

    // На всякий случай жестко фиксируем PB2 и PB5 как входы с подтяжкой Pull-Up
    GPIOB->MODER &= ~((3U << (2 * 2)) | (3U << (2 * 5)));
    GPIOB->PUPDR &= ~((3U << (2 * 2)) | (3U << (2 * 5)));
    GPIOB->PUPDR |=  ((1U << (2 * 2)) | (1U << (2 * 5)));

    // Скважность для 3 уровней яркости:
    // Индекс 0 -> Тускло (8%), Индекс 1 -> Средне (35%), Индекс 2 -> Ярко (100%)
    const uint8_t duty_table[3] = {8, 35, 100};

    // Состояние светодиодов: 0 - все выкл, 1 - горит 1-й, 2 - горят 1-й и 2-й, 3 - горят все три
    uint8_t active_leds = 0;

    // Уровень яркости для каждого из 3 светодиодов (0 - тускло, 1 - средне, 2 - ярко)
    // По умолчанию ставим всем среднюю яркость (1)
    uint8_t led_brightness[3] = {1, 1, 1};

    // Счетчик для генерации программного ШИМ (0..99)
    uint8_t pwm_counter = 0;

    // Переменные для обработки Кнопки 1 (PB2)
    uint32_t btn1_ticks = 0;
    uint8_t  btn1_long_handled = 0;

    // Переменные для обработки Кнопки 2 (PB5)
    uint32_t btn2_ticks = 0;
    uint8_t  btn2_long_handled = 0;

    while (1) {
        // ====================================================================
        // 1. ОПРОС КНОПКИ 1 (PB2): Включение (короткое) / Выключение (длинное)
        // ====================================================================
        uint8_t btn1_pressed = ((GPIOB->IDR & (1U << 2)) == 0) ? 1 : 0;

        if (btn1_pressed) {
            btn1_ticks++;
            // Если удерживаем дольше порога и длинное нажатие еще не обработано
            if (btn1_ticks >= LONG_PRESS_TICKS && !btn1_long_handled) {
                if (active_leds > 0) {
                    active_leds--; // Выключаем один светодиод (с конца)
                }
                btn1_long_handled = 1; // Блокируем повтор до отпускания кнопки
            }
        } else {
            // Кнопку только что отпустили: проверяем, было ли это короткое нажатие
            if (btn1_ticks >= DEBOUNCE_TICKS && !btn1_long_handled) {
                if (active_leds < 3) {
                    active_leds++; // Включаем следующий светодиод накопительно
                }
            }
            // Сброс таймера кнопки
            btn1_ticks = 0;
            btn1_long_handled = 0;
        }

        // ====================================================================
        // 2. ОПРОС КНОПКИ 2 (PB5): Яркость + (короткое) / Яркость - (длинное)
        // ====================================================================
        uint8_t btn2_pressed = ((GPIOB->IDR & (1U << 5)) == 0) ? 1 : 0;

        if (btn2_pressed) {
            btn2_ticks++;
            // Длинное нажатие -> уменьшаем яркость текущего светодиода
            if (btn2_ticks >= LONG_PRESS_TICKS && !btn2_long_handled) {
                if (active_leds > 0) {
                    uint8_t current_led = active_leds - 1;
                    if (led_brightness[current_led] > 0) {
                        led_brightness[current_led]--;
                    }
                }
                btn2_long_handled = 1;
            }
        } else {
            // Короткое нажатие -> увеличиваем яркость текущего светодиода
            if (btn2_ticks >= DEBOUNCE_TICKS && !btn2_long_handled) {
                if (active_leds > 0) {
                    uint8_t current_led = active_leds - 1;
                    if (led_brightness[current_led] < 2) {
                        led_brightness[current_led]++;
                    }
                }
            }
            btn2_ticks = 0;
            btn2_long_handled = 0;
        }

        // ====================================================================
        // 3. ГЕНЕРАЦИЯ ПРОГРАММНОГО ШИМ (PB6, PB8, PB9)
        // ====================================================================
        pwm_counter++;
        if (pwm_counter >= PWM_PERIOD) {
            pwm_counter = 0;
        }

        uint32_t odr_mask_set = 0;
        uint32_t odr_mask_clear = (1U << 6) | (1U << 8) | (1U << 9);

        // Светодиод 1 (PB6)
        if (active_leds >= 1 && pwm_counter < duty_table[led_brightness[0]]) {
            odr_mask_set |= (1U << 6);
        }
        // Светодиод 2 (PB8)
        if (active_leds >= 2 && pwm_counter < duty_table[led_brightness[1]]) {
            odr_mask_set |= (1U << 8);
        }
        // Светодиод 3 (PB9)
        if (active_leds >= 3 && pwm_counter < duty_table[led_brightness[2]]) {
            odr_mask_set |= (1U << 9);
        }

        // Обновляем состояние пинов одной операцией без лишнего мерцания
        GPIOB->ODR = (GPIOB->ODR & ~odr_mask_clear) | odr_mask_set;

        // Микро-задержка одного тика цикла (~35 мкс)
        delay(100);
    }
}