#ifndef __OSKERNEL_H__
#define __OSKERNEL_H__

#include <stdint.h>

#include "stm32f4xx.h"

uint8_t osKernelAddThreads(void(*task0)(void), void(*task1)(void), void(*task2)(void));
void osKernelInit(void);
void osKernelLaunch(uint32_t quanta);
void osThreadYield(void);

#endif
