#pragma once

#include <stdio.h>
#include <sv.h>

#include "util.h"

typedef enum {
  LAMBDA_ATOM,
  LAMBDA_ABSTRACTION,
  LAMBDA_APPLICATION,
} Lambda_Expr_Kind;

typedef struct Tree_Node {
  struct Tree_Node *left;
  struct Tree_Node *right;

  // points to this atom's binder if kind == LAMBDA_ATOM, else NULL
  struct Tree_Node *binder;

  Lambda_Expr_Kind kind;
  const char *name;

  void *user_data;
} Tree_Node;

typedef struct {
  Tree_Node *root;
  Vec(Tree_Node) nodes; // order here should not matter
  Vec(char) string_storage;
} Tree;

typedef Vec(Tree_Node *) Vec_Tree_Node;

Tree tree_new(void);
bool tree_parse_lambda_term(Tree *tree, const char *term);
void tree_free(Tree tree);

bool tree_add_left_child(Tree *tree, Tree_Node *node);
bool tree_add_right_child(Tree *tree, Tree_Node *node);

Tree_Node *tree_get_leftmost_node(Tree *tree, Tree_Node *node);
Tree_Node *tree_get_rightmost_node(Tree *tree, Tree_Node *node);

void tree_print_graphviz(FILE *stream, const Tree_Node *root, bool include_binders);
