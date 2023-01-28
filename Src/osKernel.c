#include "osKernel.h"

#define NUM_OF_THREADS	3
#define STACKSIZE	400  /* 1600 bytes */

#define BUS_FREQ 16000000

#define CTRL_ENABLE (1U<<0)
#define CTRL_TICKINT (1U<<1)
#define CTRL_CLKSRC (1U<<2)
#define CTRL_COUNTFLAG (1U<<16)
#define SYSTICK_RESET 0

void osSchedulerLaunch(void);

uint32_t MILLIS_PRESCALER;

struct tcb {
	int32_t *stackPtr;
	struct tcb *nextPtr;
};

typedef struct tcb tcbType;

tcbType tcbs[NUM_OF_THREADS];
tcbType *currentPtr;

int32_t TCB_STACK[NUM_OF_THREADS][STACKSIZE];

/* Registers preserved on exception
 *
 * xPSR
 * PC
 * LR
 * R12
 * R3
 * R2
 * R1
 * R0	<- top of stack
 */
void osKernelStackInit(int i) {
	tcbs[i].stackPtr = &TCB_STACK[i][STACKSIZE-16]; /* Stack Pointer */

	/* T-bit(bit 21) set to operate in thumb mode */
	TCB_STACK[i][STACKSIZE-1] = (1U<<24);  /* PSR */

	/* Optional Dummy content - register bank in cortex M4 user guide */
	TCB_STACK[i][STACKSIZE-3] = 0xAAAAAAAA; /* R14 LR */
	TCB_STACK[i][STACKSIZE-4] = 0xAAAAAAAA; /* R12 */
	TCB_STACK[i][STACKSIZE-5] = 0xAAAAAAAA; /* R3 */
	TCB_STACK[i][STACKSIZE-6] = 0xAAAAAAAA; /* R2 */
	TCB_STACK[i][STACKSIZE-7] = 0xAAAAAAAA; /* R1 */
	TCB_STACK[i][STACKSIZE-8] = 0xAAAAAAAA; /* R0 */


	TCB_STACK[i][STACKSIZE-9] = 0xAAAAAAAA; /* R11 */
	TCB_STACK[i][STACKSIZE-10] = 0xAAAAAAAA; /* R10 */
	TCB_STACK[i][STACKSIZE-11] = 0xAAAAAAAA; /* R9 */
	TCB_STACK[i][STACKSIZE-12] = 0xAAAAAAAA; /* R8 */
	TCB_STACK[i][STACKSIZE-13] = 0xAAAAAAAA; /* R7 */
	TCB_STACK[i][STACKSIZE-14] = 0xAAAAAAAA; /* R6 */
	TCB_STACK[i][STACKSIZE-15] = 0xAAAAAAAA; /* R5 */
	TCB_STACK[i][STACKSIZE-16] = 0xAAAAAAAA; /* R4 */
}

uint8_t osKernelAddThreads(void(*task0)(void), void(*task1)(void), void(*task2)(void)) {
	/* Disable global interrupts */
	__disable_irq();

	/* Make circular linked list of tasks */
	tcbs[0].nextPtr = &tcbs[1];
	tcbs[1].nextPtr = &tcbs[2];
	tcbs[2].nextPtr = &tcbs[0];

	/* Initialize stacks and PC*/
	osKernelStackInit(0);
	TCB_STACK[0][STACKSIZE-2] = (uint32_t)task0;
	osKernelStackInit(1);
	TCB_STACK[1][STACKSIZE-2] = (uint32_t)task1;
	osKernelStackInit(2);
	TCB_STACK[2][STACKSIZE-2] = (uint32_t)task2;

	/* Start from first task */
	currentPtr = &tcbs[0];

	/* Enable global interrupts */
	__enable_irq();

	return 1;
}

void osKernelInit(void) {
	MILLIS_PRESCALER = (BUS_FREQ/1000);
}

void osKernelLaunch(uint32_t quanta) {
	/* Reset SysTick */
	SysTick->CTRL = SYSTICK_RESET;

	/* Clear SysTick current val. reg */
	SysTick->VAL = 0;

	/* Load quanta */
	SysTick->LOAD = (quanta * MILLIS_PRESCALER) - 1;

	/* Set sysTick to low priority*/
	NVIC_SetPriority(SysTick_IRQn, 15);

	/* Enable sysTick, select internal clock*/
	SysTick->CTRL = CTRL_CLKSRC | CTRL_ENABLE;

	/* Enable sysTick interrupt */
	SysTick->CTRL |= CTRL_TICKINT;

	/* Launch scheduler */
	osSchedulerLaunch();
}

__attribute__((naked)) void SysTick_Handler(void) {
	/* SUSPEND CURRENT THREAD */
	/* Disable interrupts*/
	__asm("CPSID I");

	/* Save r4-r11*/
	__asm("PUSH {R4-R11}");

	/* Load address of currentPtr in R0 */
	__asm("LDR R0, =currentPtr");

	/* Load r1 from address equals r0, i.e. r1 =currentPtr */
	__asm("LDR R1, [R0]");

	/* Store Cortex-M SP at address equals r1, i.e. Save SP into tcb*/
	__asm("STR SP, [R1]");

	/* CHOOSE NEXT THREAD */
	/* Load r1 from a location 4bytes above address r1, i.e. r1 = currentPtr->next */
	__asm("LDR R1, [R1,#4]");

	/* Store R1 at address equals r0, i.e. currentPtr = r1*/
	__asm("STR R1, [R0]");

	/* Load the Cortex-M SP from address equals r1, i.e. SP = currentPtr->StackPtr */
	__asm("LDR SP, [R1]");

	/* Restore manually saved registers*/
	__asm("POP {R4-R11}");

	/* Enable global interrupts*/
	__asm("CPSIE I");

	/* Return from exception and restore r0,r1,r2,r3,r12,lr,pc,psr */
	__asm("BX LR");
}

void osSchedulerLaunch(void) {
	/* Load address of currentPtr into R0*/
	__asm("LDR R0, =currentPtr");
	/* Load R2 from address equals R0 */
	__asm("LDR R2, [R0]");
	/* Load Cortex-M SP from address equals R2, i.e. SP = currentPtr->stackPtr */
	__asm("LDR SP, [R2]");
	/* Restore manually saved registers*/
	__asm("POP {R4-R11}");
	/* restore r0,r1,r2,r3, r12 */
	__asm("POP {R0-R3}");
	__asm("POP {R12}");
	/*Skip LR*/
	__asm("ADD SP, SP, #4");
	/* Create a new start location by popping LR(task0?)*/
	__asm("POP {LR}");
	/* skip PSR*/
	__asm("ADD SP, SP, #4");
	/* Enable global interrupts*/
	__asm("CPSIE I");
	/* Return from exception */
	__asm("BX LR");
}
