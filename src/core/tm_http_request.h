/**
 * HTTP Header
 * @TODO
 *
 */
typedef struct tm_http_header {
  char *name;
  char *value;
} tm_http_header_t;


/**
 * Cookie
 * @TODO
 *
 */
typedef struct tm_http_cookie {
  char *name;
  char *domain;
  char *path;
  char *value;
  int expire;
} tm_http_cookie_t;


/**
 * HTTP Request
 *
 *
 */
typedef struct tm_http_request {
  char *request_uri;
  char *http_method;
  int http_status;
  tm_http_header_t **headers;
  tm_http_cookie_t **cookies;
} tm_http_request_t;


/**
 * HTTP Response
 *
 *
 */
typedef struct tm_http_response {
  char *data;
  tm_http_header_t **headers;
} tm_http_response_t;
