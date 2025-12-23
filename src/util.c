#include "util.h"

#include <string.h>
#include <stdio.h>

size_t str_hash(const char* s) {
  // \sum_{i=0}^{n} s[i] * p^i mod m
  // where s \in \Sigma, p >= |\Sigma| is prime
  // and m is a large prime

  if (s == NULL) return 0;

  const size_t p = 53;
  const size_t m = 1e9 + 9;

  size_t pow_p = 1;
  size_t h = 0;
  for (size_t i = 0; i < strlen(s); ++i) {
    h += ((size_t)(s[i] - 'a' + 1) * pow_p) % m;
    pow_p = (pow_p * p) % m;
  }

  return h;
}

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
