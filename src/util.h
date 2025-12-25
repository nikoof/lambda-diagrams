#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define max(a, b)                                                                                                 \
  ({                                                                                                              \
    __typeof__(a) _a = (a);                                                                                       \
    __typeof__(b) _b = (b);                                                                                       \
    _a > _b ? _a : _b;                                                                                            \
  })

#define min(a, b)                                                                                                 \
  ({                                                                                                              \
    __typeof__(a) _a = (a);                                                                                       \
    __typeof__(b) _b = (b);                                                                                       \
    _a < _b ? _a : _b;                                                                                            \
  })

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

#define SB_Fmt     "%.*s"
#define SB_Arg(sv) (int)(sv).count, (sv).items

size_t str_hash(const char* str);

typedef struct {
  char *base;
  size_t ptr;
  size_t capacity;
} Arena;

bool arena_malloc_with_capacity(Arena *arena, size_t capacity);
void *arena_alloc(Arena *arena, size_t nbytes);
