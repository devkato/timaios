/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_CONNECTION_
#define _TIMAIOS_HEADER_CONNECTION_

/**
 *
 *
 *
 */
typedef struct tm_connection {
  int fd;
  char *raw_data;
  tm_http_request_t *request;
  tm_http_response_t *response;
  int status;
  int n;
  pthread_mutex_t mutex;
} tm_connection_t;

tm_connection_t *tm_create_connection(int _fd);
void tm_destroy_connection(tm_connection_t *connection);

TM_RETURN_STATUS tm_handle_accept(int _server_socket, int _epfd, struct epoll_event _ev);  
TM_RETURN_STATUS tm_handle_read(int epfd, tm_connection_t *_tm_connection, struct epoll_event _ev);
TM_RETURN_STATUS tm_handle_write(int epfd, tm_connection_t *_tm_connection, struct epoll_event _ev);
TM_RETURN_STATUS tm_handle_close(int epfd, tm_connection_t *tm_connection);

void tm_create_response_data(tm_connection_t *_connection);

void tm_parse_request(tm_connection_t *_connection);

void tm_set_path(tm_connection_t *_connection);

#endif
