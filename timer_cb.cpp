/* FILE HEADER GOES HERE!!! */

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

static bool timer_cb_inited=FALSE;
void (*user_cb)(void);
unsigned long expire_time;

/* NEED FUNCTION HEADER */
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

/* NEED FUNCTION HEADER */
timer_cb_reg_return_type timer_cb_reg(void (*cb_func)(void), int expire_ms)
{
  unsigned long current_time;
  
  if (!timer_cb_inited)
  {
    init_tick_timer();  
  }

  current_time = millis();
  expire_time = current_time + expire_ms;
  user_cb = cb_func;
  
}

/*====================================================================
 * Time tick ISR
 *
 * This is the interrupt service routine that the arduino calls whenever our
 * 10ms timer expires.  
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

}
