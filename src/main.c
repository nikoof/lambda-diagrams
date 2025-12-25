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


typedef struct {
  Tree_Node *dst;
  Tree_Node *src;
} Node_Pair;

bool tree_copy_subtree_to_node(Tree_Node *dst, Tree_Node *src) {
  if (dst == NULL) return false;
  tree_free(dst->left);
  tree_free(dst->right);
  dst->left = NULL;
  dst->right = NULL;

  if (src == NULL) {
    dst = NULL;
    return true;
  }

  Vec(Node_Pair) stack = {0};
  nob_da_append(&stack, ((Node_Pair){.dst = dst, .src = src}));

  while (stack.count > 0) {
    Node_Pair curr = stack.items[--stack.count];

    // this is a copy
    curr.dst->kind      = curr.src->kind;
    curr.dst->binder    = curr.src->binder;
    curr.dst->user_data = curr.src->user_data;

    curr.dst->name.count = 0;
    nob_da_append_many(&curr.dst->name, curr.src->name.items, curr.src->name.count);


    if (curr.src->left != NULL) {
      if (!tree_add_left_child(curr.dst)) return false;
      nob_da_append(&stack, ((Node_Pair){.dst = curr.dst->left, .src = curr.src->left}));
    }

    if (curr.src->right != NULL) {
      if (!tree_add_right_child(curr.dst)) return false;
      nob_da_append(&stack, ((Node_Pair){.dst = curr.dst->right, .src = curr.src->right}));
    }
  }

  nob_da_free(stack);

  return true;
}

bool beta_reduce(Tree_Node *node) {
  Vec(Tree_Node*) stack = {0};
  nob_da_append(&stack, node);
  while (stack.count > 0
         && !(node->kind == LAMBDA_APPLICATION
         && node->left != NULL
         && node->left->kind == LAMBDA_ABSTRACTION)) {
    node = stack.items[--stack.count];
    if (node->left != NULL) nob_da_append(&stack, node->left);
    if (node->right != NULL) nob_da_append(&stack, node->right);
  }
  stack.count = 0;

  Vec(Tree_Node*) atoms = {0};
  nob_da_append(&stack, node->left);
  while (stack.count > 0) {
    Tree_Node *curr = stack.items[--stack.count];

    if (curr->kind == LAMBDA_ATOM) {
      nob_da_append(&atoms, curr);
    }

    if (curr->left != NULL) nob_da_append(&stack, curr->left);
    if (curr->right != NULL) nob_da_append(&stack, curr->right);
  }
  stack.count = 0;

  nob_da_foreach(Tree_Node*, atom, &atoms) {
    if (*atom == node->left->left) continue;
    printf("COPY "SB_Fmt" to "SB_Fmt"\n", SB_Arg(node->right->name), SB_Arg((*atom)->name));
    if (!tree_copy_subtree_to_node(*atom, node->right)) return false;
  }

  tree_free(node->right);
  node->right = NULL;

  Tree_Node *new_node = node->left->right;
  if (node->left != NULL) free(node->left);
  nob_sb_free(node->name);
  *node = *new_node;

  nob_da_free(stack);
  nob_da_free(atoms);

  return true;
}

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  /* const char *term = "lf.lx.f(f(f(f(f(fx)))))"; */
  // const char *term = "(lx.xxx)(lx.ly.xy)";
  const char *term = "(lf.lg.lx.(ly.y)x)(lz.z)";
  /* const char *term = "ln.lf.n(lf.ln.n(f(lf.lx.nf(fx))))(lx.f)(lx.x)"; */
  /* const char *term = "ln.lf.n(lc.la.lb.cb(lx.a(bx)))(lx.ly.x)(lx.x)f"; */
  // const char *term = "ln.lf.lx.n(lg.lh.h(gf))(lu.x)(lu.u)";
  /* const char *term = "lf.(lx.xx)(lx.f(xx))"; */
  /* const char *term = "lf.(lx.xx)f"; */

  Tree_Node *tree = calloc(1, sizeof(Tree_Node));
  assert(tree != NULL);
  if (!tree_parse_lambda_term(tree, term)) return 1;

  tree_print_graphviz(stdout, tree, true);
  if (!beta_reduce(tree)) return 1;
  tree_print_graphviz(stdout, tree, true);


  // Diagram diagram = {0};
  // diagram_from_lambda_tree(&diagram, tree);

  // SetTraceLogLevel(LOG_ERROR);
  // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  // InitWindow(800, 600, "Lambda Diagrams");
  //
  // RenderTexture2D diagram_tex = diagram_to_raylib_texture(diagram, 300, 200, 6, 5.0);
  //
  // while (!WindowShouldClose()) {
  //   BeginDrawing();
  //   ClearBackground(BLACK);
  //
  //   DrawTexture(diagram_tex.texture, 50, 50, WHITE);
  //
  //   EndDrawing();
  // }
  //
  // CloseWindow();

  tree_free(tree);
  // nob_da_free(diagram);
  return 0;
}
