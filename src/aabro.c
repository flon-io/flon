
//
// Copyright (c) 2013-2014, John Mettraux, jmettraux+flon@gmail.com
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

// https://github.com/flon-io/aabro

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

#include "aabro.h"

#define MAX_DEPTH 2048


fabr_tree *fabr_tree_malloc(
  short result,
  size_t offset,
  size_t length,
  char *note,
  fabr_parser *p,
  fabr_tree *child
)
{
  fabr_tree *t = calloc(1, sizeof(fabr_tree));

  t->name = (p->name == NULL) ? NULL : strdup(p->name);
  t->result = result;
  t->offset = offset;
  t->length = length;
  t->note = (note == NULL) ? NULL : strdup(note);
  t->parser = p;
  t->sibling = NULL;
  t->child = child;

  return t;
}

void fabr_tree_free(fabr_tree *t)
{
  if (t->name != NULL) free(t->name);
  if (t->note != NULL) free(t->note);

  for (fabr_tree *c = t->child; c != NULL; )
  {
    fabr_tree *s = c->sibling;
    fabr_tree_free(c);
    c = s;
  }

  free(t);
}

char *fabr_tree_string(const char *input, fabr_tree *t)
{
  return strndup(input + t->offset, t->length);
}

char *fabr_tree_str(char *input, fabr_tree *t)
{
  return input + t->offset;
}

typedef enum fabr_p_type
{
  fabr_pt_string,
  fabr_pt_rep,
  fabr_pt_alt,
  fabr_pt_altg,
  fabr_pt_seq,
  fabr_pt_not,
  fabr_pt_name,
  fabr_pt_presence,
  fabr_pt_absence,
  fabr_pt_n,
  fabr_pt_r,
  fabr_pt_q,
  fabr_pt_range,
  fabr_pt_rex,
  fabr_pt_error
} fabr_p_type;

char *fabr_p_names[] = { // const ?
  "string",
  "rep", "alt", "altg", "seq",
  "not", "name", "presence", "absence", "n",
  "r", "q", "range", "rex",
  "error"
};

static void fabr_t_to_s(
  fabr_tree *t, const char *input,
  flu_sbuffer *b, size_t indent, short children, short color)
{
  for (size_t i = 0; i < indent; i++) flu_sbprintf(b, "  ");

  if (t == NULL)
  {
    flu_sbprintf(b, "{null}");
    return;
  }

  char *stringc = color ? "[1;33m" : ""; // yellow
  char *clearc = color ? "[0;0m" : "";
  char *namec = color ? "[1;34m" : ""; // light blue
  char *notec = color ? "[1;31m" : ""; // light red

  char *name = "null";
  char *note = "null";
  char *resultc = ""; if (color) resultc = t->result ? "[0;0m" : "[1;30m";
  if (t->name) name = flu_sprintf("\"%s%s%s\"", namec, t->name, resultc);
  if (t->note) note = flu_sprintf("\"%s%s%s\"", notec, t->note, resultc);
  //
  flu_sbprintf(
    b,
    "%s[ %s, %d, %d, %d, %s, \"%s-%s\", ",
    resultc, name, t->result, t->offset, t->length,
    note, fabr_p_names[t->parser->type], t->parser->id);
  //
  if (t->name) free(name);
  if (t->note) free(note);

  if (children != 1 && (input == NULL || t->result != 1 || t->child))
  {
    size_t cc = 0; for (fabr_tree *c = t->child; c; c = c->sibling) ++cc;
    flu_sbprintf(b, "%zu ]%s", cc, clearc);
    return;
  }

  if (t->child == NULL)
  {
    if (input == NULL || t->result != 1)
    {
      flu_sbprintf(b, "[] ]%s", clearc);
    }
    else
    {
      char *s = flu_n_escape(input + t->offset, t->length);
      flu_sbprintf(b, "\"%s%s%s\" ]%s", stringc, s, resultc, clearc);
      free(s);
    }
    return;
  }

  flu_sbprintf(b, "[");

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (c != t->child) flu_sbputc(b, ',');
    flu_sbputc(b, '\n');
    fabr_t_to_s(c, input, b, indent + 1, children, color);
  }

  flu_sbputc(b, '\n');
  for (int i = 0; i < indent; i++) flu_sbprintf(b, "  ");
  flu_sbprintf(b, "%s] ]%s", resultc, clearc);
}

char *fabr_tree_to_string(fabr_tree *t, const char *input, short color)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fabr_t_to_s(t, input, b, 0, 1, color);
  return flu_sbuffer_to_string(b);
}

char *fabr_tree_to_str(fabr_tree *t, const char *input, short color)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fabr_t_to_s(t, input, b, 0, 0, color);
  return flu_sbuffer_to_string(b);
}

//
// the fabr_parser methods

static flu_list *fabr_p_list(flu_list *l, fabr_parser *p)
{
  // using the [not] cheap flu_list_add_unique trick
  // but parsers shan't be that big

  if (l == NULL) l = flu_list_malloc();
  int r = flu_list_add_unique(l, p);
  if (r && p->children) for (size_t i = 0; p->children[i] != NULL; i++)
  {
    fabr_p_list(l, p->children[i]);
  }
  return l;
}

static void fabr_p_free(void *v)
{
  fabr_parser *p = v;

  if (p->id != NULL) free(p->id);
  if (p->name != NULL) free(p->name);
  if (p->string != NULL) free(p->string);
  if (p->children != NULL) free(p->children);

  free(p);
}

void fabr_parser_free(fabr_parser *p)
{
  // list all parsers, then free them

  flu_list *ps = fabr_p_list(NULL, p);
  flu_list_and_items_free(ps, fabr_p_free);
}

static fabr_parser *fabr_parser_malloc(fabr_p_type type, const char *name)
{
  fabr_parser *p = calloc(1, sizeof(fabr_parser));

  p->id = NULL;
  p->name = (name == NULL) ? NULL : strdup(name);
  p->type = type;
  p->string = NULL;
  p->min = -1; p->max = -1;
  p->children = NULL;

  return p;
}

#define FABR_IDS "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define FABR_IDS_LENGTH 62

static void fabr_set_id(fabr_parser *p, size_t depth, char *id)
{
  if (p->id != NULL) { free(id); return; }

  p->id = id;

  if (p->children == NULL) return;

  for (size_t i = 0; p->children[i] != NULL; ++i)
  {
    char *cid = calloc(depth + 3, sizeof(char));
    strcpy(cid, id);
    cid[depth + 1] = (i >= FABR_IDS_LENGTH) ? '+' : FABR_IDS[i];
    fabr_set_id(p->children[i], depth + 1, cid);
  }
}

static void fabr_set_ids(fabr_parser *p)
{
  fabr_set_id(p, 0, strdup("0"));
}

//
// the builder methods

static fabr_parser **fabr_single_child(fabr_parser *p)
{
  fabr_parser **children = calloc(2, sizeof(fabr_parser *));
  children[0] = p;
  return children;
}

static void fabr_do_name(fabr_parser *named, fabr_parser *target)
{
  if (named->name == NULL) return;

  if (target->type == fabr_pt_n)
  {
    if (strcmp(target->name, named->name) != 0) return;
    if (target->children == NULL) target->children = fabr_single_child(named);
    return;
  }

  if (target->children == NULL) return;

  for (size_t i = 0; target->children[i] != NULL; i++)
  {
    fabr_do_name(named, target->children[i]);
  }
}

static size_t fabr_parse_rex_quant(const char *s, fabr_parser *p);
  // defined below

static fabr_parser *fabr_r_expand(fabr_parser *r, fabr_parser *child)
{
  fabr_parse_rex_quant(r->string, r);

  r->type = fabr_pt_rep;
  free(r->string); r->string = NULL;

  r->children = fabr_single_child(child);

  if (r->name == NULL && child->name != NULL)
  {
    r->name = child->name;
    child->name = NULL;
  }

  return r;
}

static void fabr_q_wrap(fabr_parser *last, fabr_parser *q)
{
  char *quantifier = q->string;
  char *q_name = q->name;

  // last becomes q, q becomes last

  q->type = last->type;
  q->name = last->name;
  q->string = last->string;
  q->min = last->min;
  q->max = last->max;
  q->children = last->children;

  last->type = fabr_pt_rep;
  last->name = q_name;
  last->string = NULL;
  fabr_parse_rex_quant(quantifier, last); free(quantifier);
  last->children = fabr_single_child(q);
}

static fabr_parser *fabr_wrap_children(
  fabr_parser *p, fabr_parser *c0, va_list ap)
{
  if (c0->type == fabr_pt_q)
  {
    c0->type = fabr_pt_error;
    char *ns = flu_sprintf("'%s': no preceding parser to wrap", c0->string);
    free(c0->string);
    c0->string = ns;
  }

  flu_list *l = flu_list_malloc();

  flu_list_add(l, c0);

  fabr_parser *child = NULL;

  while (1)
  {
    child = va_arg(ap, fabr_parser *);

    if (child == NULL) break;
    if (child->type == fabr_pt_r) break;

    if (child->type == fabr_pt_q)
    {
      fabr_q_wrap((fabr_parser *)l->last->item, child); continue;
    }

    flu_list_add(l, child);
  }

  p->children = (fabr_parser **)flu_list_to_array(l, FLU_F_EXTRA_NULL);

  flu_list_free(l);

  if (child == NULL || child->type == fabr_pt_q) return p;

  return fabr_r_expand(child, p);
}

fabr_parser *fabr_string(const char *s)
{
  return fabr_n_string(NULL, s);
}

fabr_parser *fabr_n_string(const char *name, const char *s)
{
  fabr_parser *p = fabr_parser_malloc(fabr_pt_string, name);
  p->string = strdup(s);
  return p;
}

fabr_parser *fabr_range(const char *range)
{
  return fabr_n_range(NULL, range);
}

fabr_parser *fabr_n_range(const char *name, const char *range)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_range, name);
  r->string = strdup(range);
  fabr_do_name(r, r);
  return r;
}

static fabr_parser *fabr_decompose_rex_group(const char *s, ssize_t n);
  // defined below

fabr_parser *fabr_rex(const char *s)
{
  return fabr_n_rex(NULL, s);
}

fabr_parser *fabr_n_rex(const char *name, const char *s)
{
  fabr_parser *p = fabr_parser_malloc(fabr_pt_rex, name);
  p->string = strdup(s);
  p->children = fabr_single_child(fabr_decompose_rex_group(s, -1));
  return p;
}

fabr_parser *fabr_rep(fabr_parser *p, ssize_t min, ssize_t max)
{
  return fabr_n_rep(NULL, p, min, max);
}

fabr_parser *fabr_n_rep(const char *name, fabr_parser *p, ssize_t min, ssize_t max)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_rep, name);
  r->min = min;
  r->max = max;
  r->children = fabr_single_child(p);
  fabr_do_name(r, p);
  return r;
}

fabr_parser *fabr_alt(fabr_parser *p, ...)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_alt, NULL);

  va_list l; va_start(l, p); r = fabr_wrap_children(r, p, l); va_end(l);

  return r;
}

fabr_parser *fabr_altg(fabr_parser *p, ...)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_altg, NULL);

  va_list l; va_start(l, p); r = fabr_wrap_children(r, p, l); va_end(l);

  return r;
}

fabr_parser *fabr_n_alt(const char *name, fabr_parser *p, ...)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_alt, name);

  va_list l; va_start(l, p); r = fabr_wrap_children(r, p, l); va_end(l);
  fabr_do_name(r, r);

  return r;
}

fabr_parser *fabr_n_altg(const char *name, fabr_parser *p, ...)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_altg, name);

  va_list l; va_start(l, p); r = fabr_wrap_children(r, p, l); va_end(l);
  fabr_do_name(r, r);

  return r;
}

fabr_parser *fabr_seq(fabr_parser *p, ...)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_seq, NULL);

  va_list l; va_start(l, p); r = fabr_wrap_children(r, p, l); va_end(l);

  return r;
}

fabr_parser *fabr_n_seq(const char *name, fabr_parser *p, ...)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_seq, name);

  va_list l; va_start(l, p); r = fabr_wrap_children(r, p, l); va_end(l);
  fabr_do_name(r, r);

  return r;
}

fabr_parser *fabr_name(const char *name, fabr_parser *p)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_name, name);
  r->children = fabr_single_child(p);
  fabr_do_name(r, r);

  return r;
}

fabr_parser *fabr_n(const char *name)
{
  return fabr_parser_malloc(fabr_pt_n, name);
}

fabr_parser *fabr_r(const char *code)
{
  return fabr_n_r(NULL, code);
}

fabr_parser *fabr_n_r(const char *name, const char *code)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_r, name);
  r->string = strdup(code);
  fabr_do_name(r, r);

  return r;
}

fabr_parser *fabr_q(const char *code)
{
  return fabr_n_q(NULL, code);
}

fabr_parser *fabr_n_q(const char *name, const char *code)
{
  fabr_parser *r = fabr_parser_malloc(fabr_pt_q, name);
  r->string = strdup(code);
  fabr_do_name(r, r);

  return r;
}

//
// the to_s methods

typedef void fabr_p_to_s_func(flu_sbuffer *, flu_list *, int, fabr_parser *);

static void fabr_p_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p);

static void fabr_p_string_to_s( // works for range and rex as well
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  if (p->name == NULL)
    flu_sbprintf(
      b, "fabr_%s(\"%s\") /* %s */",
      fabr_p_names[p->type], p->string, p->id);
  else
    flu_sbprintf(
      b, "fabr_n_%s(\"%s\", \"%s\") /* %s */",
      fabr_p_names[p->type], p->name, p->string, p->id);
}

static void fabr_p_rep_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  if (p->name == NULL)
  {
    flu_sbprintf(b, "fabr_rep( /* %s */\n", p->id);
  }
  else
  {
    flu_sbprintf(b, "fabr_n_rep( /* %s */\n", p->id);
    for (int i = 0; i < indent + 1; i++) flu_sbprintf(b, "  ");
    flu_sbprintf(b, "\"%s\",\n", p->name);
  }
  fabr_p_to_s(b, seen, indent + 1, p->children[0]);
  flu_sbprintf(b, ", %i, %i)", p->min, p->max);
}

static void fabr_p_wchildren_to_s(
  const char *n, flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  if (p->name == NULL)
  {
    flu_sbprintf(b, "fabr_%s( /* %s */\n", n, p->id);
  }
  else
  {
    flu_sbprintf(b, "fabr_n_%s( /* %s */\n", n, p->id);
    for (int i = 0; i < indent + 1; i++) flu_sbprintf(b, "  ");
    flu_sbprintf(b, "\"%s\",\n", p->name);
  }
  if (p->children != NULL) for (size_t i = 0; ; i++)
  {
    fabr_parser *c = p->children[i];
    fabr_p_to_s(b, seen, indent + 1, c);
    if (c == NULL) break;
    flu_sbprintf(b, ",\n");
  }
  flu_sbprintf(b, ")");
}

static void fabr_p_alt_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  fabr_p_wchildren_to_s(
    p->type == fabr_pt_alt ? "alt" : "altg", b, seen, indent, p);
}

static void fabr_p_seq_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  fabr_p_wchildren_to_s("seq", b, seen, indent, p);
}

static void fabr_p_name_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_name( /* %s */\n", p->id);
  for (int i = 0; i < indent + 1; i++) flu_sbprintf(b, "  ");
  flu_sbprintf(b, "\"%s\",\n", p->name);
  fabr_p_to_s(b, seen, indent + 1, p->children[0]);
  flu_sbprintf(b, ")");
}

static void fabr_p_not_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_not(...)");
}

static void fabr_p_presence_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_presence(...)");
}

static void fabr_p_absence_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_absence(...)");
}

static void fabr_p_n_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_n(\"%s\") /* %s */", p->name, p->id);
  if (p->children == NULL) flu_sbprintf(b, " /* not linked */", p->name);
  //else flu_sbprintf(b, " /* linked */", p->name);
}

static void fabr_p_r_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_r(\"%s\") /* %s */", p->string, p->id);
}

static void fabr_p_q_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  flu_sbprintf(b, "fabr_q(\"%s\") /* %s */", p->string, p->id);
}

fabr_p_to_s_func *fabr_p_to_s_funcs[] = { // const ?
  fabr_p_string_to_s,
  fabr_p_rep_to_s,
  fabr_p_alt_to_s,
  fabr_p_alt_to_s, // altg
  fabr_p_seq_to_s,
  fabr_p_not_to_s,
  fabr_p_name_to_s,
  fabr_p_presence_to_s,
  fabr_p_absence_to_s,
  fabr_p_n_to_s,
  fabr_p_r_to_s,
  fabr_p_q_to_s,
  fabr_p_string_to_s, // range
  fabr_p_string_to_s, // rex
  fabr_p_string_to_s  // "error" parser
};

void fabr_p_to_s(flu_sbuffer *b, flu_list *seen, int indent, fabr_parser *p)
{
  for (int i = 0; i < indent; i++) flu_sbprintf(b, "  ");
  if (p == NULL)
  {
    flu_sbprintf(b, "NULL");
  }
  else
  {
    int r = flu_list_add_unique(seen, p);
    if (r) fabr_p_to_s_funcs[p->type](b, seen, indent, p);
    else flu_sbprintf(b, "fabr_n(\"%s\") /* %s */", p->name, p->id);
  }
}

char *fabr_parser_to_string(fabr_parser *p)
{
  if (p->id == NULL) fabr_set_ids(p);

  flu_sbuffer *b = flu_sbuffer_malloc();
  flu_list *seen = flu_list_malloc();

  fabr_p_to_s(b, seen, 0, p);

  flu_list_free(seen);

  return flu_sbuffer_to_string(b);
}

char *fabr_parser_to_s(fabr_parser *p)
{
  //if (p->id == NULL) fabr_set_ids(p);

  size_t ccount = 0;
  if (p->children) while (p->children[ccount] != NULL) { ++ccount; }

  char *name = "";
  if (p->name) name = flu_sprintf("'%s' ", p->name);

  char *string = "";
  if (p->string) string = flu_sprintf("\"%s\" ", p->string);

  char *minmax = "";
  if (p->type == fabr_pt_rep) minmax = flu_sprintf(" mn%i mx%i", p->min, p->max);

  char *s = flu_sprintf(
    "%s t%i %s%sc%i%s",
    fabr_p_names[p->type], p->type, name, string, ccount, minmax);

  if (*name != '\0') free(name);
  if (*string != '\0') free(string);
  if (*minmax != '\0') free(minmax);

  return s;
}

//
// the parse methods

// TODO: make the parse methods static

typedef fabr_tree *fabr_p_func(
  const char *, size_t, size_t, fabr_parser *, int flags);
//
static fabr_tree *fabr_do_parse(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags);

fabr_tree *fabr_p_string(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  if (p->min == -1) p->min = strlen(p->string);

  int su = 1;
  size_t le = p->min;

  if (strncmp(input + offset, p->string, le) != 0) { su = 0; le = 0; }

  return fabr_tree_malloc(su, offset, le, NULL, p, NULL);
}

fabr_tree *fabr_p_rep(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  short result = 1;
  size_t off = offset;
  size_t length = 0;

  fabr_tree *first = NULL;
  fabr_tree *prev = NULL;

  size_t count = 0;
  //
  for (; ; count++)
  {
    if (p->max > 0 && count >= p->max) break;
    fabr_tree *t = fabr_do_parse(input, off, depth + 1, p->children[0], flags);

    if (first == NULL) first = t;
    if (prev != NULL) prev->sibling = t;
    prev = t;

    if (t->result < 0) result = -1;
    if (t->result != 1) break;
    if (t->length < 1) break;
    off += t->length;
    length += t->length;
  }

  if (result == 1 && count < p->min) result = 0;
  if (result < 0) length = 0;

  return fabr_tree_malloc(result, offset, length, NULL, p, first);
}

fabr_tree *fabr_p_alt(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  short result = 0;

  fabr_tree *first = NULL;
  fabr_tree *prev = NULL;
  fabr_tree *winner = NULL;

  for (size_t i = 0; p->children[i] != NULL; i++)
  {
    fabr_parser *pc = p->children[i];

    fabr_tree *t = fabr_do_parse(input, offset, depth + 1, pc, flags);

    if (first == NULL) first = t;
    if (prev != NULL) prev->sibling = t;
    prev = t;

    if (t->result == 1) result = 1;
    if (t->result < 0) result = t->result;

    if (result < 0) break;
    if (t->result != 1) continue;

    if (p->type == fabr_pt_alt)
    {
      winner = t; break;
    }
    if (winner != NULL && t->length <= winner->length)
    {
      t->result = 0; continue;
    }
    if (winner) winner->result = 0;
    winner = t;
  }

  return fabr_tree_malloc(
    result, offset, winner ? winner->length : 0, NULL, p, first);
}

fabr_tree *fabr_p_seq(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  short result = 1;
  size_t length = 0;
  size_t off = offset;

  fabr_tree *first = NULL;
  fabr_tree *prev = NULL;

  for (size_t i = 0; p->children[i] != NULL; i++)
  {
    fabr_parser *pc = p->children[i];

    fabr_tree *t = fabr_do_parse(input, off, depth + 1, pc, flags);

    if (first == NULL) first = t;
    if (prev != NULL) prev->sibling = t;
    prev = t;

    if (t->result != 1) { result = t->result; length = 0; break; }
    off += t->length;
    length += t->length;
  }

  return fabr_tree_malloc(result, offset, length, NULL, p, first);
}

fabr_tree *fabr_p_name(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  fabr_tree *t = fabr_do_parse(input, offset, depth + 1, p->children[0], flags);

  return fabr_tree_malloc(t->result, t->offset, t->length, NULL, p, t);
}

fabr_tree *fabr_p_n(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  if (p->children == NULL)
  {
    char *note = flu_sprintf("unlinked fabr_n(\"%s\")", p->name);
    fabr_tree *t = fabr_tree_malloc(-1, offset, 0, note, p, NULL);
    free(note);
    return t;
  }
  return fabr_do_parse(input, offset, depth, p->children[0], flags);
}

void fabr_range_next(char *range, char *next)
{
  size_t b_index = 1;
  char a = range[0];
  if (a == '\\') { a = range[1]; b_index = 2; }
  if (a == '\0') { next[0] = 0; next[1] = 0; next[2] = 0; return; }

  char b = range[b_index];
  char c = (b != '\0') ? range[b_index + 1] : 'X'; // don't read too far
  if (b != '-' || c == '\0') { next[0] = 1; next[1] = a; next[2] = a; return; }
  b = range[++b_index];
  if (b == '\\') b = range[++b_index];

  next[0] = 2; next[1] = a; next[2] = b;
}

fabr_tree *fabr_p_range(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  char *range = p->string;
  char c = (input + offset)[0];

  if (strcmp(range, "$") == 0)
  {
    return fabr_tree_malloc(c == '\0', offset, 0, NULL, p, NULL);
  }

  if (c == '\0') return fabr_tree_malloc(0, offset, 0, NULL, p, NULL);

  short success = 0;

  if (strcmp(range, ".") == 0)
  {
    success = (c != '\n');
    return fabr_tree_malloc(success, offset, success ? 1 : 0, NULL, p, NULL);
  }

  short not = (range[0] == '^'); if (not) ++range;

  char *next = calloc(3, sizeof(char));
  while (1)
  {
    fabr_range_next(range, next);
    if (next[0] == 0) break;
    if (c >= next[1] && c <= next[2]) { success = 1; break; }
    range = range + next[0];
  }
  free(next);

  if (not) success = ( ! success);
  return fabr_tree_malloc(success, offset, success ? 1 : 0, NULL, p, NULL);
}

fabr_tree *fabr_p_rex(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  fabr_tree *t = fabr_do_parse(input, offset, depth + 1, p->children[0], flags);

  fabr_tree *r = fabr_tree_malloc(t->result, offset, t->length, NULL, p, NULL);

  if ((flags & FABR_F_PRUNE) && t->result == 1)
    fabr_tree_free(t);
  else
    r->child = t;

  return r;
}

fabr_tree *fabr_p_error(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  return fabr_tree_malloc(-1, offset, 0, strdup(p->string), p, NULL);
}

fabr_tree *fabr_p_not_implemented(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  char *s0 = fabr_parser_to_string(p);
  char *s1 = flu_sprintf("not implemented %s", s0);
  fabr_tree *t = fabr_tree_malloc(-1, offset, 0, s1, p, NULL);
  free(s0);
  free(s1);
  return t;
}

fabr_p_func *fabr_p_funcs[] = { // const ?
  fabr_p_string,
  fabr_p_rep,
  fabr_p_alt,
  fabr_p_alt, //altg
  fabr_p_seq,
  fabr_p_not_implemented, //fabr_p_not,
  fabr_p_name,
  fabr_p_not_implemented, //fabr_p_presence,
  fabr_p_not_implemented, //fabr_p_absence,
  fabr_p_n,
  fabr_p_not_implemented, //fabr_p_r
  fabr_p_not_implemented, //fabr_p_q
  fabr_p_range,
  fabr_p_rex,
  fabr_p_error
};

static fabr_tree *fabr_do_parse(
  const char *input,
  size_t offset, size_t depth,
  fabr_parser *p,
  int flags)
{
  //printf("input >%s<\n", input + offset);

  if (depth > MAX_DEPTH)
  {
    return fabr_tree_malloc(
      -1, offset, 0, "too much recursion, parser loop?", p, NULL);
  }

  fabr_tree *t = fabr_p_funcs[p->type](input, offset, depth, p, flags);

  int match = flags & FABR_F_MATCH;

  if (
    match == 0 && ((flags & FABR_F_PRUNE) == 0 || t->child == NULL)
  ) return t;

  fabr_tree *first = t->child;
  t->child = NULL;
  fabr_tree **sibling = &t->child;
  fabr_tree *next = NULL;
  for (fabr_tree *c = first; c != NULL; c = next)
  {
    next = c->sibling; c->sibling = NULL;

    if (match || t->result == 0 || c->result == 0)
    {
      fabr_tree_free(c);
    }
    else
    {
      *sibling = c; sibling = &c->sibling;
    }
  }

  return t;
}


//
// entry point

fabr_tree *fabr_parse(const char *input, size_t offset, fabr_parser *p)
{
  return fabr_parse_f(input, offset, p, FABR_F_PRUNE);
}

fabr_tree *fabr_parse_all(const char *input, size_t offset, fabr_parser *p)
{
  return fabr_parse_f(input, offset, p, FABR_F_PRUNE | FABR_F_ALL);
}

fabr_tree *fabr_parse_f(
  const char *input, size_t offset, fabr_parser *p, int flags)
{
  if (p->id == NULL) fabr_set_ids(p);

  fabr_tree *t = fabr_do_parse(input, offset, 0, p, flags);

  if ((flags & FABR_F_ALL) == 0) return t;

  // check if all the input got parsed

  if (t->result == 1 && t->length < strlen(input))
  {
    t->result = 0;
    t->note = strdup(""
      "not all the input could be parsed");
  }
  else if (t->result == 1 && t->length > strlen(input))
  {
    t->result = -1;
    t->note = strdup(""
      "something wrong happened, something longer than the input got parsed");
  }

  return t;
}

int fabr_match(const char *input, fabr_parser *p)
{
  fabr_tree *t = fabr_parse_f(
    input, 0, p, FABR_F_ALL | FABR_F_PRUNE | FABR_F_MATCH);
  //fabr_tree *t = fabr_parse_f(
  //  input, 0, p, FABR_F_ALL | FABR_F_PRUNE);
  //puts(fabr_tree_to_string(t, input));

  int r = t->result;

  fabr_tree_free(t);

  return r;
}


//
// helper functions

char *fabr_error_message(fabr_tree *t)
{
  if (t->result == -1 && t->note != NULL) return t->note;

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    char *s = fabr_error_message(c);
    if (s != NULL) return s;
  }

  return NULL;
}

fabr_tree *fabr_tree_lookup(fabr_tree *t, const char *name)
{
  return fabr_subtree_lookup(&((fabr_tree){ .child = t }), name);
}

fabr_tree *fabr_subtree_lookup(fabr_tree *t, const char *name)
{
  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (name == NULL && c->name != NULL) return c;
    if (name && c->name && strcmp(c->name, name) == 0) return c;

    fabr_tree *r = fabr_subtree_lookup(c, name);
    if (r) return r;
  }

  return NULL;
}

static void fabr_t_list(flu_list *l, fabr_tree *t, fabr_tree_func *f)
{
  short r = f(t);

  if (r < 0) { return; }
  if (r > 0) { flu_list_add(l, t); return; }

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    fabr_t_list(l, c, f);
  }
}

flu_list *fabr_tree_list(fabr_tree *t, fabr_tree_func *f)
{
  flu_list *l = flu_list_malloc();

  fabr_t_list(l, t, f);

  return l;
}

static void fabr_t_list_named(flu_list *l, fabr_tree *t, const char *name)
{
  if (t->result != 1) { return; }
  if (t->name && strcmp(t->name, name) == 0) { flu_list_add(l, t); return; }

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    fabr_t_list_named(l, c, name);
  }
}

flu_list *fabr_tree_list_named(fabr_tree *t, const char *name)
{
  flu_list *l = flu_list_malloc();

  fabr_t_list_named(l, t, name);

  return l;
}

fabr_tree **fabr_tree_collect(fabr_tree *t, fabr_tree_func *f)
{
  flu_list *l = fabr_tree_list(t, f);

  fabr_tree **ts = (fabr_tree **)flu_list_to_array(l, FLU_F_EXTRA_NULL);
  flu_list_free(l);

  return ts;
}

fabr_parser *fabr_p_child(fabr_parser *p, size_t index)
{
  // expensive, but safer...
  // but well, what's the point the fabr_parser struct is public...

  if (p->children) for (size_t i = 0; p->children[i] != NULL; i++)
  {
    if (index == 0) return p->children[i];
    --index;
  }

  return NULL;
}

fabr_tree *fabr_t_child(fabr_tree *t, size_t index)
{
  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (index == 0) return c;
    --index;
  }

  return NULL;
}

//
// fabr_rex

static ssize_t fabr_find_range_end(const char *s)
{
  for (size_t i = 0; ; ++i)
  {
    char c = s[i];

    if (c == '\0') break;
    if (c == '\\') { ++i; continue; }
    if (c == ']') return i;
  }

  return -1;
}

static ssize_t fabr_find_group_end(const char *s)
{
  //printf("afge s >%s<\n", s);

  for (size_t i = 0, stack = 0, range = 0; ; ++i)
  {
    char c = s[i];

    //printf("afge 0 c:%zu '%c'\n", i, c);

    if (c == '\0') break;
    if (c == '\\') { ++i; continue; }

    //printf("afge 1 c:%zu '%c'\n", i, c);

    if (c == '[') { range = 1; continue; };
    if (c == ']') { range = 0; continue; };
    if (range) continue;

    if (stack == 0 && c == ')') return i;
    if (c == ')') --stack;
    if (c == '(') ++stack;
  }

  return -1;
}

static size_t fabr_parse_rex_quant(const char *s, fabr_parser *p)
{
  char c = s[0];

  if (c == '?') { p->min = 0; p->max = 1; return 1; }
  if (c == '*') { p->min = 0; p->max = -1; return 1; }
  if (c == '+') { p->min = 1; p->max = -1; return 1; }

  if (c != '{') { p->min = -1; p->max = -1; return 0; }

  char *s0 = strdup(s + 1);
  char *s1 = NULL;

  ssize_t j = flu_index(s0, 0, '}');
  s0[j] = '\0';
  char *comma = strchr(s0, ',');

  if (comma != NULL) {
    s1 = strdup(comma + 1);
    s0[comma - s0] = '\0';
  }
  p->min = atoi(s0);
  p->max = s1 ? atoi(s1) : p->min;

  if (s1) free(s1);
  free(s0);

  return j + 2;
}

static fabr_parser *fabr_error(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  flu_sbuffer *b = flu_sbuffer_malloc();
  flu_sbvprintf(b, format, ap);
  va_end(ap);

  fabr_parser *p = fabr_parser_malloc(fabr_pt_error, NULL);
  p->string = flu_sbuffer_to_string(b);

  return p;
}

static fabr_parser *fabr_regroup_rex(fabr_p_type t, flu_list *children)
{
  fabr_parser *p = fabr_parser_malloc(t, NULL);

  flu_list *l = flu_list_malloc();

  for (flu_node *n = children->first; n != NULL; n = n->next)
  {
    fabr_parser *pp = (fabr_parser *)n->item;

    if (pp->type != t) { flu_list_unshift(l, pp); continue; }

    size_t s = 0; while (pp->children[s] != NULL) ++s;
    for (size_t i = s; i > 0; --i) flu_list_unshift(l, pp->children[i - 1]);
    free(pp->children);
    pp->children = NULL;
    fabr_parser_free(pp);
  }

  p->children = (fabr_parser **)flu_list_to_array(l, FLU_F_EXTRA_NULL);

  flu_list_free(l);

  return p;
}

static fabr_parser *fabr_decompose_rex_sequence(const char *s, ssize_t n)
{
  //printf("adrs(\"%s\", %li) \"%s\"\n", s, n, strndup(s, n));

  size_t sl = strlen(s);

  flu_list *children = flu_list_malloc();

  fabr_parser *p = NULL;
  char *ss = NULL;
  size_t ssi = 0;

  for (size_t si = 0; ; ++si)
  {
    char c = (si == n) ? '\0' : s[si];

    if (c == '\0') break;
    //if (c == '\\') { continue; }

    if (c == '?' || c == '*' || c == '+' || c == '{')
    {
      if (p == NULL && children->size < 1)
      {
        p = fabr_error("orphan quantifier >%s<", s + si);
        flu_list_unshift(children, p);
        break;
      }

      fabr_parser *r = fabr_parser_malloc(fabr_pt_rep, NULL);
      si = si - 1 + fabr_parse_rex_quant(s + si, r);

      if (p == NULL || p->type != fabr_pt_string || strlen(p->string) == 1)
      {
        r->children = fabr_single_child((fabr_parser *)children->first->item);
        flu_list_shift(children);
      }
      else // have to grab the last char in the current string...
      {
        size_t ci = strlen(p->string) - 1;
        fabr_parser *p0 = fabr_parser_malloc(fabr_pt_string, NULL);
        p0->string = calloc(2, sizeof(char));
        p0->string[0] = p->string[ci];
        p->string[ci] = '\0';
        r->children = fabr_single_child(p0);
      }
      flu_list_unshift(children, r);
      p = NULL;
      continue;
    }

    if (c == '[')
    {
      ssize_t ei = fabr_find_range_end(s + si + 1);
      if (ei == -1)
      {
        p = fabr_error("range not closed >%s<", s + si);
        flu_list_unshift(children, p);
        break;
      }
      fabr_parser *r = fabr_parser_malloc(fabr_pt_range, NULL);
      r->string = strndup(s + si + 1, ei);
      flu_list_unshift(children, r);
      p = NULL;
      si = si + ei + 1;
      //printf("post range >%s<\n", s + si + 1);
      continue;
    }

    if (c == '(')
    {
      ssize_t ei = fabr_find_group_end(s + si + 1);
      //printf("group end for >%s< is at %i\n", s + si + 1, ei);
      if (ei == -1)
      {
        p = fabr_error("group not closed >%s<", s + si);
        flu_list_unshift(children, p);
        break;
      }
      fabr_parser *g = fabr_decompose_rex_group(s + si + 1, ei);
      flu_list_unshift(children, g);
      p = NULL;
      si = si + ei + 1;
      //printf("post group >%s<\n", s + si + 1);
      continue;
    }

    if (c == '.' || c == '$')
    {
      fabr_parser *r = fabr_parser_malloc(fabr_pt_range, NULL);
      r->string = calloc(2, sizeof(char));
      r->string[0] = c;
      flu_list_unshift(children, r);
      p = NULL;
      continue;
    }

    if (p == NULL || p->type != fabr_pt_string) {
      p = fabr_parser_malloc(fabr_pt_string, NULL);
      p->string = calloc(sl - si + 1, sizeof(char));
      flu_list_unshift(children, p);
      ss = p->string;
      ssi = 0;
    }

    if (c == '\\') { ss[ssi++] = s[++si]; continue; }

    ss[ssi++] = c;
  }

  if (children->size > 1)
    p = fabr_regroup_rex(fabr_pt_seq, children);
  else /*if (children->size == 1)*/
    p = (fabr_parser *)children->first->item;
  //else
    //p = NULL;

  flu_list_free(children);

  return p;
}

static fabr_parser *fabr_decompose_rex_group(const char *s, ssize_t n)
{
  //printf("adrG(\"%s\", %li) \"%s\"\n", s, n, strndup(s, n));

  flu_list *children = flu_list_malloc();

  for (size_t i = 0, j = 0, stack = 0, range = 0; ; j++)
  {
    char c = (j >= n) ? '\0' : s[j];
    //char c1 = (c == '\0') ? '\0' : s[j + 1];
    //printf("i: %zu, j: %zu c+1: >%c%c<\n", i, j, c, c1);

    //if (c == '\\') printf(" \\ + %c\n", s[j + 1]);
    //if (c == '\\' && (c1 == '(' || c1 == ')')) { j++; continue; }
    if (c == '\\') { j++; continue; }

    if (range && c != ']' && c != '\0') continue;
    if (range && c == ']') { range = 0; continue; }
    //
    if (c == '[') { range = 1; continue; }

    if (c == '(') { ++stack; continue; }
    if (c == ')') { --stack; continue; }

    if (c == '|' && stack > 0) continue;

    if (c == '\0' || c == '|')
    {
      fabr_parser *p = fabr_decompose_rex_sequence(s + i, j - i);
      flu_list_unshift(children, p);
      i = j + 1;
      if (c == '\0') break;
    }
  }

  fabr_parser *p = NULL;

  if (children->size > 1)
    p = fabr_regroup_rex(fabr_pt_alt, children);
  else /*if (children->size == 1)*/
    p = (fabr_parser *)children->first->item;
  //else
    //p = NULL;

  flu_list_free(children);

  return p;
}

