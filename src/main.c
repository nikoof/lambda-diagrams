#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define SV_IMPLEMENTATION
#include <sv.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

int main(void) {
  Tree_Node *parse_tree = calloc(1, sizeof(Tree_Node));
  assert(parse_tree != NULL);

  String_View term = sv_from_cstr("(lx.ly.xxy(xx)y)(lx.x)");
  if (!tree_parse_lambda_term(term, parse_tree)) return 1;
  tree_print_graphviz("tree.gv", parse_tree);

  return 0;
}
