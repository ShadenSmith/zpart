#ifndef ZPART_TIMER_H
#define ZPART_TIMER_H

/******************************************************************************
 * INCLUDES
 *****************************************************************************/
#include <sys/time.h>
#include <stddef.h>


/******************************************************************************
 * STRUCTURES
 *****************************************************************************/

/**
* @brief Represents a wall-clock timer.
*/
typedef struct
{
  int running;
  double seconds;
  struct timeval start;
  struct timeval stop;
} zp_timer_t;


/******************************************************************************
 * PUBLIC FUNCTIONS
 *****************************************************************************/

/**
* @brief Reset all fields of a zp_timer_t.
*
* @param timer The timer to reset.
*/
static inline void timer_reset(zp_timer_t * const timer)
{
  timer->running       = 0;
  timer->seconds       = 0;
  timer->start.tv_sec  = 0;
  timer->start.tv_usec = 0;
  timer->stop.tv_sec   = 0;
  timer->stop.tv_usec  = 0;
}


/**
* @brief Start a zp_timer_t. NOTE: this does not reset the timer.
*
* @param timer The timer to start.
*/
static inline void timer_start(zp_timer_t * const timer)
{
  timer->running = 1;
  gettimeofday(&(timer->start), NULL);
}


/**
* @brief Stop a zp_timer_t and update its time.
*
* @param timer The timer to stop.
*/
static inline void timer_stop(zp_timer_t * const timer)
{
  timer->running = 0;
  gettimeofday(&(timer->stop), NULL);
  timer->seconds += (double)(timer->stop.tv_sec - timer->start.tv_sec);
  timer->seconds += 1e-6 * (timer->stop.tv_usec - timer->start.tv_usec);
}


/**
* @brief Give a zp_timer_t a fresh start by resetting and starting it.
*
* @param timer The timer to refresh.
*/
static inline void timer_fstart(zp_timer_t * const timer)
{
  timer_reset(timer);
  timer_start(timer);
}

#endif
