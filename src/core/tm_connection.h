
/**
 *
 *
 *
 */
typedef struct tm_connection {
  int fd;
  char *raw_data;
  tm_http_request_t *request;
  int status;
  int n;
} tm_connection_t;

tm_connection_t *tm_create_connection(int _fd);
void tm_destroy_connection(tm_connection_t *connection);

void tm_create_response_data(tm_connection_t *_connection);

void tm_parse_request(tm_connection_t *_connection);
