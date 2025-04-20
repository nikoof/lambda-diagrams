#include "parser.h"

#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <nob.h>

#include "util.h"

void tree_free(Tree_Node *root) {
  if (root != NULL) {
    tree_free(root->left);
    tree_free(root->right);
    free(root);
  }
}

void tree_print_graphviz(const char *output_path, const Tree_Node *root) {
  FILE *f = fopen(output_path, "w");
  assert(f != NULL);
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
  }
  nob_da_free(queue);

  fprintf(f, "}\n");
}

bool matching_left_paren(String_View term, size_t *i) {
  assert(term.data[0] == '(');

  *i = 0;
  int32_t counter = 0;
  do {
    if (term.data[*i] == '(')
      counter++;
    else if (term.data[*i] == ')')
      counter--;

    if (counter == 0) {
      return true;
    }

    *i += 1;
  } while (*i < term.count);

  return false;
}

bool matching_right_paren(String_View term, size_t *i) {
  assert(term.data[term.count - 1] == ')');

  *i = term.count - 1;
  int32_t counter = 0;
  do {
    if (term.data[*i] == '(')
      counter++;
    else if (term.data[*i] == ')')
      counter--;

    if (counter == 0) {
      return true;
    }

    *i -= 1;
  } while (0 <= *i);

  return false;
}

bool tree_parse_lambda_term(String_View term, Tree_Node *node) {
  if (term.data == NULL || term.count == 0) {
    node = NULL;
    return true;
  }

  if (term.data[0] == '(') {
    size_t i;
    if (!matching_left_paren(term, &i)) {
      return false; // unmatched paren
    }

    if (i == term.count - 1) {
      sv_chop_left(&term, 1);
      sv_chop_right(&term, 1);
    }
  }

  node->name = term;
  if (term.count == 1) {
    node->kind = LAMBDA_ATOM;
  } else if (term.data[0] == 'l') {
    String_View left = {0};
    if (!sv_try_chop_by_delim(&term, '.', &left)) {
      return false;
    }

    node->kind = LAMBDA_ABSTRACTION;

    node->left = malloc(sizeof(Tree_Node));
    assert(node->left != NULL);
    node->left->parent = node;
    node->left->name = left;

    node->right = malloc(sizeof(Tree_Node));
    assert(node->right != NULL);
    node->right->parent = node;

    if (!tree_parse_lambda_term(term, node->right)) return false;
  } else {
    node->kind = LAMBDA_APPLICATION;

    String_View right = {0};
    if (term.data[term.count - 1] == ')') {
      size_t i;
      matching_right_paren(term, &i);

      right = sv_from_parts(term.data + i, term.count - i);
    } else {
      right = sv_from_parts(term.data + term.count - 1, 1);
    }

    String_View left = sv_from_parts(term.data, term.count - right.count);

    node->left = malloc(sizeof(Tree_Node));
    assert(node->left != NULL);
    node->left->parent = node;

    if (!tree_parse_lambda_term(left, node->left)) return false;

    node->right = malloc(sizeof(Tree_Node));
    assert(node->right != NULL);
    node->right->parent = node;

    if (!tree_parse_lambda_term(right, node->right)) return false;
  }

  return true;
}
