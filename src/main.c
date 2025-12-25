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
  (void)argc;
  (void)argv;
  /* const char *term = "lf.lx.f(f(f(f(f(fx)))))"; */
  /* const char *term = "(lx.xx)(lx.xx)"; */
  /* const char *term = "ln.lf.n(lf.ln.n(f(lf.lx.nf(fx))))(lx.f)(lx.x)"; */
  /* const char *term = "ln.lf.n(lc.la.lb.cb(lx.a(bx)))(lx.ly.x)(lx.x)f"; */
  const char *term = "ln.lf.lx.n(lg.lh.h(gf))(lu.x)(lu.u)";
  /* const char *term = "lf.(lx.xx)(lx.f(xx))"; */
  /* const char *term = "lf.(lx.xx)f"; */

  Tree_Node *tree = calloc(1, sizeof(Tree_Node));
  assert(tree != NULL);
  if (!tree_parse_lambda_term(tree, term)) return 1;

  tree_print_graphviz(stdout, tree, true);

  Diagram diagram = {0};
  diagram_from_lambda_tree(&diagram, tree);

  SetTraceLogLevel(LOG_ERROR);
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "Lambda Diagrams");

  RenderTexture2D diagram_tex = diagram_to_raylib_texture(diagram, 300, 200, 6, 5.0);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BLACK);

    DrawTexture(diagram_tex.texture, 50, 50, WHITE);

    EndDrawing();
  }

  CloseWindow();

  tree_free(tree);
  nob_da_free(diagram);
  return 0;
}
