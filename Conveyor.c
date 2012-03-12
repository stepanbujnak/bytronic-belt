/***************************************************************************
*                           School of Engineering 
*                  The Robert Gordon University, Aberdeen
****************************************************************************
*                  File name: Conveyor.c
*                  Author:  Stepan Bujnak & Mihail Florin Popa
*                  Created: 10/11/2011
*                  Class:   Computer Science BSc. (Hons)
*                  Group:   7
****************************************************************************
*                  M68HC12 C Source File
*                  EN2540 Microprocessors and Microcontrollers
*
* Title: Conveyor Belt Project
* Description:
*
***************************************************************************/

#include <stdio.h>
#include "timer.h"

#define OUTPUT(x) *PORTA = x
#define ASSEMBLY_STACK_MAX 6

enum _Bool {
  TRUE = 1,
  FALSE = 0
};

typedef enum _Bool bool;

enum error {
  ERROR_NOT_RUNNING,
  ERROR_NO_CHANGE
};

enum event {
  EVENT_SORT_PEG           = 1,
  EVENT_SORT_OBJECT        = 2,
  EVENT_ASSEMBLY_HOPPER    = 4,
  EVENT_QUALITY_ASSEMBLED  = 8,
  EVENT_QUALITY_OBJECT     = 16,
  EVENT_REJECT_OBJECT      = 32,
  EVENT_START              = 64,
  EVENT_STOP               = 128
};

enum action {
  ACTION_CHAIN_MOTOR       = 1,
  ACTION_SORT_SOLENOID     = 2,
  ACTION_ASSEMBLY_SOLENOID = 4,
  ACTION_REJECT_SOLENOID   = 8,
  ACTION_BELT_MOTOR        = 16
};

unsigned char *PORTA = (unsigned char *)0x0000;
unsigned char *DDRA  = (unsigned char *)0x0002;
unsigned char *PTH   = (unsigned char *)0x0260;
unsigned char *DDRH  = (unsigned char *)0x0262;
unsigned char *PERH  = (unsigned char *)0x0264;

/******************************************************************************
 * Name: handle
 * Parameters: int input: the input we got from the input port
 * Description: 
 *****************************************************************************/
int handle(int);

int
main() {
  int output, old_output = 0;

  *DDRH = 0x00;                   /* make Port H an input port */
  *PERH = 0xFF;                   /* enable Port H */
  *DDRA = 0xFF;                   /* make Port A an output port */
  
  timer_init();

  while (1) {
    output = handle(*PTH);

    if (output >= 0) {
      OUTPUT(output);
    }
  }
 
  return 0;
}

int
handle(int input) {
  int flags = 0;
  int temp;

  static bool sort_next_peg = FALSE;
  static bool assembled = FALSE;
  static bool running = FALSE;
  static bool assembly_solenoid_open = FALSE;
  static unsigned int assembly_stack = 0;
  static int old = 0x00;
  TIMER(sort_timer);
  TIMER(assembly_timer_wait);
  TIMER(assembly_timer_close);
  
  if (timer_expired(&sort_timer)) {
    sort_next_peg = FALSE;
  }
  
  if (timer_expired(&assembly_timer_wait)) {
    OUTPUT(ACTION_CHAIN_MOTOR | ACTION_BELT_MOTOR | ACTION_ASSEMBLY_SOLENOID);
    assembly_solenoid_open = TRUE;
    timer_set(&assembly_timer_close, 500);
  }

  if (timer_expired(&assembly_timer_close)) {
    assembly_solenoid_open = FALSE;
    OUTPUT(ACTION_CHAIN_MOTOR | ACTION_BELT_MOTOR);
  }

  if (!(input ^ old)) {
    return -ERROR_NO_CHANGE;
  }
  
  temp = input;
  input = (input ^ old) & input;
  
  /* Swap values */
  old ^= temp;
  temp ^= old;
  old ^= temp;

  if (input & EVENT_START) {
    running = TRUE;
  
    puts("Start button pressed\r");
  }

  /* If not running, do nothing */
  if (!running) {
    flags = -ERROR_NOT_RUNNING;
    
    return flags;
  }
  
  if (input & EVENT_STOP) {
    flags &= ~ACTION_CHAIN_MOTOR;
    flags &= ~ACTION_BELT_MOTOR;

    running = FALSE;

    puts("Stop button pressed\r");
  }

  if (input & EVENT_SORT_PEG) {
    sort_next_peg = TRUE;
  }

  if (input & EVENT_SORT_OBJECT) {
    if (!sort_next_peg) {
      if (assembly_stack != ASSEMBLY_STACK_MAX) {
        flags |= ACTION_SORT_SOLENOID;
    
        if (assembly_stack == 0) {
          timer_set(&assembly_timer_wait, 1000);
        }
        
        assembly_stack++;

        puts("Ring detected, pushed to the stack\r");
      } else {
        puts("Ring detected, stack is full\r");
      }
    } else {
      timer_set(&sort_timer, 2000);
      puts("Peg detected\r");
    }
  }
  
  if ((temp ^ old) & (~old & EVENT_ASSEMBLY_HOPPER)) {
    if (assembly_stack > 0) {
      assembly_stack--;
    
      if (assembly_stack != 0) {
        timer_set(&assembly_timer_close, 500);
        assembly_solenoid_open = TRUE;
      }
    
      puts("Hopper is empty, pushing new ring\r");
    } else {
      puts("Hopper is empty, no new ring in the stack\r");
    }
  }
  
  if (input & EVENT_QUALITY_ASSEMBLED) {
    assembled = TRUE;
  }

  if (input & EVENT_REJECT_OBJECT) {
    if (!assembled) {
      flags |= ACTION_REJECT_SOLENOID;
    
      puts("Not assembled, pushing away\r");
    } else {
      puts("Successfully assembled\r");
    }

    assembled = FALSE;
  }
  
  if (assembly_solenoid_open) {
    flags |= ACTION_ASSEMBLY_SOLENOID;
  }
  
  flags |= ACTION_CHAIN_MOTOR | ACTION_BELT_MOTOR;

  return flags;
}
