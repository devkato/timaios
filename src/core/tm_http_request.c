#include "timaios.h"

/**
 *
 *
 *
 */
tm_http_request_t *create_request(char *_request_string)
{
  tm_http_request_t *request;
  request = (tm_http_request_t *)malloc(sizeof(tm_http_request_t));
  
  return request;
}


/**
 *
 *
 *
 */
void destroy_request(tm_http_request_t *_request)
{
  free(_request);
}

