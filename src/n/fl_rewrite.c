
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

  if (v->child == NULL) return 0; // head
  if (strchr("sqya", v->child->type) == NULL) return 0;
  if (v->child->type == 'a' && ! is_tree(v->child)) return 0;

  if (v->child->sibling == NULL) return 0; // attributes
  if (v->child->sibling->type != 'o') return 0;

  if (v->child->sibling->sibling == NULL) return 0; // line
  if (v->child->sibling->sibling->type != 'n') return 0;

  if (v->child->sibling->sibling->sibling == NULL) return 0; // children
  if (v->child->sibling->sibling->sibling->type != 'a') return 0;

  for (
    fdja_value *c = v->child->sibling->sibling->sibling->child;
    c;
    c = c->sibling
  ) if ( ! is_tree(c)) return 0;

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
  fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  fdja_value *vname = fdja_l(tree, "0");

  if ( ! fdja_is_stringy(vname)) return 0;

  char *name = fdja_to_string(vname);

  flon_instruction *inst = lookup_instruction('e', name);
  if (inst) { free(name); return 0; }

  fdja_value *v = lookup_var(node, 'l', name); // 'l' for "local"

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

static int rewrite_tree(fdja_value *node, fdja_value *msg, fdja_value *tree);
  // declaration...

static fdja_value *to_tree(
  flu_list *l, fdja_value *lnumber, fdja_value *node, fdja_value *msg)
{
  fdja_value *r = NULL;

  //printf("to_tree(): l:%zu\n", l->size);
  //size_t i = 0; for (flu_node *n = l->first; n; n = n->next)
  //{
  //  printf("%zu: ", i++); fdja_putdc(n->item);
  //}

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

  fdja_push(r, fdja_clone(lnumber)); // line number
  fdja_push(r, fdja_array_malloc()); // no children

  rewrite_tree(node, msg, r);

  return r;
}

static int rewrite_infix(
  const char *op, fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  //if (fdja_strcmp(fdja_value_at(tree, 0), op) == 0) return 0;

  fdja_value *atts = fdja_value_at(tree, 1);
  fdja_value *line = fdja_value_at(tree, 2);

  int seen = 0;
  for (fdja_value *a = atts->child; a; a = a->sibling)
  {
    if (is_op(a, op)) { seen = 1; break; }
  }
  if ( ! seen) return 0;

  fdja_value *t = fdja_array_malloc();
  fdja_push(t, fdja_s(op));
  /*fdja_value *natts = */fdja_push(t, fdja_object_malloc());
  fdja_push(t, fdja_clone(line));
  fdja_value *nchildren = fdja_push(t, fdja_array_malloc());

  flu_list *l = flu_list_malloc();
  flu_list_add(l, fdja_value_at(tree, 0));

  fdja_value *a = atts->child;
  for (; ; a = a->sibling)
  {
    if (l == NULL) l = flu_list_malloc();

    if (a && ! is_op(a, op)) { flu_list_add(l, a); continue; }

    fdja_push(nchildren, to_tree(l, line, node, msg));

    flu_list_free(l); l = NULL;

    if (a == NULL) break;
  }

  fdja_replace(tree, t);

  return 1;
}

static int rewrite_prefix(
  const char *op, fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  if (fdja_strcmp(fdja_value_at(tree, 0), op) != 0) return 0;

  fdja_value *children = fdja_value_at(tree, 3);
  if (children->child != NULL) return 0;

  fdja_value *atts = fdja_value_at(tree, 1);
  fdja_value *line = fdja_value_at(tree, 2);

  fdja_value *t = fdja_array_malloc();
  fdja_push(t, fdja_s(op));
  /*fdja_value *natts = */fdja_push(t, fdja_object_malloc());
  fdja_push(t, fdja_clone(line));
  fdja_value *nchildren = fdja_push(t, fdja_array_malloc());

  for (fdja_value *a = atts->child; a; a = a->sibling)
  {
    flu_list *l = flu_list_malloc(); flu_list_add(l, a);
    fdja_push(nchildren, to_tree(l, line, node, msg));
    flu_list_free(l);
  }

  fdja_replace(tree, t);

  return 1;
}

static int rewrite_pinfix(
  const char *op, fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  return
    rewrite_infix(op, node, msg, tree) ||
    rewrite_prefix(op, node, msg, tree);
}

static int rewrite_head_if(
  fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  fdja_value *vname = fdja_value_at(tree, 0);

  if (
    fdja_strcmp(vname, "if") != 0 &&
    fdja_strcmp(vname, "unless") != 0
  ) return 0;

  fdja_value *atts = fdja_value_at(tree, 1);
  fdja_value *lnumber = fdja_value_at(tree, 2);

  flu_list *cond = flu_list_malloc();
  flu_list *then = flu_list_malloc();
  flu_list *elze = flu_list_malloc();
  flu_list *l = cond;

  for (fdja_value *v = atts->child; v; v = v->sibling)
  {
    if (fdja_strcmp(v, "then") == 0) l = then;
    else if (fdja_strcmp(v, "else") == 0) l = elze;
    flu_list_add(l, v);
  }
  atts->child = NULL;

  fdja_value *children = fdja_value_at(tree, 3);

  if (cond->size > 0)
  {
    if (then->size > 0)
    {
      if (elze->size > 0)
      {
        fdja_free(flu_list_shift(elze));
        fdja_unshift(children, to_tree(elze, lnumber, node, msg));
      }
      fdja_free(flu_list_shift(then));
      fdja_unshift(children, to_tree(then, lnumber, node, msg));
    }
    fdja_unshift(children, to_tree(cond, lnumber, node, msg));
  }

  flu_list_and_items_free(cond, (void (*)(void *))fdja_value_free);
  flu_list_and_items_free(then, (void (*)(void *))fdja_value_free);
  flu_list_and_items_free(elze, (void (*)(void *))fdja_value_free);

  return 1;
}

static int rewrite_post_if(
  fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  fdja_value *atts = fdja_value_at(tree, 1);

  flu_list *cond = NULL;

  for (fdja_value *a = atts->child; a; a = a->sibling)
  {
    if (cond)
    {
      flu_list_add(cond, a);
    }
    else if (fdja_strcmp(a, "if") == 0 || fdja_strcmp(a, "unless") == 0)
    {
      cond = flu_list_malloc();
      flu_list_add(cond, a);
    }
  }

  if (cond == NULL) return 0;

  fdja_value *lnumber = fdja_value_at(tree, 2);

  fdja_value *condt = to_tree(cond, lnumber, node, msg);

  fdja_value *then_atts = fdja_object_malloc();
  for (fdja_value *a = atts->child; a; a = a->sibling)
  {
    if (a == cond->first->item) break;
    fdja_set(then_atts, a->key, fdja_clone(a));
  }

  flu_list_free(cond);

  fdja_value *thent = fdja_array_malloc();
  fdja_push(thent, fdja_lc(tree, "0")); // head
  fdja_push(thent, then_atts);
  fdja_push(thent, fdja_clone(lnumber));
  fdja_push(thent, fdja_lc(tree, "3")); // children

  fdja_push(fdja_value_at(condt, 3), thent);

  fdja_replace(tree, condt);

  return 1;
}

static int rewrite_tree(fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  //printf("rewrite_tree() "); fdja_putdc(tree);

// TODO: at first, rewrite the "post" stuff !!!

  int rw = 0; // rewritten? not yet

  rw |= rewrite_as_call_or_invoke(node, msg, tree);

  rw |= rewrite_post_if(node, msg, tree);
  rw |= rewrite_head_if(node, msg, tree);

  // in precedence order
  //
  rw |= rewrite_pinfix("or", node, msg, tree);
  rw |= rewrite_pinfix("and", node, msg, tree);
  rw |= rewrite_pinfix("==", node, msg, tree);
  rw |= rewrite_pinfix("!=", node, msg, tree);
  rw |= rewrite_pinfix(">", node, msg, tree);
  rw |= rewrite_pinfix(">=", node, msg, tree);
  rw |= rewrite_pinfix("<", node, msg, tree);
  rw |= rewrite_pinfix("<=", node, msg, tree);
  rw |= rewrite_pinfix("+", node, msg, tree);
  rw |= rewrite_pinfix("-", node, msg, tree);
  rw |= rewrite_pinfix("*", node, msg, tree);
  rw |= rewrite_pinfix("/", node, msg, tree);
  rw |= rewrite_pinfix("%", node, msg, tree);

  //rw |= rewrite_pinfix("!", node, msg, tree); // TODO: it's an instruction

  return rw;
}

int flon_rewrite_tree(fdja_value *node, fdja_value *msg)
{
  fdja_value *tree = fdja_l(msg, "tree");

  expand(fdja_l(tree, "0"), node, msg); // name / head
  expand(fdja_l(tree, "1"), node, msg); // attributes

  //if (fdja_l(tree, "1._")) return 0;
    // TODO: replace with my line vs my child line check...

  fdja_value *origin = fdja_lc(tree, "4");

  int rewritten = rewrite_tree(node, msg, tree);

  if (rewritten)
  {
    if (origin && fdja_value_at(tree, 4) == NULL)
    {
      fdja_push(tree, fdja_clone(origin));
    }
    fdja_set(node, "tree", fdja_clone(tree));
  }

  fdja_free(origin);

  fdja_set(node, "inst", fdja_lc(tree, "0"));

  return rewritten;
}

