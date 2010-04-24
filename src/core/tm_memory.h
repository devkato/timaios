/**
 * @TODO comment
 *
 *
 */

#ifndef _TIMAIOS_HEADER_MEMORY_
#define _TIMAIOS_HEADER_MEMORY_

/**
 *
 *
 *
 */
void tm_memory_reset(void *_pointer, size_t _size);


/**
 *
 *
 *
 */
void *tm_memory_allocate(size_t _size);


/**
 *
 *
 *
 */
void tm_memory_free(void *_pointer);


#endif
