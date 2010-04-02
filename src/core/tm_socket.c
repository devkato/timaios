#include "timaios.h"

/**
 *
 *
 *
 */
TM_SERVER_SOCKET tm_initialize_socket()
{
  int sock;
  struct sockaddr_in addr;
  int yes = 1;
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(configuration.port);
  addr.sin_addr.s_addr = INADDR_ANY;
  
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
  
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  
  /* set non-blocking */
  // int val = 1;
  // ioctl(sock, FIONBIO, &val);
  
  listen(sock, 5);
  
  return sock;
}


/**
 * set the socket as non-blocking one
 *
 * @param _fd file descriptor of the socket
 *
 */
void tm_setnonblocking(int _fd)
{
  int val = 1;
  ioctl(_fd, FIONBIO, &val);
}
