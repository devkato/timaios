#include "timaios.h"

/**
 * get unix timestamp
 *
 */
int tm_gettimeofday()
{
  struct timeval tv;
  
  if (gettimeofday(&tv, NULL) < 0) {
    return -1;
  }
  
  return (int)tv.tv_sec;
}
