#include "timaios.h"

/**
 *
 *
 *
 */
void tm_memory_reset(void *_pointer, size_t _size)
{
  memset(_pointer, 0, _size);
}


/**
 *
 *
 *
 */
void *tm_memory_allocate(size_t _size)
{
  void *p = malloc(_size);
  return p;
}


/**
 *
 *
 *
 */
void tm_memory_free(void *_pointer)
{
  free(_pointer);
}
