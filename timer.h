#ifndef _TIMER_H
#define _TIMER_H

#define IOREGS_BASE 0x0000

#define _IO8(offset) (*(volatile unsigned char *) (IOREGS_BASE + offset))
#define _IO16(offset) (*(volatile unsigned short *) (IOREGS_BASE + offset))

#define TIOS    _IO8(0x40)
#define TCNT    _IO16(0x44)
#define TSCR    _IO8(0x46)
#define TMSK2   _IO8(0x4D)
#define TFLG1   _IO8(0x4E)
#define TFLG2   _IO8(0x4F)

#define TOI     0x80
#define TEN     0x80
#define TOF     0x80

#define TIMER_PRESCALE 3
#define TOF_INTERRUPT (*(volatile unsigned short *)0x3E5E)

#define TIMER(var) static struct timer var = {0}

struct timer {
    unsigned int expires_in;
    unsigned char new;
};

void timer_init(void);
void timer_set(struct timer *, unsigned int);
void timer_interrupt(void);
void timer_add(struct timer *);
void timer_remove(struct timer *);
int timer_running(struct timer *);
int timer_expired(struct timer *);


#endif /* _TIMER_H */
