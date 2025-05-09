#include <assert.h>
#include <stddef.h>

#include "parser.h"

#include <raylib.h>
#include <raymath.h>

#define SV_IMPLEMENTATION
#include <sv.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

typedef struct {
  size_t x, y;
} Usize2;

typedef enum {
  LINE_HORIZONTAL,
  LINE_VERTICAL,
} Line_Orientation;

typedef struct {
  Usize2 start;
  Usize2 end;
  Line_Orientation orientation;
  Lambda_Expr_Kind kind;
} Line;

typedef struct {
  Vec(Line) lines;
} Diagram;

bool diagram_from_lambda_tree(Diagram *diagram, Tree *tree, Tree_Node *node, size_t *curr_x, size_t *curr_y) {
  switch (node->kind) {
  case LAMBDA_ATOM: {
    Line *binder_line = node->binder->user_data;
    Line line = {
        .start = {*curr_x, binder_line->start.y},
        .end = {*curr_x, 1000},
        .orientation = LINE_VERTICAL,
        .kind = LAMBDA_ATOM,
    };

    *curr_x += 1;

    nob_da_append(&diagram->lines, line);
    node->user_data = &diagram->lines.items[diagram->lines.count - 1];
  } break;
  case LAMBDA_ABSTRACTION: {
    Line line_ = {
        .start = {*curr_x, *curr_y},
        .end = {1000, *curr_y},
        .orientation = LINE_HORIZONTAL,
        .kind = LAMBDA_ABSTRACTION,
    };
    *curr_y += 1;

    nob_da_append(&diagram->lines, line_);
    node->user_data = &diagram->lines.items[diagram->lines.count - 1];
    Line *line = node->user_data;
    diagram_from_lambda_tree(diagram, tree, node->right, curr_x, curr_y);
    /* *curr_y -= 1; // FIXME: doesn't account for deeper abstractions in terms to the left */
    line->end.x = *curr_x - 1;
  } break;
  case LAMBDA_APPLICATION: {
    diagram_from_lambda_tree(diagram, tree, node->left, curr_x, curr_y);
    diagram_from_lambda_tree(diagram, tree, node->right, curr_x, curr_y);

    Tree_Node *left_expr_node = node->left;
    while (left_expr_node->kind == LAMBDA_ABSTRACTION) {
      left_expr_node = left_expr_node->right;
    }
    Tree_Node *right_expr_node = node->right;
    while (right_expr_node->kind == LAMBDA_ABSTRACTION) {
      right_expr_node = right_expr_node->right;
    }
    Line *left = tree_get_leftmost_node(tree, left_expr_node)->user_data;
    Line *right = tree_get_leftmost_node(tree, right_expr_node)->user_data;
    assert(left != NULL && right != NULL);

    right->end.y = *curr_y;

    Line line_ = {
        .start = {left->start.x, *curr_y},
        .end = right->end,
        .orientation = LINE_HORIZONTAL,
        .kind = LAMBDA_APPLICATION,
    };
    nob_da_append(&diagram->lines, line_);
    node->user_data = &diagram->lines.items[diagram->lines.count - 1];

    *curr_y += 1;
  } break;
  }

  return true;
}

int main(int argc, char **argv) {
  /* const char *term = "lf.lx.f(f(f(f(f(fx)))))"; */
  /* const char *term = "(lx.xx)(lx.xx)"; */
  /* const char *term = "ln.lf.n(lf.ln.n(f(lf.lx.nf(fx))))(lx.f)(lx.x)"; */
  const char *term = "ln.lf.lx.n(lg.lh.h(gf))(lu.x)(lu.u)";

  Tree tree = tree_new();
  if (!tree_parse_lambda_term(&tree, term)) return 1;

  FILE *tree_gv = fopen("lambda_tree.gv", "w");
  assert(tree_gv != NULL && "Failed to open file");
  tree_print_graphviz(tree_gv, tree.root);

  Diagram diagram = {0};
  size_t x = 0, y = 0;
  if (!diagram_from_lambda_tree(&diagram, &tree, tree.root, &x, &y)) return 1;
  for (size_t i = 0; i < diagram.lines.count; ++i) {
    Line *line = &diagram.lines.items[i];
    char *kind;
    switch (line->kind) {
    case LAMBDA_ATOM:
      kind = "    Atom";
      break;
    case LAMBDA_ABSTRACTION:
      kind = "Abstract";
      break;
    case LAMBDA_APPLICATION:
      kind = "Applicat";
      break;
    }
    printf("%s({x=%zu, y=%zu}, {x=%zu, y=%zu})\n", kind, line->start.x, line->start.y, line->end.x, line->end.y);
  }

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "Lambda Diagrams");

  const size_t SCALE = 20, WIDTH = 5;
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    for (size_t i = 0; i < diagram.lines.count; ++i) {
      Line *line = &diagram.lines.items[i];
      if (line->kind == LAMBDA_ABSTRACTION) {
        Vector2 start = {.x = line->start.x - 0.4, .y = line->start.y};
        Vector2 end = {.x = line->end.x + 0.4, .y = line->end.y};
        DrawLineEx(Vector2AddValue(Vector2Scale(start, SCALE), 50), Vector2AddValue(Vector2Scale(end, SCALE), 50),
                   WIDTH, WHITE);
      } else {
        Vector2 start = {.x = line->start.x, .y = line->start.y};
        Vector2 end = {.x = line->end.x, .y = line->end.y};
        DrawLineEx(Vector2AddValue(Vector2Scale(start, SCALE), 50), Vector2AddValue(Vector2Scale(end, SCALE), 50),
                   WIDTH, WHITE);
      }
    }

    EndDrawing();
  }

  CloseWindow();

  tree_free(tree);
  return 0;
}
