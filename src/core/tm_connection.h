
/**
 *
 *
 *
 */
typedef struct tm_connection {
  int fd;
  char raw_data[TM_REQUEST_MAX_READ_SIZE];
  tm_http_request_t *request;
  int status;
  int n;
  int method;
} tm_connection_t;

tm_connection_t *tm_create_connection(int _fd);
void tm_destroy_connection(tm_connection_t *connection);

void tm_parse_request(tm_connection_t *_connection);
