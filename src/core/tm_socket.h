/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_SOCKET_
#define _TIMAIOS_HEADER_SOCKET_

/**
 *
 *
 *
 */
TM_SERVER_SOCKET tm_initialize_socket();

void tm_setnonblocking(int _fd);

int tm_readv(int _fd, char *data);

int tm_writev(int _fd, struct iovec _iovec[], int buffernum);

int tm_write_response_data(tm_connection_t *_connection);


#endif
