#pragma once

#include <assert.h>
#include <stddef.h>

#include "parser.h"

typedef struct {
  size_t x, y;
} Usize2;

typedef enum {
  LINE_HORIZONTAL,
  LINE_VERTICAL,
} Line_Orientation;

typedef struct {
  Usize2 start;
  Usize2 end;
  Line_Orientation orientation;
  Lambda_Expr_Kind kind;
} Line;

typedef Vec(Line) Diagram;

void diagram_from_lambda_tree(Diagram *diagram, Tree *tree);
