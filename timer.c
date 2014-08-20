#include <stdio.h>
#include <stdlib.h>

#include "timer.h"

#pragma interrupt_handler timer_interrupt

static unsigned char timer_initialized = 0;
static unsigned int  miliseconds       = 0;

void
timer_init(void) {
    if (!timer_initialized) {
        asm("sei");
        TSCR |= TEN;
        TMSK2 = TOI | TIMER_PRESCALE;
        TFLG2 |= TOF;

        TOF_INTERRUPT = (unsigned short)&timer_interrupt;

        asm("cli");
        timer_initialized = 1;
    }
}

void
timer_set(struct timer *t, unsigned int time) {
    t->expires_in = miliseconds + time;
    t->new = 1;
}

int
timer_expired(struct timer *t) {
    if (!t->new) {
        return 0;
    } else {
        if (t->expires_in < miliseconds) {
            t->new = 0;
            return 1;
        } else {
            return 0;
        }
    }
}

void
timer_interrupt(void) {
    miliseconds += 33;

    if (miliseconds > 60000) {
        miliseconds = 0;
    }

    TFLG2 |= TOF;
}
