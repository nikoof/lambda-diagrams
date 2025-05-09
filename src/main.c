#include <assert.h>
#include <stddef.h>

#include "parser.h"

#include <raylib.h>
#include <raymath.h>

#define SV_IMPLEMENTATION
#include <sv.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

typedef enum {
  LINE_HORIZONTAL,
  LINE_VERTICAL,
} Line_Orientation;

typedef struct {
  Vector2 start;
  Vector2 end;
} Line;

typedef struct {
  Vec(Line) lines;
} Diagram;

bool diagram_from_lambda_tree(Diagram *diagram, Tree *tree, Tree_Node *node, size_t *curr_x, size_t *curr_y) {
  switch (node->kind) {
  case LAMBDA_ATOM: {
    Line line = {
        .start = {*curr_x, ((Line *)node->binder->user_data)->start.y},
        .end = {*curr_x, ((Line *)node->parent->user_data)->start.y},
    };

    if (node->parent == node->binder) {
      line.end.y += 1;
    }
    nob_da_append(&diagram->lines, line);
    *curr_x -= 1;
  } break;
  case LAMBDA_ABSTRACTION: {
    Line line = {
        .start = {0, *curr_y},
        .end = {10000, *curr_y},
    };
    nob_da_append(&diagram->lines, line);
    node->user_data = &diagram->lines.items[diagram->lines.count - 1];
    *curr_y += 1;

    diagram_from_lambda_tree(diagram, tree, node->right, curr_x, curr_y);
  } break;
  case LAMBDA_APPLICATION: {
    Line line = {
        .start = {0, *curr_y},
        .end = {0, *curr_y},
    };
    nob_da_append(&diagram->lines, line);
    node->user_data = &diagram->lines.items[diagram->lines.count - 1];
    size_t saved_y = *curr_y;
    diagram_from_lambda_tree(diagram, tree, node->right, curr_x, curr_y);
    *curr_y = saved_y;
    ((Line *)node->user_data)->end.x = *curr_x + 1;
    ((Line *)node->user_data)->end.y = *curr_y;
    ((Line *)node->user_data)->start.x = *curr_x;
    ((Line *)node->user_data)->start.y = *curr_y;
    *curr_y += 1;
    diagram_from_lambda_tree(diagram, tree, node->left, curr_x, curr_y);
  } break;
  }

  return true;
}

int main(int argc, char **argv) {
  const char *term = "(lx.xxxxxxx)(ly.yy)";

  Tree tree = tree_new();
  if (!tree_parse_lambda_term(&tree, term)) return 1;
  tree_print_graphviz(stdout, tree.root);

  Diagram diagram = {0};
  size_t x = 10, y = 0;
  if (!diagram_from_lambda_tree(&diagram, &tree, tree.root, &x, &y)) return 1;
  for (size_t i = 0; i < diagram.lines.count; ++i) {
    Line *line = &diagram.lines.items[i];
    printf("Horiz({%zu, y=%zu}, {x=%zu, y=%zu})\n", line->start.x, line->start.y, line->end.x, line->end.y);
  }

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "Lambda Diagrams");

  const size_t SCALE = 20, WIDTH = 5;
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    for (size_t i = 0; i < diagram.lines.count; ++i) {
      Line *line = &diagram.lines.items[i];
      DrawLineEx(Vector2AddValue(Vector2Scale(line->start, SCALE), 50),
                 Vector2AddValue(Vector2Scale(line->end, SCALE), 50), WIDTH, WHITE);
    }

    EndDrawing();
  }

  CloseWindow();

  tree_free(tree);
  return 0;
}
