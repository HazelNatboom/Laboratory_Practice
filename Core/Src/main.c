#include <stdint.h>
#include <init.h>
#include "main.h"

void delay(volatile uint32_t count) {
    while (count--) {
        __NOP();
    }
}

int main(void) {
    System_Init();

    // Переменные для Кнопки 2 (PB5) и светодиодов 2, 3, 4 (PB6, PB8, PB9)
    uint8_t seq_step = 0;          
    uint8_t btn2_last = 1;         

    // Переменные для Кнопки 1 / Светодиода 1 (PB2)
    uint32_t btn1_press_count = 0; 
    uint8_t btn1_last = 1;         
    uint8_t btn2_blocked = 0;      // Флаг блокировки Кнопки 2 (от Кнопки 1)
    uint8_t led1_active_mode = 0;  // Состояние Светодиода 1 (0 - выкл, 1 - вкл)

    while (1) {
        // ==========================================
        // 1. КНОПКА 1 (PB2) -> Управляет блокировкой Кнопки 2 и Светодиодом 1
        // ==========================================
        GPIOB->MODER &= ~(3U << (2 * 2)); // Перевод PB2 в режим ВХОДА для опроса кнопки
        delay(1000); 
        
        uint8_t btn1_curr = (GPIOB->IDR & (1U << 2)) ? 1 : 0;
        if (btn1_last == 1 && btn1_curr == 0) { // Нажатие Кнопки 1
            delay(160000); // Антидребезг
            GPIOB->MODER &= ~(3U << (2 * 2));
            if ((GPIOB->IDR & (1U << 2)) == 0) {
                btn1_press_count++; 
                
                // Каждые 5 нажатий Кнопки 1 переключаем режим
                if (btn1_press_count % 5 == 0) {
                    btn2_blocked = !btn2_blocked;       // Отключаем/включаем Кнопку 2
                    led1_active_mode = !led1_active_mode; // Включаем/выключаем Светодиод 1
                }
            }
        }
        btn1_last = btn1_curr;

        // Управляем физическим состоянием пина PB2 (Светодиод 1)
        if (led1_active_mode) {
            GPIOB->MODER &= ~(3U << (2 * 2));
            GPIOB->MODER |=  (1U << (2 * 2)); // Выход
            GPIOB->ODR   |=  (1U << 2);       // Светодиод 1 горит
        } else {
            GPIOB->MODER &= ~(3U << (2 * 2)); // Вход (Светодиод 1 выключен)
        }


        // ==========================================
        // 2. КНОПКА 2 (PB5) -> Накопительное включение светодиодов 2, 3, 4
        // ==========================================
        if (btn2_blocked == 0) {
            uint8_t btn2_curr = (GPIOB->IDR & (1U << 5)) ? 1 : 0;
            if (btn2_last == 1 && btn2_curr == 0) { // Нажатие Кнопки 2
                delay(160000); // Антидребезг
                if ((GPIOB->IDR & (1U << 5)) == 0) {
                    seq_step++;
                    if (seq_step > 3) {
                        seq_step = 1; 
                    }

                    // Гасим PB6, PB8, PB9 и накопительно зажигаем
                    GPIOB->ODR &= ~((1U << 6) | (1U << 8) | (1U << 9));
                    if (seq_step >= 1) GPIOB->ODR |= (1U << 6); // Светодиод 2
                    if (seq_step >= 2) GPIOB->ODR |= (1U << 8); // Светодиод 3
                    if (seq_step >= 3) GPIOB->ODR |= (1U << 9); // Светодиод 4
                }
            }
            btn2_last = btn2_curr;
        }

        delay(50000); 
    }
}