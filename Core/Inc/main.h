#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

// Макросы адресов регистров
#define RCC_AHB1ENR     (*(volatile uint32_t *)0x40023830UL)
#define GPIOB_MODER     (*(volatile uint32_t *)0x40020400UL)
#define GPIOB_PUPDR     (*(volatile uint32_t *)0x4002040CUL)
#define GPIOB_IDR       (*(volatile uint32_t *)0x40020410UL)
#define GPIOB_ODR       (*(volatile uint32_t *)0x40020414UL)

// Макросы тактирования
#define RCC_GPIOB_EN    (1U << 1)

// Макросы битовых масок для пинов ODR / IDR
#define PIN_PB2         (1U << 2)
#define PIN_PB5         (1U << 5)
#define PIN_PB6         (1U << 6)
#define PIN_PB8         (1U << 8)
#define PIN_PB9         (1U << 9)
#define LEDS_234_MASK   (PIN_PB6 | PIN_PB8 | PIN_PB9)

// Макросы настройки режимов MODER и PUPDR
#define MODER_PB2_MASK  (3U << (2 * 2))
#define MODER_PB2_OUT   (1U << (2 * 2))
#define MODER_PB5_MASK  (3U << (5 * 2))
#define MODER_PB6_MASK  (3U << (6 * 2))
#define MODER_PB6_OUT   (1U << (6 * 2))
#define MODER_PB8_MASK  (3U << (8 * 2))
#define MODER_PB8_OUT   (1U << (8 * 2))
#define MODER_PB9_MASK  (3U << (9 * 2))
#define MODER_PB9_OUT   (1U << (9 * 2))

#define PUPDR_PB2_MASK  (3U << (2 * 2))
#define PUPDR_PB2_PU    (1U << (2 * 2))
#define PUPDR_PB5_MASK  (3U << (5 * 2))
#define PUPDR_PB5_PU    (1U << (5 * 2))

void System_Init(void);

#endif