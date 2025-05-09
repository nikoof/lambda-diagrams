#include <assert.h>
#include <stddef.h>

#include <raylib.h>
#include <raymath.h>

#include "diagram.h"
#include "parser.h"

#define SV_IMPLEMENTATION
#include <sv.h>
#define NOB_IMPLEMENTATION
#include <nob.h>

int main(int argc, char **argv) {
  /* const char *term = "lf.lx.f(f(f(f(f(fx)))))"; */
  /* const char *term = "(lx.xx)(lx.xx)"; */
  /* const char *term = "ln.lf.n(lf.ln.n(f(lf.lx.nf(fx))))(lx.f)(lx.x)"; */
  /* const char *term = "ln.lf.n(lc.la.lb.cb(lx.a(bx)))(lx.ly.x)(lx.x)f"; */
  /* const char *term = "ln.lf.lx.n(lg.lh.h(gf))(lu.x)(lu.u)"; */
  /* const char *term = "lf.(lx.xx)(lx.f(xx))"; */
  const char *term = "lf.(lx.xx)f";

  Tree tree = tree_new();
  if (!tree_parse_lambda_term(&tree, term)) return 1;

  tree_print_graphviz(stdout, tree.root, false);

  Diagram diagram = {0};
  diagram_from_lambda_tree(&diagram, &tree);

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "Lambda Diagrams");

  const size_t WIDTH = 4;
  const Vector2 SCALE = {
      .x = 25,
      .y = 10,
  };
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    for (size_t i = 0; i < diagram.count; ++i) {
      // FIXME: Corners where lines meet aren't filled in properly.
      Line *line = &diagram.items[i];
      if (line->kind == LAMBDA_ABSTRACTION) {
        Vector2 start = {.x = line->start.x - 0.4, .y = line->start.y};
        Vector2 end = {.x = line->end.x + 0.4, .y = line->end.y};
        DrawLineEx(Vector2AddValue(Vector2Multiply(start, SCALE), 50),
                   Vector2AddValue(Vector2Multiply(end, SCALE), 50), WIDTH, WHITE);
      } else {
        Vector2 start = {.x = line->start.x, .y = line->start.y};
        Vector2 end = {.x = line->end.x, .y = line->end.y};
        DrawLineEx(Vector2AddValue(Vector2Multiply(start, SCALE), 50),
                   Vector2AddValue(Vector2Multiply(end, SCALE), 50), WIDTH, WHITE);
      }
    }

    EndDrawing();
  }

  CloseWindow();

  tree_free(tree);
  nob_da_free(diagram);
  return 0;
}
