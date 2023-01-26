#ifndef __TIMEBASE_H__
#define __TIMEBASE_H__

#include <stdint.h>

void timebase_init(void);
uint32_t get_tick();
void delay_s(uint32_t delay);

#endif
