/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_LOGGER_
#define _TIMAIOS_HEADER_LOGGER_

int tm_open_file(const char *filename, int flags, int mode);

int tm_logger_init();

void tm_logger_write(char *line);


#endif
