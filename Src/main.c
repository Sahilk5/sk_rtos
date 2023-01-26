#include "led.h"
#include "uart.h"
#include "timebase.h"

int main(void) {
	led_init();
	uart_tx_init();
	timebase_init();

	printf("Welcome to the RTOS!\n");
	while(1) {
		led_on();
		delay_s(1);
		led_off();
		delay_s(1);
	}
}
