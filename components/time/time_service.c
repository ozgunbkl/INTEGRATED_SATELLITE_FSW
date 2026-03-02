#include "time_service.h"

static uint64_t total_milliseconds = 0;

void TIME_Init(void) {
    total_milliseconds = 0;
}

void TIME_Tick1ms(void) {
    total_milliseconds++;
}

uint32_t TIME_GetSeconds(void) {
    return (uint32_t)(total_milliseconds / 1000);
}

uint64_t TIME_GetMilliseconds(void) {
    return total_milliseconds;
}
