#include "parser.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <nob.h>

#include "util.h"

typedef Pair(size_t, size_t) IndexPair;
typedef Vec(IndexPair) VecIndexPair;

Tree tree_new(void) {
  Tree tree = {0};
  nob_da_append(&tree.nodes, (Tree_Node){0});
  tree.root = &tree.nodes.items[0];
  return tree;
}

void tree_free(Tree tree) {
  nob_da_free(tree.nodes);
}

bool tree_add_left_child(Tree *tree, Tree_Node *node) {
  if (node->left != NULL) {
    fprintf(stderr, "Left child of node " SV_Fmt " already exists.\n", SV_Arg(node->name));
    return false;
  }

  nob_da_append(&tree->nodes, (Tree_Node){0});
  node->left = &tree->nodes.items[tree->nodes.count - 1];
  node->left->parent = node;
  return true;
}

bool tree_add_right_child(Tree *tree, Tree_Node *node) {
  if (node->right != NULL) {
    fprintf(stderr, "Right child of node " SV_Fmt " already exists.\n", SV_Arg(node->name));
    return false;
  }

  nob_da_append(&tree->nodes, (Tree_Node){0});
  node->right = &tree->nodes.items[tree->nodes.count - 1];
  node->right->parent = node;
  return true;
}

Tree_Node *tree_get_leftmost_node(Tree *tree, Tree_Node *node) {
  while (node != NULL) {
    if (node->left == NULL) return node;
    node = node->left;
  }

  return NULL;
}

Tree_Node *tree_get_rightmost_node(Tree *tree, Tree_Node *node) {
  while (node != NULL) {
    if (node->right == NULL) return node;
    node = node->right;
  }

  return NULL;
}

bool compute_matching_parens(const char *term, VecIndexPair *pairs);
bool tree_parse_lambda_term_impl(Tree *tree, VecIndexPair paren_pairs, const char *term, size_t l, size_t r,
                                 Tree_Node *node, Tree_Node **variable_table);

bool tree_parse_lambda_term(Tree *tree, const char *term) {
  VecIndexPair paren_pairs = {0};
  if (!compute_matching_parens(term, &paren_pairs)) return false;
  Tree_Node *variable_table[256] = {0};

  bool retval =
      tree_parse_lambda_term_impl(tree, paren_pairs, term, 0, strlen(term) - 1, tree->root, variable_table);

  nob_da_free(paren_pairs);
  return retval;
}

void tree_print_graphviz(FILE *f, const Tree_Node *root, bool include_binders) {
  fprintf(f, "strict graph {\n");

  Vec(Tree_Node *) queue = {0};
  nob_da_append(&queue, (Tree_Node *)root);

  size_t i = 0;
  while (i < queue.count && root != NULL) {
    Tree_Node *node = queue.items[i++];

    node->i = i;
    fprintf(f, "\t%zu [label=\"" SV_Fmt "\"]\n", node->i, SV_Arg(node->name));

    if (node->left != NULL) {
      nob_da_append(&queue, node->left);
    }

    if (node->right != NULL) {
      nob_da_append(&queue, node->right);
    }
  }
  queue.count = 0;

  nob_da_append(&queue, (Tree_Node *)root);
  i = 0;
  while (i < queue.count && root != NULL) {
    Tree_Node *node = queue.items[i++];

    if (node->left != NULL) {
      nob_da_append(&queue, node->left);
      fprintf(f, "\t%zu -- %zu\n", node->i, node->left->i);
    }

    if (node->right != NULL) {
      nob_da_append(&queue, node->right);
      fprintf(f, "\t%zu -- %zu\n", node->i, node->right->i);
    }

    if (include_binders && node->kind == LAMBDA_ATOM && node->binder != NULL) {
      fprintf(f, "\t%zu -- %zu\n", node->i, node->binder->i);
    }
  }
  nob_da_free(queue);

  fprintf(f, "}\n");
}

bool compute_matching_parens(const char *term, VecIndexPair *pairs) {
  Vec(size_t) stack = {0};

  for (size_t i = 0; i < strlen(term); ++i) {
    if (term[i] == '(') {
      nob_da_append(&stack, i);
    } else if (term[i] == ')') {
      if (stack.count == 0) {
        fprintf(stderr, "Unmatched ')' in %s\n", term);
        fprintf(stderr, "%*s^\n", 17 + (int)i, "");
        return false;
      }
      IndexPair p = {
          .left = stack.items[--stack.count],
          .right = i,
      };
      nob_da_append(pairs, p);
    }
  }

  if (stack.count != 0) {
    fprintf(stderr, "Unmatched '(' in %s\n", term);
    fprintf(stderr, "%*s^\n", 17 + (int)stack.items[stack.count - 1], "");
    nob_da_free(stack);
    return false;
  }

  return true;
}

size_t matching_right_paren(VecIndexPair pairs, size_t self) {
  for (size_t i = 0; i < pairs.count; ++i) {
    if (pairs.items[i].left == self) {
      return pairs.items[i].right;
    }
  }

  return -1;
}

size_t matching_left_paren(VecIndexPair pairs, size_t self) {
  for (size_t i = 0; i < pairs.count; ++i) {
    if (pairs.items[i].right == self) {
      return pairs.items[i].left;
    }
  }

  return -1;
}

bool tree_parse_lambda_term_impl(Tree *tree, VecIndexPair paren_pairs, const char *term, size_t l, size_t r,
                                 Tree_Node *node, Tree_Node **variable_table) {
  size_t len = r - l + 1;
  if (term == NULL || len == 0) {
    node = NULL;
    return true;
  }

  if (term[l] == '(') {
    if (matching_right_paren(paren_pairs, l) == r) {
      l += 1;
      r -= 1;
      len -= 2;
    }
  }

  node->name = sv_from_parts(term + l, len);
  if (len == 1) {
    node->kind = LAMBDA_ATOM;
    node->binder = variable_table[(size_t)*node->name.data];
    assert(node->binder != NULL &&
           "This atom's binder should have been recorded in the variable table before we got to it.");
  } else if (term[l] == 'l') {
    node->kind = LAMBDA_ABSTRACTION;

    size_t i = 0;
    while (l + i < r && term[l + i] != '.') {
      i += 1;
    }

    if (i >= len) {
      fprintf(stderr, "Could not parse lambda term. Unmatched 'l' in lambda abstraction in %s\n", term);
      fprintf(stderr, "%*s^", 68 + (int)l, "");
      return false;
    }

    tree_add_left_child(tree, node);
    tree_add_right_child(tree, node);

    node->left->name = sv_from_parts(term + l + 1, 1);

    // NOTE: This check does not allow to bind the same variable name multiple times in the same bound expression
    // (but in disjoint abstractions).

    /* if (variable_table[(size_t)*node->left->name.data] != 0) { */
    /*   fprintf(stderr, "Variable '" SV_Fmt "' already bound.\n", SV_Arg(node->left->name)); */
    /*   return false; */
    /* } */
    variable_table[(size_t)*node->left->name.data] = node;

    if (!tree_parse_lambda_term_impl(tree, paren_pairs, term, l + i + 1, r, node->right, variable_table))
      return false;
  } else {
    node->kind = LAMBDA_APPLICATION;

    size_t i = (term[r] == ')') ? matching_left_paren(paren_pairs, r) : r;

    tree_add_left_child(tree, node);
    tree_add_right_child(tree, node);
    if (!tree_parse_lambda_term_impl(tree, paren_pairs, term, l, i - 1, node->left, variable_table)) return false;
    if (!tree_parse_lambda_term_impl(tree, paren_pairs, term, i, r, node->right, variable_table)) return false;
  }

  return true;
}
