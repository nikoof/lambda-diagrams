#include "diagram.h"

#include <nob.h>

Tree_Node *get_leftmost_atom_node(Tree_Node *node) {
  while (node != NULL) {
    while (node->kind == LAMBDA_ABSTRACTION) {
      node = node->right;
    }

    if (node->left == NULL) return node;
    node = node->left;
  }

  return NULL;
}

void diagram_from_lambda_tree_impl(Diagram *diagram, Tree *tree, Tree_Node *node, size_t *breadth, size_t *depth);
void diagram_from_lambda_tree(Diagram *diagram, Tree *tree) {
  size_t breadth = 0, depth = 0;
  diagram_from_lambda_tree_impl(diagram, tree, tree->root, &breadth, &depth);
  Tree_Node *main_node = get_leftmost_atom_node(tree->root);
  assert(main_node != NULL && "Well-formed lambda tree should have leftmost atom node");
  assert(main_node->line != NULL && "Node in complete diagram should have a corresponding line");

  size_t lowest_line_y = 0;
  for (size_t i = 0; i < diagram->count; ++i) {
    Line *line = &diagram->items[i];
    if (line->orientation == LINE_HORIZONTAL) {
      if (line->start.y > lowest_line_y) {
        lowest_line_y = line->start.y;
      }
    }
  }
  Line *main_line = main_node->line;
  main_line->end.y = lowest_line_y + 1;
}

/*
 * This is a rough description of what the following algorithm does.
 *
 * We keep track of the current breadth and depth of the diagram (i.e. width and height, but in this coordinate
 * system +\infty is down). Then, we recursively go through the lambda tree and add lines to the diagram as
 * follows:
 * - LAMBDA_ATOM:
 *       Add a vertical line from the variable's binder down to \infty (other endpoint will be set when the
 *       application line is added) and increase breadth by one (move to the left).
 * - LAMBDA_ABSTRACTION:
 *       Add a horizontal line at the current depth value and increment depth by one then recurse on the bound
 *       expression (right child)
 * - LAMBDA_APPLICATION:
 *       This one is the most convoluted. We first recurse on both children (importantly, first the left then the
 *       right). This sets up all of the atom vertical lines. Then, we find the leftmost node in each subtree of
 *       this node's children. We then get the corresponding vertical lines and connect them via a horizontal line.
 *
 *       FIXME: We compute the depth of the horizontal line by linearly searching through the array.
 *
 *       We then set the other endpoint of the bound variables and add the horizontal application line to the
 *       diagram.
 *
 * There are probably more elegant ways to do this.
 */
void diagram_from_lambda_tree_impl(Diagram *diagram, Tree *tree, Tree_Node *node, size_t *breadth, size_t *depth) {
  switch (node->kind) {
  case LAMBDA_ATOM: {
    Line *binder_line = node->binder->line;
    Line line = {
        .start = {*breadth, binder_line->start.y},
        .end = {*breadth, 1000},
        .orientation = LINE_VERTICAL,
        .kind = LAMBDA_ATOM,
    };

    *breadth += 1;

    nob_da_append(diagram, line);
    node->line = &diagram->items[diagram->count - 1];
  } break;
  case LAMBDA_ABSTRACTION: {
    Line line_ = {
        .start = {*breadth, *depth},
        .end = {1000, *depth},
        .orientation = LINE_HORIZONTAL,
        .kind = LAMBDA_ABSTRACTION,
    };

    nob_da_append(diagram, line_);
    node->line = &diagram->items[diagram->count - 1];
    Line *line = node->line;

    *depth += 1;
    diagram_from_lambda_tree_impl(diagram, tree, node->right, breadth, depth);
    *depth -= 1;

    line->end.x = *breadth - 1;
  } break;
  case LAMBDA_APPLICATION: {
    diagram_from_lambda_tree_impl(diagram, tree, node->left, breadth, depth);
    diagram_from_lambda_tree_impl(diagram, tree, node->right, breadth, depth);

    Tree_Node *left_tree = get_leftmost_atom_node(node->left);
    Tree_Node *right_tree = get_leftmost_atom_node(node->right);
    assert(left_tree != NULL && right_tree != NULL &&
           "Subtree of well-formed lambda tree should have leftmost atom node");

    Line *left = left_tree->line;
    Line *right = right_tree->line;
    assert(left != NULL && right != NULL &&
           "Lines corresponding to these lambda atoms should have been set by now.");

    /* Compute the y coordinate of the lowest line currently in the diagram. This is not the most efficient
     * approach, but it is the simplest; it could (should) be optimized at some point. */
    size_t lowest_line_y = 0;
    for (size_t i = 0; i < diagram->count; ++i) {
      Line *line = &diagram->items[i];
      if (line->orientation == LINE_HORIZONTAL) {
        if (line->start.y > lowest_line_y && line->end.x >= left->start.x) {
          lowest_line_y = line->start.y;
        }
      }
    }

    right->end.y = lowest_line_y + 1;

    Line line = {
        .start = {left->start.x, lowest_line_y + 1},
        .end = right->end,
        .orientation = LINE_HORIZONTAL,
        .kind = LAMBDA_APPLICATION,
    };
    nob_da_append(diagram, line);
    node->line = &diagram->items[diagram->count - 1];
  } break;
  }
}
