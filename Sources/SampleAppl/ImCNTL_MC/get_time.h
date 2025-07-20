#ifndef __GET_TIME_H
#define __GET_TIME_H

extern double get_k_frequency();      // <- this is external program
extern double get_current_time() ;    // <- this is external program

static double start_measure;

static inline void start_time_measurement(  )
{
  start_measure = get_current_time();
};

static inline double end_time_measurement(  )
{
  return ( get_current_time(  ) - start_measure );
};

static inline double get_interval()
{
  return ( end_time_measurement() );
};

#endif  // __GET_TIME_H