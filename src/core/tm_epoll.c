#include "timaios.h"

/**
 * create descriptor for event polling.
 *
 * @param _num_of_event number of events to be registered.(but almost ignored according to man page of "epoll")
 */
int tm_epoll_create(int _num_of_event)
{
  int epoll_file_descriptor = epoll_create(1);//epoll_create(_num_of_event);
  
  if (epoll_file_descriptor < 0) {
    tm_perror("epoll_create");
    return 1;
  }

  return epoll_file_descriptor;
}
