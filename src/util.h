#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define expect(msg) assert(false && msg)

#define Vec(T)                                                                                                    \
  struct {                                                                                                        \
    T *items;                                                                                                     \
    size_t count;                                                                                                 \
    size_t capacity;                                                                                              \
  }

#define Pair(T, U)                                                                                                \
  struct {                                                                                                        \
    T left;                                                                                                       \
    U right;                                                                                                      \
  }

typedef struct {
  char *base;
  size_t ptr;
  size_t capacity;
} Arena;

bool arena_malloc_with_capacity(Arena *arena, size_t capacity);
void *arena_alloc(Arena *arena, size_t nbytes);
