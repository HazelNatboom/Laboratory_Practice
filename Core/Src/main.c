#include <init.h>
#include <stdint.h>
#include "main.h"

void delay(volatile uint32_t count) {
    while (count--) {
        __asm volatile ("nop");
    }
}

int main(void) {
    System_Init();

    uint8_t seq_step = 0;          
    uint8_t btn2_last = 1;         

    uint32_t btn1_press_count = 0; 
    uint8_t btn1_last = 1;         
    uint8_t btn2_blocked = 0;      
    uint8_t led1_active_mode = 0;  

    while (1) {
        // ==========================================
        // 1. КНОПКА 1 (PB2) -> Блокировка Кнопки 2 и управление Светодиодом 1
        // ==========================================
        GPIOB_MODER &= ~MODER_PB2_MASK; // Перевод PB2 в режим входа
        delay(1000); 
        
        uint8_t btn1_curr = (GPIOB_IDR & PIN_PB2) ? 1 : 0;
        if (btn1_last == 1 && btn1_curr == 0) {
            delay(160000); 
            GPIOB_MODER &= ~MODER_PB2_MASK;
            if ((GPIOB_IDR & PIN_PB2) == 0) {
                btn1_press_count++; 
                
                if (btn1_press_count % 5 == 0) {
                    btn2_blocked = !btn2_blocked;       
                    led1_active_mode = !led1_active_mode; 
                }
            }
        }
        btn1_last = btn1_curr;

        // Управление Светодиодом 1 на пине PB2
        if (led1_active_mode) {
            GPIOB_MODER &= ~MODER_PB2_MASK;
            GPIOB_MODER |=  MODER_PB2_OUT; // Выход
            GPIOB_ODR   |=  PIN_PB2;       // Светодиод 1 горит
        } else {
            GPIOB_MODER &= ~MODER_PB2_MASK; // Вход (Светодиод 1 выключен)
        }

        // ==========================================
        // 2. КНОПКА 2 (PB5) -> Накопительное включение светодиодов 2, 3, 4
        // ==========================================
        if (btn2_blocked == 0) {
            uint8_t btn2_curr = (GPIOB_IDR & PIN_PB5) ? 1 : 0;
            if (btn2_last == 1 && btn2_curr == 0) {
                delay(160000); 
                if ((GPIOB_IDR & PIN_PB5) == 0) {
                    seq_step++;
                    if (seq_step > 3) {
                        seq_step = 1; 
                    }

                    GPIOB_ODR &= ~LEDS_234_MASK;
                    if (seq_step >= 1) GPIOB_ODR |= PIN_PB6; 
                    if (seq_step >= 2) GPIOB_ODR |= PIN_PB8; 
                    if (seq_step >= 3) GPIOB_ODR |= PIN_PB9; 
                }
            }
            btn2_last = btn2_curr;
        }

        delay(50000); 
    }
}