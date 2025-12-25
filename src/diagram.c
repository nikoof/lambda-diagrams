#include "diagram.h"

#include <nob.h>
#include <raylib.h>
#include <raymath.h>

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

void diagram_from_lambda_tree_impl(Diagram *diagram, Tree_Node *node, size_t *breadth, size_t *depth);
void diagram_from_lambda_tree(Diagram *diagram, Tree_Node *tree) {
  size_t breadth = 0, depth = 0;
  diagram_from_lambda_tree_impl(diagram, tree, &breadth, &depth);
  Tree_Node *main_node = get_leftmost_atom_node(tree);
  assert(main_node != NULL && "Well-formed lambda tree should have leftmost atom node");
  assert(main_node->user_data != NULL && "Node in complete diagram should have a corresponding line");

  size_t lowest_line_y = 0;
  for (size_t i = 0; i < diagram->count; ++i) {
    Line *line = &diagram->items[i];
    if (line->orientation == LINE_HORIZONTAL) {
      if (line->start.y > lowest_line_y) {
        lowest_line_y = line->start.y;
      }
    }
  }
  Line *main_line = main_node->user_data;
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
void diagram_from_lambda_tree_impl(Diagram *diagram, Tree_Node *node, size_t *breadth, size_t *depth) {
  switch (node->kind) {
  case LAMBDA_ATOM: {
    Line *binder_line = node->binder->user_data;
    Line line = {
        .start = {*breadth, binder_line->start.y},
        .end = {*breadth, 1000},
        .orientation = LINE_VERTICAL,
        .kind = LAMBDA_ATOM,
    };

    *breadth += 1;

    nob_da_append(diagram, line);
    node->user_data = &diagram->items[diagram->count - 1];
  } break;
  case LAMBDA_ABSTRACTION: {
    Line line_ = {
        .start = {*breadth, *depth},
        .end = {1000, *depth},
        .orientation = LINE_HORIZONTAL,
        .kind = LAMBDA_ABSTRACTION,
    };

    nob_da_append(diagram, line_);
    node->user_data = &diagram->items[diagram->count - 1];
    Line *line = node->user_data;

    *depth += 1;
    diagram_from_lambda_tree_impl(diagram, node->right, breadth, depth);
    *depth -= 1;

    line->end.x = *breadth - 1;
  } break;
  case LAMBDA_APPLICATION: {
    diagram_from_lambda_tree_impl(diagram, node->left, breadth, depth);
    diagram_from_lambda_tree_impl(diagram, node->right, breadth, depth);

    Tree_Node *left_tree = get_leftmost_atom_node(node->left);
    Tree_Node *right_tree = get_leftmost_atom_node(node->right);
    assert(left_tree != NULL && right_tree != NULL &&
           "Subtree of well-formed lambda tree should have leftmost atom node");

    Line *left = left_tree->user_data;
    Line *right = right_tree->user_data;
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
    node->user_data = &diagram->items[diagram->count - 1];
  } break;
  }
}

RenderTexture2D diagram_to_raylib_texture(Diagram diagram, size_t width, size_t height, size_t line_width,
                                          double serif_multiplier) {
  RenderTexture2D texture = LoadRenderTexture(width, height);
  size_t diagram_width = 0, diagram_height = 0;
  for (size_t i = 0; i < diagram.count; ++i) {
    if (diagram.items[i].orientation == LINE_HORIZONTAL) {
      diagram_width = max(diagram_width, diagram.items[i].end.x);
    }

    if (diagram.items[i].orientation == LINE_VERTICAL) {
      diagram_height = max(diagram_height, diagram.items[i].end.y);
    }
  }

  const Vector2 scale = {
      .x = roundf((float)width / (diagram_width + 1)),
      .y = roundf((float)height / (diagram_height + 1)),
  };

  BeginTextureMode(texture);

  for (size_t i = 0; i < diagram.count; ++i) {
    Line *line = &diagram.items[i];

    Vector2 start = {.x = line->start.x, .y = diagram_height - line->start.y};
    Vector2 end = {.x = line->end.x, .y = diagram_height - line->end.y};

    start = Vector2Multiply(start, scale);
    end = Vector2Multiply(end, scale);

    if (line->kind == LAMBDA_ABSTRACTION) {
      start.x -= serif_multiplier * line_width;
      end.x += serif_multiplier * line_width;
    }

    if (line->orientation == LINE_VERTICAL) {
      end.y -= 0.5 * line_width;
    }

    start = Vector2Add(start, (Vector2){3.0 * line_width, 0.0});
    end = Vector2Add(end, (Vector2){3.0 * line_width, 0.0});

    DrawLineEx(start, end, line_width, WHITE);
  }
  EndTextureMode();

  return texture;
}
