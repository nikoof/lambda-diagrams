#include <assert.h>
#include <math.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define SV_IMPLEMENTATION
#include <sv.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

int main(int argc, char **argv) {
  Tree_Node *parse_tree = calloc(1, sizeof(Tree_Node));
  assert(parse_tree != NULL);

  // (lx.ly.xxyxx)y)(lx.x)
  String_View term = sv_from_cstr(argv[1]);

  Arena main_arena = {0};
  if (!arena_malloc_with_capacity(&main_arena, (1 << ((int)log2(term.count) + 1)) * sizeof(Tree_Node))) {
    fprintf(stderr, "Could not allocate enough space for full binary tree with %zu leaves (%zu bytes).\n",
            term.count, main_arena.capacity);
  }

  if (!tree_parse_lambda_term(&main_arena, term, parse_tree)) return 1;
  tree_print_graphviz(stdout, parse_tree);

  free(main_arena.base);
  return 0;
}
