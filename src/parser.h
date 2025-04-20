#pragma once

#include <sv.h>

typedef enum {
  LAMBDA_ATOM,
  LAMBDA_ABSTRACTION,
  LAMBDA_APPLICATION,
} Lambda_Expr_Kind;

typedef struct Tree_Node {
  struct Tree_Node *parent;
  struct Tree_Node *left;
  struct Tree_Node *right;

  size_t i;

  Lambda_Expr_Kind kind;
  String_View name;
} Tree_Node;

void tree_print_graphviz(const char *output_path, const Tree_Node *root);
void tree_free(Tree_Node *root);
bool tree_parse_lambda_term(String_View term, Tree_Node *node);
