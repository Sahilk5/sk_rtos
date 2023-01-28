#include "led.h"
#include "uart.h"
#include "osKernel.h"

#define QUANTA 2

typedef uint32_t TaskProfiler;

TaskProfiler Task0_TaskProfiler, Task1_TaskProfiler, Task2_TaskProfiler;
TaskProfiler PTask0_TaskProfiler, PTask1_TaskProfiler;
void motor_run(void);
void motor_stop(void);
void valve_open(void);
void valve_close(void);

int32_t semaphore1, semaphore2;

void task0(void) {
	while(1) {
		Task0_TaskProfiler++;
		osSemaphoreWait(&semaphore1);
		motor_run();
		osSemaphoreSet(&semaphore2);
		osThreadYield();
	}
}

void task1(void) {
	while(1) {
		Task1_TaskProfiler++;
		osSemaphoreWait(&semaphore2);
		valve_open();
		osSemaphoreSet(&semaphore1);
	}
}

void task2(void) {
	while(1) {
		Task2_TaskProfiler++;
	}
}

void Ptask0(void) {
	PTask0_TaskProfiler++;
}

int main() {
	uart_tx_init();

	tim_2_1hz_interrupt_init();

	osSemaphoreInit(&semaphore1, 1);
	osSemaphoreInit(&semaphore2, 0);

	osKernelInit();
	osKernelAddThreads(&task0, &task1, &task2);
	osKernelLaunch(QUANTA);
}

void TIM2_IRQHandler(void) {
	/* Clear update interrupt flag */
	TIM2->SR &= ~(SR_UIF);
	/* Stuff */
	PTask1_TaskProfiler++;
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
