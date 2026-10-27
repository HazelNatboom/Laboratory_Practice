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
        *(volatile uint32_t *)0x40020400UL &= ~(3U << 4); // PB2 в режим входа (MODER)
        delay(1000); 
        
        // Читаем бит 2 в регистре IDR (0x40020410)
        uint8_t btn1_curr = (*(volatile uint32_t *)0x40020410UL & (1U << 2)) ? 1 : 0;
        if (btn1_last == 1 && btn1_curr == 0) {
            delay(160000); 
            *(volatile uint32_t *)0x40020400UL &= ~(3U << 4);
            if ((*(volatile uint32_t *)0x40020410UL & (1U << 2)) == 0) {
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
            *(volatile uint32_t *)0x40020400UL &= ~(3U << 4);
            *(volatile uint32_t *)0x40020400UL |=  (1U << 4); // PB2 на выход (MODER)
            *(volatile uint32_t *)0x40020414UL |=  (1U << 2); // Включаем PB2 (ODR)
        } else {
            *(volatile uint32_t *)0x40020400UL &= ~(3U << 4); // PB2 на вход (выключен)
        }

        // ==========================================
        // 2. КНОПКА 2 (PB5) -> Накопительное включение светодиодов 2, 3, 4
        // ==========================================
        if (btn2_blocked == 0) {
            // Читаем бит 5 в регистре IDR (0x40020410)
            uint8_t btn2_curr = (*(volatile uint32_t *)0x40020410UL & (1U << 5)) ? 1 : 0;
            if (btn2_last == 1 && btn2_curr == 0) {
                delay(160000); 
                if ((*(volatile uint32_t *)0x40020410UL & (1U << 5)) == 0) {
                    seq_step++;
                    if (seq_step > 3) {
                        seq_step = 1; 
                    }

                    // Обновляем PB6, PB8, PB9 в регистре ODR (0x40020414)
                    *(volatile uint32_t *)0x40020414UL &= ~((1U << 6) | (1U << 8) | (1U << 9));
                    if (seq_step >= 1) *(volatile uint32_t *)0x40020414UL |= (1U << 6); 
                    if (seq_step >= 2) *(volatile uint32_t *)0x40020414UL |= (1U << 8); 
                    if (seq_step >= 3) *(volatile uint32_t *)0x40020414UL |= (1U << 9); 
                }
            }
            btn2_last = btn2_curr;
        }

        delay(50000); 
    }
}