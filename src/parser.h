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
  struct Tree_Node *parent;
  struct Tree_Node *left;
  struct Tree_Node *right;
  struct Tree_Node *binder;

  size_t i;

  Lambda_Expr_Kind kind;
  String_View name;

  void *user_data;
} Tree_Node;

typedef struct {
  Vec(Tree_Node) nodes;
  Tree_Node *root;
} Tree;

typedef Vec(Tree_Node *) Vec_Tree_Node;

Tree tree_new(void);
bool tree_parse_lambda_term(Tree *tree, const char *term);
void tree_free(Tree tree);

bool tree_add_right_child(Tree *tree, Tree_Node *node);
bool tree_add_right_child(Tree *tree, Tree_Node *node);

void tree_print_graphviz(FILE *stream, const Tree_Node *root);
