// "get_time.h"
#ifndef _GET_TIME_H
#define _GET_TIME_H

#ifdef __cplusplus 
extern  "C" {
#endif

void start_time_measurement() ;

__int64 end_time_measurement() ;

double get_interval() ;
double get_k_frequency();
double get_cur_time();
inline double get_current_time() { return get_cur_time() ; } ;

#ifdef __cplusplus 
}
#endif

#endif // _GET_TIME_H

