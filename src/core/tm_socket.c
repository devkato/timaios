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
  int send_buffer_size = 65535;
  // int timeout = 0;
  
  sock = socket(AF_INET, SOCK_STREAM, 0);
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(configuration.port);
  addr.sin_addr.s_addr = INADDR_ANY;
  
  setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(int));
  setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (const void *)&send_buffer_size, sizeof(int));
  setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const void *)&send_buffer_size, sizeof(int));
  setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const void*)&yes, (socklen_t)sizeof(int));
  setsockopt(sock, IPPROTO_TCP, TCP_CORK, (const void*)&yes, (socklen_t)sizeof(int));
  
  // setsockopt(sock, IPPROTO_TCP, TCP_DEFER_ACCEPT, (const void*)&timeout, sizeof(int));
  // setsockopt(sock, IPPROTO_TCP, TCP_QUICKACK, (const void*)&yes, (socklen_t)sizeof(int));
  tm_setnonblocking(sock);
  
  bind(sock, (struct sockaddr *)&addr, sizeof(addr));
  
  listen(sock, 64);
  
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

void tm_create_child_process()
{
  // if (socketpair(AF_UNIX, SOCK_STREAM, 0, ngx_processes[s].channel) == -1)
}


/**
 * write data to file descriptor with writev.
 *
 * @param _fd file descriptor
 * @param _iovec[] data array to write
 * @param buffernum size of data array
 *
 * @return number of bytes written
 */
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
  
  _iovec[0].iov_base = _data;
  _iovec[0].iov_len = TM_REQUEST_MAX_READ_SIZE;
  int n = readv(_fd, _iovec, 1);
  
  // tm_debug("[tm_readv] n : %d\n", n);
  // tm_debug("[tm_readv] data : %s\n", _data);
  
  // int n = read(_fd, _data, TM_REQUEST_MAX_READ_SIZE);
  
  if (n < 0) {
    tm_perror("read");
  }
  
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

int tm_write_response_data2(tm_connection_t *_connection)
{
  char *data;
  struct tm *date;
  char *datestr;
  int n;
  
  data = tm_memory_allocate((size_t)8192 * 2);
  
  /* set request time */
  datestr = tm_memory_allocate((size_t)256);

  // tm_debug("_connection->request->timestamp : %d\n", (int)_connection->request->timestamp);

  date = localtime(&_connection->request->timestamp);
  strftime(datestr, 255, "%A, %d %B %Y %H:%M:%S GMT", date);

  snprintf(data,
    8192 * 2,
    "HTTP/1.1 %d %s\r\n"
    "Cache-Control: private, max-age=0\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: %ld\r\n"
    "Server: timaios/%s\r\n"
    "Date: %s\r\n"
    "\r\n"
    "%s",
    _connection->response->status,
    tm_http_from_status_code_to_string(_connection->response->status),
    strlen(_connection->response->data),
    TM_VERSION,
    datestr,
    _connection->response->data
  );
  
  n = write(_connection->fd, data, strlen(data));
  
  if (n < 0) {
    tm_perror("write");
  }
  
  tm_memory_free(data);
  tm_memory_free(datestr);
  
  return n;
}

