#include "led.h"

#define GPIOAEN	(1U<<0)
#define LED_PIN (1U<<5)
void led_init(void) {
	/*  Enable clock for PORT A in AHB bus */
	RCC->AHB1ENR |= GPIOAEN;

	/* Set led pin as output*/
	GPIOA->MODER |= (1U<<10);
	GPIOA->MODER &= ~(1U<<11);
}

void led_on(void) {
	/* Set led pin high */
	GPIOA->ODR |= LED_PIN;
}

void led_off(void) {
	/* Set led pin off */
	GPIOA->ODR &= ~LED_PIN;
}
