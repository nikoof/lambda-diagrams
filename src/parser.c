#include "parser.h"

#include <assert.h>
#include <raylib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <nob.h>

#include "util.h"

bool tree_add_left_child(Arena *arena, Tree_Node *node) {
  node->left = arena_alloc(arena, sizeof(Tree_Node));
  if (node->left == NULL) {
    fprintf(stderr, "Could not allocate %zu bytes for tree node. Full arena (%zu/%zu).\n", sizeof(Tree_Node),
            arena->ptr, arena->capacity);
    return false;
  }

  memset(node->left, 0, sizeof(Tree_Node));
  node->left->parent = node;
  return true;
}

bool tree_add_right_child(Arena *arena, Tree_Node *node) {
  node->right = arena_alloc(arena, sizeof(Tree_Node));
  if (node->right == NULL) {
    fprintf(stderr, "Could not allocate %zu bytes for tree node. Full arena (%zu/%zu).\n", sizeof(Tree_Node),
            arena->ptr, arena->capacity);
    return false;
  }

  memset(node->right, 0, sizeof(Tree_Node));
  node->right->parent = node;
  return true;
}

void tree_print_graphviz(FILE *f, const Tree_Node *root) {
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

typedef Pair(size_t, size_t) IndexPair;
typedef Vec(IndexPair) VecIndexPair;
bool compute_matching_parens(String_View term, VecIndexPair *pairs) {
  Vec(size_t) stack = {0};

  for (size_t i = 0; i < term.count; ++i) {
    if (term.data[i] == '(') {
      nob_da_append(&stack, i);
    } else if (term.data[i] == ')') {
      if (stack.count == 0) {
        fprintf(stderr, "Unmatched ')' in " SV_Fmt "\n", SV_Arg(term));
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
    fprintf(stderr, "Unmatched '(' in " SV_Fmt "\n", SV_Arg(term));
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

// TODO: Refactor this. Maybe using String_View is not ideal here. It is easier
// to just precompute all the parenthesis pairs and use indices.
bool tree_parse_lambda_term(Arena *arena, String_View term, Tree_Node *node) {
  if (term.data == NULL || term.count == 0) {
    node = NULL;
    return true;
  }

  VecIndexPair paren_pairs = {0};
  if (!compute_matching_parens(term, &paren_pairs)) return false;

  if (term.data[0] == '(') {
    if (matching_right_paren(paren_pairs, 0) == term.count - 1) {
      sv_chop_left(&term, 1);
      sv_chop_right(&term, 1);

      paren_pairs.count = 0;
      if (!compute_matching_parens(term, &paren_pairs)) return false;
    }
  }

  node->name = term;
  if (term.count == 1) {
    node->kind = LAMBDA_ATOM;
  } else if (term.data[0] == 'l') {
    node->kind = LAMBDA_ABSTRACTION;

    String_View left = {0};
    if (!sv_try_chop_by_delim(&term, '.', &left)) {
      fprintf(stderr, "Could not parse lambda term. Unmatched 'l' in lambda abstraction in " SV_Fmt "\n",
              SV_Arg(term));
      return false;
    }

    tree_add_left_child(arena, node);
    tree_add_right_child(arena, node);

    node->left->name = left;
    if (!tree_parse_lambda_term(arena, term, node->right)) return false;
  } else {
    node->kind = LAMBDA_APPLICATION;

    String_View right = {0};
    if (term.data[term.count - 1] == ')') {
      size_t i = matching_left_paren(paren_pairs, term.count - 1);
      right = sv_from_parts(term.data + i, term.count - i);
    } else {
      right = sv_from_parts(term.data + term.count - 1, 1);
    }
    String_View left = sv_from_parts(term.data, term.count - right.count);

    tree_add_left_child(arena, node);
    tree_add_right_child(arena, node);
    if (!tree_parse_lambda_term(arena, left, node->left)) return false;
    if (!tree_parse_lambda_term(arena, right, node->right)) return false;
  }

  nob_da_free(paren_pairs);
  return true;
}
