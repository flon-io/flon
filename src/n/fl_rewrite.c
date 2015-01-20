
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

static int rewrite_as_call_or_invoke(
  fdja_value *tree, fdja_value *node, fdja_value *msg)
{
  fdja_value *vname = fdja_l(tree, "0");

  if ( ! fdja_is_stringy(vname)) return 0;

  char *name = fdja_to_string(vname);

  flon_instruction *inst = lookup_instruction('e', name);
  if (inst) { free(name); return 0; }

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

  return 1;
}

static int rewrite_tree(fdja_value *tree, fdja_value *node, fdja_value *msg);
  // declaration...

static fdja_value *to_tree(flu_list *l, fdja_value *node, fdja_value *msg)
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

    size_t index = 0;
    for (flu_node *n = l->first->next; n; n = n->next)
    {
      fdja_value *v = n->item;
      char *key = v->key; if (is_index(key)) key = flu_sprintf("_%zu", index);
      ++index;
      fdja_set(atts, key, fdja_clone(v));
      if (key != v->key) free(key);
    }
  }

  fdja_push(r, fdja_array_malloc()); // no children

  rewrite_tree(r, node, msg);

  return r;
}

static int rewrite(
  fdja_value *tree, const char *op, fdja_value *node, fdja_value *msg)
{
  if (fdja_strcmp(fdja_l(tree, "0"), op) == 0) return 0;

  fdja_value *atts = fdja_l(tree, "1");

  int seen = 0;
  for (fdja_value *a = atts->child; a; a = a->sibling)
  {
    if (is_op(a, op)) { seen = 1; break; }
  }
  if ( ! seen) return 0;

  fdja_value *t = fdja_array_malloc();
  fdja_push(t, fdja_s(op));
  /*fdja_value *natts = */fdja_push(t, fdja_object_malloc());
  fdja_value *nchildren = fdja_push(t, fdja_array_malloc());

  flu_list *l = flu_list_malloc();
  flu_list_add(l, fdja_l(tree, "0"));

  fdja_value *a = atts->child;
  for (; ; a = a->sibling)
  {
    if (l == NULL) l = flu_list_malloc();

    if (a && ! is_op(a, op)) { flu_list_add(l, a); continue; }

    fdja_push(nchildren, to_tree(l, node, msg));

    flu_list_free(l); l = NULL;

    if (a == NULL) break;
  }

  fdja_replace(tree, t);

  return 1;
}

static int rewrite_head_if(
  fdja_value *tree, fdja_value *node, fdja_value *msg)
{
  fdja_value *vname = fdja_l(tree, "0");

  if (
    fdja_strcmp(vname, "if") != 0 &&
    fdja_strcmp(vname, "unless") != 0
  ) return 0;

  fdja_value *atts = fdja_l(tree, "1");
  flu_list *l = flu_list_malloc();
  for (fdja_value *v = atts->child; v; v = v->sibling)
  {
    flu_list_add(l, v);
  }
  atts->child = NULL;

  fdja_unshift(fdja_l(tree, "2"), to_tree(l, node, msg));

  flu_list_and_items_free(l, (void (*)(void *))fdja_value_free);

  return 1;
}

static int rewrite_post_if(
  fdja_value *tree, fdja_value *node, fdja_value *msg)
{
  // TODO

  return 0;
}

static int rewrite_tree(
  fdja_value *tree, fdja_value *node, fdja_value *msg)
{
  int rw = 0; // rewritten? not yet

  rw |= rewrite_as_call_or_invoke(tree, node, msg);

  rw |= rewrite_head_if(tree, node, msg);
  rw |= rewrite_post_if(tree, node, msg);

  // in precedence order
  //
  rw |= rewrite(tree, "or", node, msg);
  rw |= rewrite(tree, "and", node, msg);
  rw |= rewrite(tree, "==", node, msg);
  rw |= rewrite(tree, "!=", node, msg);
  rw |= rewrite(tree, ">", node, msg);
  rw |= rewrite(tree, ">=", node, msg);
  rw |= rewrite(tree, "<", node, msg);
  rw |= rewrite(tree, "<=", node, msg);
  rw |= rewrite(tree, "+", node, msg);
  rw |= rewrite(tree, "-", node, msg);
  rw |= rewrite(tree, "*", node, msg);
  rw |= rewrite(tree, "/", node, msg);
  rw |= rewrite(tree, "%", node, msg);

  //rw |= rewrite(tree, "!", node, msg); // TODO: it's an instruction

  return rw;
}

int flon_rewrite_tree(fdja_value *node, fdja_value *msg)
{
  // TODO: expand head and attributes?

  fdja_value *tree = fdja_l(msg, "tree");
  //fdja_putdc(tree);

  expand(fdja_l(tree, "0"), node, msg); // name / head
  //expand(fdja_l(tree, "1"), node, msg); // attributes

  fdja_set(node, "inst", fdja_lc(tree, "0"));

  //if (fdja_l(tree, "1._")) return 0;
    // TODO: replace with my line vs my child line check...

  int rewritten = rewrite_tree(tree, node, msg);

  if (rewritten)
  {
    fdja_set(node, "tree", fdja_clone(tree));
    fdja_set(node, "inst", fdja_lc(tree, "0"));
  }

  return rewritten;
}

