#include "timaios.h"

/**
 *
 *
 *
 */
int tm_epoll_create(int _num_of_event)
{
  int epoll_file_descriptor = epoll_create(_num_of_event);
  
  if (epoll_file_descriptor < 0) {
    tm_perror("epoll_create");
    return 1;
  }

  return epoll_file_descriptor;
}
