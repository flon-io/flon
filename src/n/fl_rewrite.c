
//
// Copyright (c) 2013-2015, John Mettraux, jmettraux+flon@gmail.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// Made in Japan.
//


static void unshift_attribute(char *name, fdja_value *tree)
{
  fdja_value *atts = fdja_l(tree, "1");
  fdja_value *att0 = atts->child;
  fdja_value *natt = fdja_s(name);
  atts->child = natt; natt->sibling = att0;
  natt->key = strdup("_0");

  for (fdja_value *e = natt->sibling; e; e = e->sibling)
  {
    ssize_t i = att_index(e->key);
    if (i < 0) continue;
    free(e->key);
    e->key = flu_sprintf("_%d", i + 1);
  }
}

static void rewrite_as_call_or_invoke(
  fdja_value *vname, fdja_value *tree, fdja_value *node, fdja_value *msg)
{
  char *name = fdja_to_string(vname);

  flon_instruction *inst = lookup_instruction('e', name);
  if (inst) { free(name); return; }

  fdja_value *v = lookup_var(node, name);

  if (is_callable(v))
  {
    //printf("***\n"); fdja_putdc(tree);
    fdja_psetv(node, "inst", "call");
    fdja_replace(fdja_l(tree, "0"), fdja_s("call"));
    unshift_attribute(name, tree);
    //fdja_putdc(tree);
  }

  free(name);
}

static void rewrite_tree(fdja_value *tree, fdja_value *node, fdja_value *msg);
  // forward ...

static fdja_value *to_tree(flu_list *l, fdja_value *node, fdja_value *msg)
{
  fdja_value *r = fdja_array_malloc();

  if (l->size == 1 && ! fdja_is_stringy(l->first->item))
  {
    fdja_push(r, fdja_s("val"));
    fdja_value *atts = fdja_push(r, fdja_object_malloc());
    fdja_set(atts, "_0", fdja_clone(l->first->item));
  }
  else
  {
    fdja_push(r, fdja_clone(l->first->item));
    fdja_value *atts = fdja_push(r, fdja_object_malloc());
    for (flu_node *n = l->first->next; n; n = n->next)
    {
      fdja_value *v = n->item;
      fdja_set(atts, v->key, fdja_clone(v));
    }
  }

  fdja_push(r, fdja_array_malloc());

  rewrite_tree(r, node, msg);

  return r;
}

static int is_op(fdja_value *v, const char *op)
{
  if (v->type == 'q') return 0;
  return fdja_strcmp(v, op) == 0;
}

static void rewrite(
  fdja_value *tree, const char *op, fdja_value *node, fdja_value *msg)
{
  if (fdja_strcmp(fdja_l(tree, "0"), op) == 0) return;

  fdja_value *atts = fdja_l(tree, "1");

  int seen = 0;
  for (fdja_value *a = atts->child; a; a = a->sibling)
  {
    if (is_op(a, op)) { seen = 1; break; }
  }
  if ( ! seen) return;

  fdja_value *t = fdja_array_malloc();
  fdja_push(t, fdja_s(op));
  fdja_push(t, fdja_object_malloc());
  fdja_value *children = fdja_push(t, fdja_array_malloc());

  flu_list *l = flu_list_malloc();
  flu_list_add(l, fdja_l(tree, "0"));

  fdja_value *a = atts->child;
  for (; ; a = a->sibling)
  {
    if (l == NULL) l = flu_list_malloc();

    if (a && ! is_op(a, op)) { flu_list_add(l, a); continue; }

    fdja_push(children, to_tree(l, node, msg)); flu_list_free(l); l = NULL;

    if (a == NULL) break;
  }

  fdja_replace(tree, t);
}

static void rewrite_tree(fdja_value *tree, fdja_value *node, fdja_value *msg)
{
  fdja_value *vname = fdja_l(tree, "0"); expand(vname, node, msg);
  fdja_value *vatt0 = fdja_l(tree, "1._0"); expand(vatt0, node, msg);

  //fdja_putdc(node);
  //fdja_putdc(msg);
  //fdja_putdc(vname);

  if (fdja_is_stringy(vname))
    rewrite_as_call_or_invoke(vname, tree, node, msg);

  //if (is_comparator(vatt0))
  //  rewrite_comparison(vatt0, tree, node, msg);

  // in precedence order
  //
  rewrite(tree, "or", node, msg);
  rewrite(tree, "and", node, msg);
  rewrite(tree, "==", node, msg); rewrite(tree, "!=", node, msg);
  rewrite(tree, ">", node, msg); rewrite(tree, ">=", node, msg);
  rewrite(tree, "<", node, msg); rewrite(tree, "<=", node, msg);
  rewrite(tree, "+", node, msg); rewrite(tree, "-", node, msg);
  rewrite(tree, "*", node, msg); rewrite(tree, "/", node, msg);
    rewrite(tree, "%", node, msg);

  //rewrite(tree, "!", node, msg); // TODO: it's an instruction
}

void flon_rewrite_tree(fdja_value *node, fdja_value *msg)
{
  //fdja_putdc(fdja_l(msg, "tree"));

  rewrite_tree(fdja_l(msg, "tree"), node, msg);
}

