#include "timaios.h"

/**
 * initialize server socket
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
  
  listen(sock, 5);
  
  return sock;
}


/**
 * set the socket as non-blocking one
 *
 * @param _fd file descriptor of the socket
 */
void tm_setnonblocking(int _fd)
{
  int val = 1;
  ioctl(_fd, FIONBIO, &val);
}

int tm_writev(int _fd, struct iovec _iovec[], int buffernum)
{
  return writev(_fd, _iovec, buffernum);
}


/**
 * read data from the file descriptor and save it
 *
 * @param _fd file descriptor to read
 * @param _data to be saved into the pointer
 *
 * @return number of read bytes.
 */
int tm_readv(int _fd, char *_data)
{
  struct iovec _iovec[1];
  
  /* @TODO memory leak? */
  _iovec[0].iov_base = _data;
  _iovec[0].iov_len = TM_REQUEST_MAX_READ_SIZE;
  int n = readv(_fd, _iovec, 1);
  
  // tm_debug("[tm_readv] n : %d\n", n);
  // tm_debug("[tm_readv] data : %s\n", data);
  
  return n;
}


/**
 * write HTTP Response Body with writev
 *
 * @param connection instance
 *
 * @return number of written bytes.
 */
int tm_write_response_data(tm_connection_t *_connection)
{
  struct iovec iovec[3];
  
  /* @TODO memory leak? */
  iovec[0].iov_base = _connection->response->header;
  iovec[0].iov_len = strlen(_connection->response->header);
  
  iovec[1].iov_base = "\r\n";
  iovec[1].iov_len = 2;
  
  iovec[2].iov_base = _connection->response->data;
  iovec[2].iov_len = strlen(_connection->response->data);
  
  return tm_writev(_connection->fd, iovec, 3);
}
