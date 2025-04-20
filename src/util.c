#include "util.h"

bool arena_malloc_with_capacity(Arena *arena, size_t capacity) {
  arena->base = malloc(capacity);
  arena->ptr = 0;
  arena->capacity = capacity;
  return arena->base != NULL;
}

void *arena_alloc(Arena *arena, size_t nbytes) {
  if (arena->ptr + nbytes >= arena->capacity) {
    return NULL;
  }

  void *ptr = arena->base + arena->ptr;
  arena->ptr += nbytes;
  return ptr;
}
