
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

char *suf_to_s(size_t c)
{
  char s[16]; s[15] = 0;

  size_t i = 0; for (; i < 14; ++i)
  {
    s[14 - i] = 103 + (c % 20);
    c = c / 20;
    if (c == 0) break;
  }

  return strdup(s + 14 - i);
}

static int is_op(fdja_value *v, const char *op)
{
  if (v->type == 'q') return 0;
  return fdja_strcmp(v, op) == 0;
}

static int is_tree(fdja_value *v)
{
  if (v == NULL) return 0;
  if (v->type != 'a') return 0;

  if (v->child == NULL) return 0;

  if (v->child->sibling == NULL) return 0;
  if (v->child->sibling->type != 'o') return 0;

  if (v->child->sibling->sibling == NULL) return 0;
  if (v->child->sibling->sibling->type != 'a') return 0;

  if (v->child->sibling->sibling->sibling != NULL) return 0;

  if (strchr("sqya", v->child->type) == NULL) return 0;
  if (v->child->type == 'a' && ! is_tree(v->child)) return 0;

  for (fdja_value *c = v->child->sibling->sibling->child; c; c = c->sibling)
  {
    if ( ! is_tree(c)) return 0;
  }

  return 1;
}

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
  fdja_value *vname, fdja_value *tree, ssize_t *nidsuf,
  fdja_value *node, fdja_value *msg)
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

static void rewrite_tree(
  fdja_value *tree, ssize_t *nidsuf, fdja_value *node, fdja_value *msg);
    // declaration...

static fdja_value *to_tree(
  flu_list *l, ssize_t *nidsuf, fdja_value *node, fdja_value *msg)
{
  fdja_value *r = NULL;

  if (l->size == 1 && is_tree(l->first->item))
  {
    r = fdja_clone(l->first->item);
  }
  else if (l->size == 1 && ! fdja_is_stringy(l->first->item))
  {
    r = fdja_array_malloc();
    fdja_push(r, fdja_s("val"));
    fdja_value *atts = fdja_push(r, fdja_object_malloc());
    fdja_set(atts, "_0", fdja_clone(l->first->item));
  }
  else
  {
    r = fdja_array_malloc();
    fdja_push(r, fdja_clone(l->first->item));
    fdja_value *atts = fdja_push(r, fdja_object_malloc());
    for (flu_node *n = l->first->next; n; n = n->next)
    {
      fdja_value *v = n->item;
      fdja_set(atts, v->key, fdja_clone(v));
    }
  }

  fdja_push(r, fdja_array_malloc());

  rewrite_tree(r, nidsuf, node, msg);

  return r;
}

static void rewrite(
  fdja_value *tree, const char *op, ssize_t *nidsuf,
  fdja_value *node, fdja_value *msg)
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
  fdja_value *natts = fdja_push(t, fdja_object_malloc());
  fdja_value *nchildren = fdja_push(t, fdja_array_malloc());

  if (nidsuf > -1) fdja_set(natts, "_", fdja_sym(suf_to_s(++*nidsuf)));

  flu_list *l = flu_list_malloc();
  flu_list_add(l, fdja_l(tree, "0"));

  fdja_value *a = atts->child;
  for (; ; a = a->sibling)
  {
    if (l == NULL) l = flu_list_malloc();

    if (a && ! is_op(a, op)) { flu_list_add(l, a); continue; }

    fdja_value *c = fdja_push(nchildren, to_tree(l, nidsuf, node, msg));
    fdja_set(fdja_l(c, "1"), "_", fdja_sym(suf_to_s(++*nidsuf)));

    flu_list_free(l); l = NULL;

    if (a == NULL) break;
  }

  fdja_replace(tree, t);
}

static void rewrite_head_if(
  fdja_value *name, fdja_value *tree, ssize_t *nidsuf,
  fdja_value *node, fdja_value *msg)
{
  if (fdja_strcmp(name, "if") != 0 && fdja_strcmp(name, "unless") != 0) return;

  fdja_value *atts = fdja_l(tree, "1");
  flu_list *l = flu_list_malloc();
  for (fdja_value *v = atts->child; v; v = v->sibling)
  {
    flu_list_add(l, v);
  }
  atts->child = NULL;

  fdja_value *c =
    fdja_unshift(fdja_l(tree, "2"), to_tree(l, nidsuf, node, msg));
  fdja_pset(c, "1._", fdja_sym(suf_to_s(++*nidsuf)));

  flu_list_and_items_free(l, (void (*)(void *))fdja_value_free);
}

static void rewrite_post_if(
  fdja_value *vname, fdja_value *tree, ssize_t *nidsuf,
  fdja_value *node, fdja_value *msg)
{
  // TODO
}

static void rewrite_tree(
  fdja_value *tree, ssize_t *nidsuf, fdja_value *node, fdja_value *msg)
{
  fdja_value *vname = fdja_l(tree, "0"); expand(vname, node, msg);
  fdja_value *vatt0 = fdja_l(tree, "1._0"); expand(vatt0, node, msg);

  //fdja_putdc(node);
  //fdja_putdc(msg);
  //fdja_putdc(vname);

  if (fdja_is_stringy(vname))
    rewrite_as_call_or_invoke(vname, tree, nidsuf, node, msg);

  rewrite_head_if(vname, tree, nidsuf, node, msg);
  rewrite_post_if(vname, tree, nidsuf, node, msg);

  // in precedence order
  //
  rewrite(tree, "or", nidsuf, node, msg);
  rewrite(tree, "and", nidsuf, node, msg);
  rewrite(tree, "==", nidsuf, node, msg);
  rewrite(tree, "!=", nidsuf, node, msg);
  rewrite(tree, ">", nidsuf, node, msg);
  rewrite(tree, ">=", nidsuf, node, msg);
  rewrite(tree, "<", nidsuf, node, msg);
  rewrite(tree, "<=", nidsuf, node, msg);
  rewrite(tree, "+", nidsuf, node, msg);
  rewrite(tree, "-", nidsuf, node, msg);
  rewrite(tree, "*", nidsuf, node, msg);
  rewrite(tree, "/", nidsuf, node, msg);
  rewrite(tree, "%", nidsuf, node, msg);

  //rewrite(tree, "!", node, msg); // TODO: it's an instruction
}

void flon_rewrite_tree(fdja_value *node, fdja_value *msg)
{
  //fdja_putdc(fdja_l(msg, "tree"));

  fdja_value *tree = fdja_l(msg, "tree");

  if (fdja_l(tree, "1._")) return;

  ssize_t counter = -1;

  rewrite_tree(tree, &counter, node, msg);
}

