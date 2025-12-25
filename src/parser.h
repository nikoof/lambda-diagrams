#pragma once

#include <stdio.h>
#include <sv.h>
#include <nob.h>

#include "util.h"

typedef enum {
  LAMBDA_ATOM,
  LAMBDA_ABSTRACTION,
  LAMBDA_APPLICATION,
} Lambda_Expr_Kind;

typedef struct Tree_Node {
  struct Tree_Node *left;
  struct Tree_Node *right;

  Lambda_Expr_Kind kind;
  char atom; // name of the (bound) atom
  struct Tree_Node *binder; // points to this atom's binder if kind == LAMBDA_ATOM, else NULL

  void *user_data;
} Tree_Node;

bool tree_parse_lambda_term(Tree_Node *tree, const char *term);
void tree_free(Tree_Node *tree);

bool tree_add_left_child(Tree_Node *node);
bool tree_add_right_child(Tree_Node *node);

Tree_Node *tree_get_leftmost_node(Tree_Node *node);
Tree_Node *tree_get_rightmost_node(Tree_Node *node);

void tree_node_label(Nob_String_Builder *sb, Tree_Node *node);
void tree_print_graphviz(FILE *stream, const Tree_Node *root, bool include_binders);
