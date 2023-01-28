#include "led.h"
#include "uart.h"
#include "osKernel.h"

#define QUANTA 20

typedef uint32_t TaskProfiler;

TaskProfiler Task0_TaskProfiler, Task1_TaskProfiler, Task2_TaskProfiler;
void motor_run(void);
void motor_stop(void);
void valve_open(void);
void valve_close(void);

void task0(void) {
	while(1) {
		Task0_TaskProfiler++;
		motor_run();
	}
}

void task1(void) {
	while(1) {
		Task1_TaskProfiler++;
		valve_open();
	}
}

void task2(void) {
	while(1) {
		Task2_TaskProfiler++;
		motor_stop();
		valve_close();
	}
}

int main() {
	uart_tx_init();
	osKernelInit();
	osKernelAddThreads(&task0, &task1, &task2);
	osKernelLaunch(QUANTA);
}

void motor_run(void) {
	printf("motor run called\n");
}

void motor_stop(void) {
	printf("motor stop called\n");
}

void valve_open(void) {
	printf("valve open called \n");
}

void valve_close(void) {
	printf("valve close called \n");
}
