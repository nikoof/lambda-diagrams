#include <assert.h>
#include <raylib.h>
#include <stddef.h>

#include "parser.h"

#define SV_IMPLEMENTATION
#include <sv.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

int main(int argc, char **argv) {
  const char *term = "(lx.ly.xxy(xx)y)(lx.x)";

  Tree tree = tree_new();
  if (!tree_parse_lambda_term(&tree, term)) return 1;
  tree_print_graphviz(stdout, tree.root);

  tree_free(tree);
  return 0;
}
