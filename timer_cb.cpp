/*====================================================
 * TIMER_CB.CPP
 * 
 * This library provides a timer callback service.  
 * 
 *  First iteration only allows for one timer outstanding
 *  at a given point in time.
 *
 *  Implementation:
 *     Since timers on the arduino only go to a couple seconds,
 *     we create a tick timer with associated ISR that gets 
 *     called every 10 ms.  We use that to 
 *     check and see whether a given timer has expired.
 *   
 * Side Effects and Dependencies:
 *    Takes over TIMER1.   
 *======================================================*/

#include "timer_cb.h"
#include <arduino.h>

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

// No explicit init function needed; we'll use this 
// flag to check and see if we need to init the library.
static bool timer_cb_inited=FALSE;

// Currently only support one timer at a given time.
void (*user_cb)(void)=NULL;

// uptime mark:  when does our timer expire.
unsigned long expire_time;

/*====================================================
 * INTERNAL Function:  init_tick_timer
 * 
 * Description:
 *  This function initializes our timer module, specifically
 *  setting up TIMER1 as a 10ms periodic timer.
 *
 * Parameters:  None
 *
 * Return Value: None
 *======================================================*/
static void init_tick_timer( void )
{
  // current iteration:  tick every 10 ms.
  //    Math:  10ms period = 100 Hz.
  //           use prescaler of 256.
  //           compare match register:  16 MHz/256/100 = 625.
  int compare_match_reg=625;

  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = compare_match_reg;// compare match register 
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (B00000100);    // 256 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts  

}  // init_tick_timer

/*====================================================
 * API Function:  timer_cb_reg
 * 
 * Description:
 *  This function registers a callback function to fire
 *  at a given time in the future.
 * 
 * Parameters:
 *    cb_func:  function to call after specified time.  
 *              Note no return value or parameters.
 *    expire_ms:  how many MS in the future to call the 
 *                callback function
 *
 * Return Value: error code for registering the timer.  
 *               See timer_cb_reg_return_type above for specifics.
 * 
 * Side Effects and Dependencies:
 *    callback function will run in interrupt space, so make it as
 *    lightweight as possible.
 *======================================================*/
timer_cb_reg_return_type timer_cb_reg(void (*cb_func)(void), int expire_ms)
{
  unsigned long current_time;
  
  // Check to see whether we've initialized the timer callback module.
  if (!timer_cb_inited)
  {
    init_tick_timer();  
  }

  // what time is it now?
  current_time = millis();

  // how far in the future will our timer expire?
  expire_time = current_time + expire_ms;

  // set the user callback.
  user_cb = cb_func;
  
}  // timer_cb_reg

/*====================================================================
 * Time tick ISR
 *
 * This is the interrupt service routine that the arduino calls whenever our
 * 10ms timer expires.
 *
 * We currently check current time and, if the expire for our callback has passed,
 * call the callback function.   
 * ==================================================================*/
ISR(TIMER1_COMPA_vect)
{
  unsigned long current_time;
  
  // What time is it now?
  current_time = millis();

  // did our callback expire?
  if (current_time >= expire_time)
  {
    // call the user callback
    user_cb();
  }

}  // Time Tick ISR
