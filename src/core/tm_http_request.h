/**
 * HTTP Requestヘッダ構造体
 *
 *
 */
typedef struct tm_http_request_headers {
  char *name;
  char *value;
} tm_http_request_headers_t;


/**
 * HTTP Responseヘッダ構造体
 *
 *
 */
typedef struct tm_http_response_headers {
  char *name;
  char *value;
} tm_http_response_headers_t;


/**
 * Cookie構造体
 *
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
 * HTTPリクエスト構造体
 *
 *
 */
typedef struct tm_http_request {
  char *request_uri;
  char *http_method;
  int http_status;
  char *response_data;
  tm_http_request_headers_t **request_headers;
  tm_http_response_headers_t **response_headers;
  tm_http_cookie_t **cookies;
} tm_http_request_t;


/**
 *
 *
 *
 */
tm_http_request_t *create_request(char *_request_string);


/**
 *
 *
 *
 */
void destroy_request(tm_http_request_t *_request);
