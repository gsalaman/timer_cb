#ifndef TIMER_CB_H
#define TIMER_CB_H
/*====================================================
 * TIMER_CB.H
 * 
 * This library provides a timer callback service.  
 * 
 *  First iteration only allows for one timer outstanding
 *  at a given point in time.
 *   
 * Side Effects and Dependencies:
 *    [fill me in!]   
 *======================================================*/

typedef enum
{
  TIMER_CB_REG_SUCCESS=0,
  TIMER_CB_REG_NO_TIMER_AVAILABLE,
  TIMER_CB_REG_INVALID_EXPIRE
} timer_cb_reg_return_type;

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
extern timer_cb_reg_return_type timer_cb_reg(void (*cb_func)(void), int expire_ms);

#endif // TIMER_CB_H
