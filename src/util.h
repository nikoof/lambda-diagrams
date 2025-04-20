#pragma once

#include <stddef.h>

#define Vec(T)                                                                                                    \
  struct {                                                                                                        \
    T *items;                                                                                                     \
    size_t count;                                                                                                 \
    size_t capacity;                                                                                              \
  }
