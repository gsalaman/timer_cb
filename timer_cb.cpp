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
 *     called every 2 ms.  We use that to 
 *     check and see whether a given timer has expired.
 *   
 * Side Effects and Dependencies:
 *    Takes over TIMER2.   
 *======================================================*/

#include "timer_cb.h"
#include <arduino.h>

// No explicit init function needed; we'll use this 
// flag to check and see if we need to init the library.
static bool timer_cb_inited=false;

// Currently only support one timer at a given time.
void (*user_cb)(void)=NULL;

// uptime mark:  when does our timer expire.
unsigned long expire_time;

/*====================================================
 * INTERNAL Function:  init_tick_timer
 * 
 * Description:
 *  This function initializes our timer module, specifically
 *  setting up TIMER2 as a 2ms periodic timer.
 *
 * Parameters:  None
 *
 * Return Value: None
 *======================================================*/
static void init_tick_timer( void )
{
  // current iteration:  tick every 2 ms.
  //    Math:  2ms period = 500 Hz.
  //           use prescaler of 256 
  //           compare match register:  16 MHz/256/500 = 125.
  int compare_match_reg=125;

  // initialize timer2 
  noInterrupts();           // disable all interrupts
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;
  OCR2A = compare_match_reg;// compare match register 
  TCCR2B |= (B00000010);   // CTC mode
  TCCR2B |= (B00000110);    // 256 prescaler 
  TIMSK2 |= (1 << OCIE2A);   // enable timer compare interrupt
  interrupts();              // enable all interrupts  
  
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
  
  // Causal operations only.
  if (expire_ms < 0)
  {  
    return TIMER_CB_REG_INVALID_EXPIRE;
  }

  // current implementation only allows one callback at a time.  
  if (user_cb)
  {
    return TIMER_CB_REG_NO_TIMER_AVAILABLE;
  }
  
  // Check to see whether we've initialized the timer callback module.
  if (!timer_cb_inited)
  {
    init_tick_timer();  
    timer_cb_inited = true;
  }

  // what time is it now?
  current_time = millis();

  // how far in the future will our timer expire?
  expire_time = current_time + expire_ms;

  // set the user callback.
  user_cb = cb_func;

  return TIMER_CB_REG_SUCCESS;  

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
ISR(TIMER2_COMPA_vect)
{
  unsigned long current_time;

  // If there's no timer registered, we have nothing to do!
  if (!user_cb) return;
  
  // What time is it now?
  current_time = millis();

  // did our callback expire?
  if (current_time >= expire_time)
  {
    // call the user callback
    user_cb();

    // this is a one time call, so remove the callback
    user_cb = NULL;
  }

}  // Time Tick ISR
