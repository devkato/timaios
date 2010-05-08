/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_HTTP_
#define _TIMAIOS_HEADER_HTTP_

int tm_http_callback_for_header_filed(http_parser *p, const char *buf, size_t len);
int tm_http_callback_for_header_value(http_parser *p, const char *buf, size_t len);
int tm_http_callback_for_path(http_parser *p, const char *buf, size_t len);
int tm_http_callback_for_url(http_parser *p, const char *buf, size_t len);
int tm_http_callback_for_query_string(http_parser *p, const char *buf, size_t len);


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
  char *path;
  char *http_method;
  int http_status;
  tm_http_header_t **headers;
  tm_http_cookie_t **cookies;
  time_t timestamp;
} tm_http_request_t;


/**
 * HTTP Response
 *
 *
 */
typedef struct tm_http_response {
  char *data;
  // tm_http_header_t **headers;
  char *header; /* @TODO make headers as struct array */
  int status;
} tm_http_response_t;


const char *tm_http_from_status_code_to_string(int _status_code);


#endif
