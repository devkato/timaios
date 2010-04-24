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
  connection->request->request_uri = tm_memory_allocate((size_t)TM_REQUEST_URL_MAX_LENGTH);
  connection->request->http_method = tm_memory_allocate((size_t)TM_HTTP_METHOD_MAX_LENGTH);
  connection->request->timestamp = time(NULL);
  connection->request->path = tm_memory_allocate(1024);  // @TODO define

  connection->response = tm_memory_allocate(sizeof(tm_http_response_t));
  connection->response->data = tm_memory_allocate((size_t)TM_RESPONSE_DATA_MAX_LENGTH);
  connection->response->header = tm_memory_allocate((size_t)1024);
  
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
TM_RETURN_STATUS tm_handle_read(int epfd, tm_connection_t *tm_connection, struct epoll_event ev)
{
  /* event that read data from client's request */
  tm_connection->n = tm_readv(tm_connection->fd, tm_connection->raw_data);
  
  /* @TODO manage EAGAIN status */
  if (tm_connection->n < 0) {
    if (errno == EAGAIN) {
      tm_debug("waiting data ...");
      return 1;
    } else {
      tm_perror("read");
      return 1;
    }
  }
  
  tm_connection->status = TM_CONNECTION_READ;
  tm_parse_request(tm_connection);

  ev.events = EPOLLOUT | EPOLLHUP | EPOLLET;
  
  if (epoll_ctl(epfd, EPOLL_CTL_MOD, tm_connection->fd, &ev) != 0) {
    tm_perror("epoll_ctl(EPOLL_CTL_MOD)");
    return 1;
  }
  
  return 0;
}


/* event that write response data to the client */
TM_RETURN_STATUS tm_handle_write(int epfd, tm_connection_t *tm_connection, struct epoll_event ev)
{
  int n;
  
  /* @TODO pluggable response data */
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
  char *raw_data;
  // char *req_line;
  // char *val;
  // char *log_line;
  size_t n;
  int len;
  http_parser *parser;
  
  if (_connection->raw_data == NULL || strlen(_connection->raw_data) <= 0) {
    tm_debug("invalid request\n");
    return;
  }
  
  raw_data = tm_memory_allocate(TM_REQUEST_MAX_READ_SIZE);
  tm_strcpyn(raw_data, _connection->raw_data, TM_REQUEST_MAX_READ_SIZE);
  
  len = strlen(raw_data);
  
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
  
  tm_debug("%s\n", raw_data);
  
  parser = (http_parser *)tm_memory_allocate(sizeof(http_parser));
  http_parser_init(parser, HTTP_REQUEST);
  n = http_parser_execute(parser, settings, raw_data, len);
  tm_debug("(len, n) => (%d, %d)\n", len, (int)n);

  tm_memory_free(raw_data);
  tm_memory_free(parser);
/*
  
  // printf("_connection->raw_data : %s\n", _connection->raw_data);
  
  // raw_data = tm_memory_allocate(strlen(_connection->raw_data));
  raw_data = tm_memory_allocate(TM_REQUEST_MAX_READ_SIZE);
  
  // tm_debug("_connection->raw_data : %s\n", _connection->raw_data);
  
  tm_strcpyn(raw_data, _connection->raw_data, TM_REQUEST_MAX_READ_SIZE);
  // tm_strcpy(raw_data, _connection->raw_data);
  // printf("raw_data : %s\n", raw_data);
  
  req_line = strtok(raw_data, TM_REQ_LINE_BREAK);
  if (req_line == NULL) {
    tm_debug("invalid request\n");
    return;
  }
  
  val = strtok(req_line, TM_REQ_LINE_BLANK);
  tm_strcpy(_connection->request->http_method, val);
  // tm_debug("method : %s\n", _connection->request->http_method);
  
  val = strtok(NULL, TM_REQ_LINE_BLANK);  
  tm_strcpy(_connection->request->request_uri, val);
  // tm_debug("uri : %s\n", _connection->request->request_uri);

  val = strtok(NULL, TM_REQ_LINE_BLANK);
  tm_set_path(_connection);

  log_line = tm_memory_allocate(1024);
  
  sprintf(log_line, "%d %s %s\n",
    (int)_connection->request->timestamp,
    _connection->request->http_method,
    _connection->request->request_uri
  );
  
  if (strcmp(_connection->request->request_uri, "/") != 0) {
    tm_debug("invalid request!\n");
    tm_debug("raw_data : %s\n", raw_data);
    tm_debug("method : %s\n", _connection->request->http_method);
    tm_debug("uri : %s\n", _connection->request->request_uri);
    tm_debug("path : %s\n", _connection->request->path);
  }

  tm_logger_write(log_line);
  
  tm_memory_free(log_line);
  tm_memory_free(raw_data);

  // @TODO Parse other lines ...
*/
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
