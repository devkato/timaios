#include "timaios.h"

/**
 * create connection instance.
 *
 * @param _fd file descriptor of the socket that is made from client's request.
 *
 * @return created connection instance.
 */
tm_connection_t *tm_create_connection(int _fd)
{
  tm_connection_t *connection;
  
  connection = tm_memory_allocate(sizeof(tm_connection_t));
  if (connection == NULL) {
    tm_perror("malloc");
    return NULL;
  }
  
  tm_memory_reset(connection, sizeof(tm_connection_t));
  
  connection->fd = _fd;
  connection->raw_data = tm_memory_allocate((size_t)TM_REQUEST_MAX_READ_SIZE);
  connection->request = tm_memory_allocate(sizeof(tm_http_request_t));
  
  // reset memory
  tm_memory_reset(connection->raw_data, TM_REQUEST_MAX_READ_SIZE);
  tm_memory_reset(connection->request, sizeof(tm_http_request_t));
  
  connection->request->request_uri = tm_memory_allocate((size_t)TM_REQUEST_URL_MAX_LENGTH);
  connection->request->http_method = tm_memory_allocate((size_t)TM_HTTP_METHOD_MAX_LENGTH);
  connection->request->timestamp = time(NULL);
  connection->request->path = tm_memory_allocate(1024);  // @TODO define

  connection->response = tm_memory_allocate(sizeof(tm_http_response_t));
  connection->response->data = tm_memory_allocate((size_t)TM_RESPONSE_DATA_MAX_LENGTH);
  connection->response->header = tm_memory_allocate((size_t)TM_RESPONSE_HEADER_MAX_LENGTH);
  
  return connection;
}


/**
 * free memory that is used by connection instance.
 *
 * @param connection instance
 */
void tm_destroy_connection(tm_connection_t *_connection)
{
  tm_memory_free(_connection->raw_data);
  
  tm_memory_free(_connection->request->request_uri);
  tm_memory_free(_connection->request->http_method);
  tm_memory_free(_connection->request->path);
  tm_memory_free(_connection->request);

  tm_memory_free(_connection->response->header);
  tm_memory_free(_connection->response->data);
  tm_memory_free(_connection->response);

  tm_memory_free(_connection);
  
  return;
}

TM_RETURN_STATUS tm_handle_accept(int _server_socket, int _epfd, struct epoll_event _ev)
{
  socklen_t len;
  int client_socket;
  struct sockaddr_in client;
  
  /* handling event that accept client'request */
  len = sizeof(client);
  client_socket = accept(_server_socket, (struct sockaddr *)&client, &len);
  tm_setnonblocking(client_socket);
  
  if (client_socket < 0) {
    if (errno == EAGAIN) {
      tm_debug("MADA KONAI");
      return -1;
    } else {
      tm_perror("accept");
      return -1;
    }
  }
  
  tm_memory_reset(&_ev, sizeof(_ev));
  
  _ev.events = EPOLLIN | EPOLLHUP | EPOLLET;
  _ev.data.ptr = tm_create_connection(client_socket);

  if (_ev.data.ptr == NULL) {
    tm_debug("failed to create connection");
    return 1;
  }
  
  if (epoll_ctl(_epfd, EPOLL_CTL_ADD, client_socket, &_ev) != 0) {
    tm_perror("epoll_ctl:(EPOLL_CTL_ADD)");
    return 1;
  }
  
  return 0;
}


/**
 * Read data from client.
 * 
 * @TODO sometimes(at the end of performance check) tm_readv gets invalid data from client socket.
 * probably problem exists on timeout setting or so?
 *
 */
TM_RETURN_STATUS tm_handle_read(int epfd, tm_connection_t *_connection, struct epoll_event ev)
{
  tm_debug("before read : %s", _connection->raw_data);
  
  /* event that read data from client's request */
  _connection->n = tm_readv(_connection->fd, _connection->raw_data);
  
  tm_debug("_connection->n : %d", _connection->n);
  tm_debug("strlen(_connection->raw_data) : %d", (int)strlen(_connection->raw_data));
  tm_debug("_connection->raw_data after tm_readv");
  tm_debug("\n%s", _connection->raw_data);
  
  /* @TODO manage EAGAIN status */
  if (_connection->n < 0) {
    if (errno == EAGAIN) {
      tm_debug("waiting data ...");
      return 1;
    } else {
      tm_perror("read");
      return 1;
    }
  }
  
  _connection->status = TM_CONNECTION_READ;
  tm_parse_request(_connection);

  ev.events = EPOLLOUT | EPOLLHUP | EPOLLET;
  
  if (epoll_ctl(epfd, EPOLL_CTL_MOD, _connection->fd, &ev) != 0) {
    tm_perror("epoll_ctl(EPOLL_CTL_MOD)");
    return 1;
  }
  
  return 0;
}


/* event that write response data to the client */
TM_RETURN_STATUS tm_handle_write(int epfd, tm_connection_t *tm_connection, struct epoll_event ev)
{
  int n;
  // tm_debug("tm_connection->request->path : %s", tm_connection->request->path);
  
  tm_action_t *action = tm_action_find(tm_connection->request->path);
  if (action) {
    action->func(tm_connection);
  } else {
    tm_action_not_found(tm_connection);
  }
  
  /* write header data */
  tm_http_write_header(tm_connection);
  
  /* write data into socket */
  n = tm_write_response_data(tm_connection);
  
  // n = tm_write_response_data2(tm_connection);
  
  if (n < 0) {
    tm_perror("write");
    return 1;
  }
  
  tm_connection->status = TM_CONNECTION_WRITE;
  
  tm_handle_close(epfd, tm_connection);
  
  tm_destroy_connection(ev.data.ptr);
  
  return 0;
}


TM_RETURN_STATUS tm_handle_close(int epfd, tm_connection_t *tm_connection)
{
  if (epoll_ctl(epfd, EPOLL_CTL_DEL, tm_connection->fd, NULL) != 0) {
    tm_perror("epoll_ctl(EPOLL_CTL_DEL)");
    return -1;
  }
  
  if (close(tm_connection->fd) != 0) {
    tm_perror("close");
    return -1;
  }

  return 0;
}


/**
 * parse request data and set several informatino about the request into connection
 *
 * @param connection instance
 * buggy!
 */
void tm_parse_request(tm_connection_t *_connection)
{
  size_t n;
  int len;
  http_parser *parser;
  
  if (_connection->raw_data == NULL || strlen(_connection->raw_data) <= 0) {
    tm_debug("invalid request");
    return;
  }
  
  tm_debug("\n%s", _connection->raw_data);

  len = strlen(_connection->raw_data);
  
  http_parser_settings settings = { 
    .on_message_begin     = NULL,
    .on_header_field      = tm_http_callback_for_header_filed,
    .on_header_value      = tm_http_callback_for_header_value,
    .on_path              = tm_http_callback_for_path,
    .on_url               = tm_http_callback_for_url,
    .on_fragment          = NULL,
    .on_query_string      = tm_http_callback_for_query_string,
    .on_body              = NULL,
    .on_headers_complete  = NULL,
    .on_message_complete  = NULL
  };
  
  parser = (http_parser *)tm_memory_allocate(sizeof(http_parser));
  tm_memory_reset(parser, sizeof(http_parser));
  
  parser->data = (tm_http_request_t *)(_connection->request);
  
  http_parser_init(parser, HTTP_REQUEST);
  n = http_parser_execute(parser, settings, _connection->raw_data, len);
  tm_debug("(len, n) => (%d, %d)\n", len, (int)n);
  
  tm_memory_free(parser);
  
  tm_debug("_connection->request->path : %s", _connection->request->path);
}


/**
 * get and save path from request uri to request object.
 *
 * @param connection instance
 */
void tm_set_path(tm_connection_t *_connection)
{
  int i;
  int len = strlen(_connection->request->request_uri);
  char c;
  for (i = 0; i < len; i++) {
    c = _connection->request->request_uri[i];
    if (c == '?') {
      break;
    }
    // tm_debug("%c\n", c);
  }
  
  sprintf(_connection->request->path, "%.*s", i, _connection->request->request_uri);

  // tm_strcpyn(_connection->request->path, _connection->request->request_uri, i);

  // tm_debug("_connection->request->path : %s\n", _connection->request->path);
  // tm_debug("_connection->request->request_uri : %s\n", _connection->request->request_uri);
  // tm_debug("strlen(_connection->request->request_uri) : %d  \n", (int)strlen(_connection->request->request_uri));
  // tm_debug("strlen(_connection->request->path) : %d\n", (int)strlen(_connection->request->path));
}
