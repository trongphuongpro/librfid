//! \file utils.c
//! \brief Utility functions, structs for Tiva c.
//! \author Nguyen Trong Phuong (aka trongphuongpro)
//! \date 2020 April 8

#include "utils.h"

#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/systick.h"


volatile uint16_t counter = 0;


static void systick_isr();

void systick_isr() {
    if (counter > 0) {
        counter--;
    }
}

void delay_init() {
    SysTickPeriodSet(SysCtlClockGet()/1000); // period = 1 ms
    SysTickIntRegister(systick_isr);
    SysTickEnable();
}

void delay_ms(uint16_t ms) {
    counter = ms;

    while (counter);
}

/********************* End of File *******************************************/