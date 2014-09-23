
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

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <limits.h>

#include "aabro.h"

#define MAX_DEPTH 2048


abr_tree *abr_tree_malloc(
  short result,
  size_t offset,
  size_t length,
  char *note,
  abr_parser *p,
  abr_tree *child
)
{
  abr_tree *t = calloc(1, sizeof(abr_tree));

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

void abr_tree_free(abr_tree *t)
{
  if (t->name != NULL) free(t->name);
  if (t->note != NULL) free(t->note);

  for (abr_tree *c = t->child; c != NULL; )
  {
    abr_tree *s = c->sibling;
    abr_tree_free(c);
    c = s;
  }

  free(t);
}

char *abr_tree_string(const char *input, abr_tree *t)
{
  return strndup(input + t->offset, t->length);
}

char *abr_tree_str(char *input, abr_tree *t)
{
  return input + t->offset;
}

typedef enum abr_p_type
{
  abr_pt_string,
  abr_pt_rep,
  abr_pt_alt,
  abr_pt_seq,
  abr_pt_not,
  abr_pt_name,
  abr_pt_presence,
  abr_pt_absence,
  abr_pt_n,
  abr_pt_r,
  abr_pt_q,
  abr_pt_range,
  abr_pt_rex,
  abr_pt_error
} abr_p_type;

char *abr_p_names[] = { // const ?
  "string",
  "rep", "alt", "seq",
  "not", "name", "presence", "absence", "n",
  "r", "q", "range", "rex",
  "error"
};

static void abr_t_to_s(
  abr_tree *t, const char *input, flu_sbuffer *b, int indent)
{
  for (int i = 0; i < indent; i++) flu_sbprintf(b, "  ");

  if (t == NULL)
  {
    flu_sbprintf(b, "{null}");
    return;
  }

  char *name = "null";
  char *note = "null";
  if (t->name) name = flu_sprintf("\"%s\"", t->name);
  if (t->note) note = flu_sprintf("\"%s\"", t->note);
  //
  flu_sbprintf(
    b,
    "[ %s, %d, %d, %d, %s, \"%s-%s\", ",
    name, t->result, t->offset, t->length,
    note, abr_p_names[t->parser->type], t->parser->id);
  //
  if (t->name) free(name);
  if (t->note) free(note);

  if (t->child == NULL)
  {
    if (input == NULL || t->result != 1)
    {
      flu_sbprintf(b, "[] ]");
    }
    else
    {
      char *s = flu_n_escape(input + t->offset, t->length);
      flu_sbprintf(b, "\"%s\" ]", s);
      free(s);
    }
    return;
  }

  flu_sbprintf(b, "[");

  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (c != t->child) flu_sbputc(b, ',');
    flu_sbputc(b, '\n');
    abr_t_to_s(c, input, b, indent + 1);
  }

  flu_sbputc(b, '\n');
  for (int i = 0; i < indent; i++) flu_sbprintf(b, "  ");
  flu_sbprintf(b, "] ]");
}

char *abr_tree_to_string(abr_tree *t)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  abr_t_to_s(t, NULL, b, 0);
  return flu_sbuffer_to_string(b);
}

char *abr_tree_to_string_with_leaves(const char *input, abr_tree *t)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  abr_t_to_s(t, input, b, 0);
  return flu_sbuffer_to_string(b);
}

//
// the abr_parser methods

static flu_list *abr_p_list(flu_list *l, abr_parser *p)
{
  // using the [not] cheap flu_list_add_unique trick
  // but parsers shan't be that big

  if (l == NULL) l = flu_list_malloc();
  int r = flu_list_add_unique(l, p);
  if (r && p->children) for (size_t i = 0; p->children[i] != NULL; i++)
  {
    abr_p_list(l, p->children[i]);
  }
  return l;
}

static void abr_p_free(void *v)
{
  abr_parser *p = v;

  if (p->id != NULL) free(p->id);
  if (p->name != NULL) free(p->name);
  if (p->string != NULL) free(p->string);
  if (p->children != NULL) free(p->children);

  free(p);
}

void abr_parser_free(abr_parser *p)
{
  // list all parsers, then free them

  flu_list *ps = abr_p_list(NULL, p);
  flu_list_and_items_free(ps, abr_p_free);
}

static abr_parser *abr_parser_malloc(abr_p_type type, const char *name)
{
  abr_parser *p = calloc(1, sizeof(abr_parser));

  p->id = NULL;
  p->name = (name == NULL) ? NULL : strdup(name);
  p->type = type;
  p->string = NULL;
  p->min = -1; p->max = -1;
  p->children = NULL;

  return p;
}

#define ABR_IDS "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define ABR_IDS_LENGTH 62

static void abr_set_id(abr_parser *p, size_t depth, char *id)
{
  if (p->id != NULL) { free(id); return; }

  p->id = id;

  if (p->children == NULL) return;

  for (size_t i = 0; p->children[i] != NULL; ++i)
  {
    char *cid = calloc(depth + 3, sizeof(char));
    strcpy(cid, id);
    cid[depth + 1] = (i >= ABR_IDS_LENGTH) ? '+' : ABR_IDS[i];
    abr_set_id(p->children[i], depth + 1, cid);
  }
}

static void abr_set_ids(abr_parser *p)
{
  abr_set_id(p, 0, strdup("0"));
}

//
// the builder methods

static abr_parser **abr_single_child(abr_parser *p)
{
  abr_parser **children = calloc(2, sizeof(abr_parser *));
  children[0] = p;
  return children;
}

static void abr_do_name(abr_parser *named, abr_parser *target)
{
  if (named->name == NULL) return;

  if (target->type == abr_pt_n)
  {
    if (strcmp(target->name, named->name) != 0) return;
    if (target->children == NULL) target->children = abr_single_child(named);
    return;
  }

  if (target->children == NULL) return;

  for (size_t i = 0; target->children[i] != NULL; i++)
  {
    abr_do_name(named, target->children[i]);
  }
}

static size_t abr_parse_rex_quant(const char *s, abr_parser *p);
  // defined below

static abr_parser *abr_r_expand(abr_parser *r, abr_parser *child)
{
  abr_parse_rex_quant(r->string, r);

  r->type = abr_pt_rep;
  free(r->string); r->string = NULL;

  r->children = abr_single_child(child);

  if (r->name == NULL && child->name != NULL)
  {
    r->name = child->name;
    child->name = NULL;
  }

  return r;
}

static void abr_q_wrap(abr_parser *last, abr_parser *q)
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

  last->type = abr_pt_rep;
  last->name = q_name;
  last->string = NULL;
  abr_parse_rex_quant(quantifier, last); free(quantifier);
  last->children = abr_single_child(q);
}

static abr_parser *abr_wrap_children(abr_parser *p, abr_parser *c0, va_list ap)
{
  if (c0->type == abr_pt_q)
  {
    c0->type = abr_pt_error;
    char *ns = flu_sprintf("'%s': no preceding parser to wrap", c0->string);
    free(c0->string);
    c0->string = ns;
  }

  flu_list *l = flu_list_malloc();

  flu_list_add(l, c0);

  abr_parser *child = NULL;

  while (1)
  {
    child = va_arg(ap, abr_parser *);

    if (child == NULL) break;
    if (child->type == abr_pt_r) break;

    if (child->type == abr_pt_q)
    {
      abr_q_wrap((abr_parser *)l->last->item, child); continue;
    }

    flu_list_add(l, child);
  }

  p->children = (abr_parser **)flu_list_to_array(l, FLU_F_EXTRA_NULL);

  flu_list_free(l);

  if (child == NULL || child->type == abr_pt_q) return p;

  return abr_r_expand(child, p);
}

abr_parser *abr_string(const char *s)
{
  return abr_n_string(NULL, s);
}

abr_parser *abr_n_string(const char *name, const char *s)
{
  abr_parser *p = abr_parser_malloc(abr_pt_string, name);
  p->string = strdup(s);
  return p;
}

abr_parser *abr_range(const char *range)
{
  return abr_n_range(NULL, range);
}

abr_parser *abr_n_range(const char *name, const char *range)
{
  abr_parser *r = abr_parser_malloc(abr_pt_range, name);
  r->string = strdup(range);
  abr_do_name(r, r);
  return r;
}

static abr_parser *abr_decompose_rex_group(const char *s, ssize_t n);
  // defined below

abr_parser *abr_rex(const char *s)
{
  return abr_n_rex(NULL, s);
}

abr_parser *abr_n_rex(const char *name, const char *s)
{
  abr_parser *p = abr_parser_malloc(abr_pt_rex, name);
  p->string = strdup(s);
  p->children = abr_single_child(abr_decompose_rex_group(s, -1));
  return p;
}

abr_parser *abr_rep(abr_parser *p, ssize_t min, ssize_t max)
{
  return abr_n_rep(NULL, p, min, max);
}

abr_parser *abr_n_rep(const char *name, abr_parser *p, ssize_t min, ssize_t max)
{
  abr_parser *r = abr_parser_malloc(abr_pt_rep, name);
  r->min = min;
  r->max = max;
  r->children = abr_single_child(p);
  abr_do_name(r, p);
  return r;
}

abr_parser *abr_alt(abr_parser *p, ...)
{
  abr_parser *r = abr_parser_malloc(abr_pt_alt, NULL);

  va_list l; va_start(l, p); r = abr_wrap_children(r, p, l); va_end(l);

  return r;
}

abr_parser *abr_n_alt(const char *name, abr_parser *p, ...)
{
  abr_parser *r = abr_parser_malloc(abr_pt_alt, name);

  va_list l; va_start(l, p); r = abr_wrap_children(r, p, l); va_end(l);
  abr_do_name(r, r);

  return r;
}

abr_parser *abr_seq(abr_parser *p, ...)
{
  abr_parser *r = abr_parser_malloc(abr_pt_seq, NULL);

  va_list l; va_start(l, p); r = abr_wrap_children(r, p, l); va_end(l);

  return r;
}

abr_parser *abr_n_seq(const char *name, abr_parser *p, ...)
{
  abr_parser *r = abr_parser_malloc(abr_pt_seq, name);

  va_list l; va_start(l, p); r = abr_wrap_children(r, p, l); va_end(l);
  abr_do_name(r, r);

  return r;
}

abr_parser *abr_name(const char *name, abr_parser *p)
{
  abr_parser *r = abr_parser_malloc(abr_pt_name, name);
  r->children = abr_single_child(p);
  abr_do_name(r, r);

  return r;
}

abr_parser *abr_n(const char *name)
{
  return abr_parser_malloc(abr_pt_n, name);
}

abr_parser *abr_r(const char *code)
{
  return abr_n_r(NULL, code);
}

abr_parser *abr_n_r(const char *name, const char *code)
{
  abr_parser *r = abr_parser_malloc(abr_pt_r, name);
  r->string = strdup(code);
  abr_do_name(r, r);

  return r;
}

abr_parser *abr_q(const char *code)
{
  return abr_n_q(NULL, code);
}

abr_parser *abr_n_q(const char *name, const char *code)
{
  abr_parser *r = abr_parser_malloc(abr_pt_q, name);
  r->string = strdup(code);
  abr_do_name(r, r);

  return r;
}

//
// the to_s methods

typedef void abr_p_to_s_func(flu_sbuffer *, flu_list *, int, abr_parser *);

static void abr_p_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p);

static void abr_p_string_to_s( // works for range and rex as well
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  if (p->name == NULL)
    flu_sbprintf(
      b, "abr_%s(\"%s\") /* %s */",
      abr_p_names[p->type], p->string, p->id);
  else
    flu_sbprintf(
      b, "abr_n_%s(\"%s\", \"%s\") /* %s */",
      abr_p_names[p->type], p->name, p->string, p->id);
}

static void abr_p_rep_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  if (p->name == NULL)
  {
    flu_sbprintf(b, "abr_rep( /* %s */\n", p->id);
  }
  else
  {
    flu_sbprintf(b, "abr_n_rep( /* %s */\n", p->id);
    for (int i = 0; i < indent + 1; i++) flu_sbprintf(b, "  ");
    flu_sbprintf(b, "\"%s\",\n", p->name);
  }
  abr_p_to_s(b, seen, indent + 1, p->children[0]);
  flu_sbprintf(b, ", %i, %i)", p->min, p->max);
}

static void abr_p_wchildren_to_s(
  const char *n, flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  if (p->name == NULL)
  {
    flu_sbprintf(b, "abr_%s( /* %s */\n", n, p->id);
  }
  else
  {
    flu_sbprintf(b, "abr_n_%s( /* %s */\n", n, p->id);
    for (int i = 0; i < indent + 1; i++) flu_sbprintf(b, "  ");
    flu_sbprintf(b, "\"%s\",\n", p->name);
  }
  if (p->children != NULL) for (size_t i = 0; ; i++)
  {
    abr_parser *c = p->children[i];
    abr_p_to_s(b, seen, indent + 1, c);
    if (c == NULL) break;
    flu_sbprintf(b, ",\n");
  }
  flu_sbprintf(b, ")");
}

static void abr_p_alt_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  abr_p_wchildren_to_s("alt", b, seen, indent, p);
}

static void abr_p_seq_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  abr_p_wchildren_to_s("seq", b, seen, indent, p);
}

static void abr_p_name_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_name( /* %s */\n", p->id);
  for (int i = 0; i < indent + 1; i++) flu_sbprintf(b, "  ");
  flu_sbprintf(b, "\"%s\",\n", p->name);
  abr_p_to_s(b, seen, indent + 1, p->children[0]);
  flu_sbprintf(b, ")");
}

static void abr_p_not_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_not(...)");
}

static void abr_p_presence_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_presence(...)");
}

static void abr_p_absence_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_absence(...)");
}

static void abr_p_n_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_n(\"%s\") /* %s */", p->name, p->id);
  if (p->children == NULL) flu_sbprintf(b, " /* not linked */", p->name);
  //else flu_sbprintf(b, " /* linked */", p->name);
}

static void abr_p_r_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_r(\"%s\") /* %s */", p->string, p->id);
}

static void abr_p_q_to_s(
  flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  flu_sbprintf(b, "abr_q(\"%s\") /* %s */", p->string, p->id);
}

abr_p_to_s_func *abr_p_to_s_funcs[] = { // const ?
  abr_p_string_to_s,
  abr_p_rep_to_s,
  abr_p_alt_to_s,
  abr_p_seq_to_s,
  abr_p_not_to_s,
  abr_p_name_to_s,
  abr_p_presence_to_s,
  abr_p_absence_to_s,
  abr_p_n_to_s,
  abr_p_r_to_s,
  abr_p_q_to_s,
  abr_p_string_to_s, // range
  abr_p_string_to_s, // rex
  abr_p_string_to_s  // "error" parser
};

void abr_p_to_s(flu_sbuffer *b, flu_list *seen, int indent, abr_parser *p)
{
  for (int i = 0; i < indent; i++) flu_sbprintf(b, "  ");
  if (p == NULL)
  {
    flu_sbprintf(b, "NULL");
  }
  else
  {
    int r = flu_list_add_unique(seen, p);
    if (r) abr_p_to_s_funcs[p->type](b, seen, indent, p);
    else flu_sbprintf(b, "abr_n(\"%s\") /* %s */", p->name, p->id);
  }
}

char *abr_parser_to_string(abr_parser *p)
{
  if (p->id == NULL) abr_set_ids(p);

  flu_sbuffer *b = flu_sbuffer_malloc();
  flu_list *seen = flu_list_malloc();

  abr_p_to_s(b, seen, 0, p);

  flu_list_free(seen);

  return flu_sbuffer_to_string(b);
}

char *abr_parser_to_s(abr_parser *p)
{
  //if (p->id == NULL) abr_set_ids(p);

  size_t ccount = 0;
  if (p->children) while (p->children[ccount] != NULL) { ++ccount; }

  char *name = "";
  if (p->name) name = flu_sprintf("'%s' ", p->name);

  char *string = "";
  if (p->string) string = flu_sprintf("\"%s\" ", p->string);

  char *minmax = "";
  if (p->type == abr_pt_rep) minmax = flu_sprintf(" mn%i mx%i", p->min, p->max);

  char *s = flu_sprintf(
    "%s t%i %s%sc%i%s",
    abr_p_names[p->type], p->type, name, string, ccount, minmax);

  if (*name != '\0') free(name);
  if (*string != '\0') free(string);
  if (*minmax != '\0') free(minmax);

  return s;
}

//
// the parse methods

// TODO: make the parse methods static

typedef abr_tree *abr_p_func(
  const char *, size_t, size_t, abr_parser *, int flags);
//
static abr_tree *abr_do_parse(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags);

abr_tree *abr_p_string(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  if (p->min == -1) p->min = strlen(p->string);

  int su = 1;
  size_t le = p->min;

  if (strncmp(input + offset, p->string, le) != 0) { su = 0; le = 0; }

  return abr_tree_malloc(su, offset, le, NULL, p, NULL);
}

abr_tree *abr_p_rep(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  short result = 1;
  size_t off = offset;
  size_t length = 0;

  abr_tree *first = NULL;
  abr_tree *prev = NULL;

  size_t count = 0;
  //
  for (; ; count++)
  {
    if (p->max > 0 && count >= p->max) break;
    abr_tree *t = abr_do_parse(input, off, depth + 1, p->children[0], flags);

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

  return abr_tree_malloc(result, offset, length, NULL, p, first);
}

abr_tree *abr_p_alt(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  short result = 0;
  size_t length = 0;

  abr_tree *first = NULL;
  abr_tree *prev = NULL;

  for (size_t i = 0; p->children[i] != NULL; i++)
  {
    abr_parser *pc = p->children[i];

    abr_tree *t = abr_do_parse(input, offset, depth + 1, pc, flags);

    if (first == NULL) first = t;
    if (prev != NULL) prev->sibling = t;
    prev = t;

    result = t->result;
    if (result < 0) { break; }
    if (result == 1) { length = t->length; break; }
  }

  return abr_tree_malloc(result, offset, length, NULL, p, first);
}

abr_tree *abr_p_seq(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  short result = 1;
  size_t length = 0;
  size_t off = offset;

  abr_tree *first = NULL;
  abr_tree *prev = NULL;

  for (size_t i = 0; p->children[i] != NULL; i++)
  {
    abr_parser *pc = p->children[i];

    abr_tree *t = abr_do_parse(input, off, depth + 1, pc, flags);

    if (first == NULL) first = t;
    if (prev != NULL) prev->sibling = t;
    prev = t;

    if (t->result != 1) { result = t->result; length = 0; break; }
    off += t->length;
    length += t->length;
  }

  return abr_tree_malloc(result, offset, length, NULL, p, first);
}

abr_tree *abr_p_name(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  abr_tree *t = abr_do_parse(input, offset, depth + 1, p->children[0], flags);

  return abr_tree_malloc(t->result, t->offset, t->length, NULL, p, t);
}

abr_tree *abr_p_n(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  if (p->children == NULL)
  {
    char *note = flu_sprintf("unlinked abr_n(\"%s\")", p->name);
    abr_tree *t = abr_tree_malloc(-1, offset, 0, note, p, NULL);
    free(note);
    return t;
  }
  return abr_do_parse(input, offset, depth, p->children[0], flags);
}

void abr_range_next(char *range, char *next)
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

abr_tree *abr_p_range(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  char *range = p->string;
  char c = (input + offset)[0];

  if (strcmp(range, "$") == 0)
  {
    return abr_tree_malloc(c == '\0', offset, 0, NULL, p, NULL);
  }

  if (c == '\0') return abr_tree_malloc(0, offset, 0, NULL, p, NULL);

  short success = 0;

  if (strcmp(range, ".") == 0)
  {
    success = (c != '\n');
    return abr_tree_malloc(success, offset, success ? 1 : 0, NULL, p, NULL);
  }

  short not = (range[0] == '^'); if (not) ++range;

  char *next = calloc(3, sizeof(char));
  while (1)
  {
    abr_range_next(range, next);
    if (next[0] == 0) break;
    if (c >= next[1] && c <= next[2]) { success = 1; break; }
    range = range + next[0];
  }
  free(next);

  if (not) success = ( ! success);
  return abr_tree_malloc(success, offset, success ? 1 : 0, NULL, p, NULL);
}

abr_tree *abr_p_rex(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  abr_tree *t = abr_do_parse(input, offset, depth + 1, p->children[0], flags);

  abr_tree *r = abr_tree_malloc(t->result, offset, t->length, NULL, p, NULL);

  if (flags & ABR_F_PRUNE && t->result == 1)
    abr_tree_free(t);
  else
    r->child = t;

  return r;
}

abr_tree *abr_p_error(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  return abr_tree_malloc(-1, offset, 0, strdup(p->string), p, NULL);
}

abr_tree *abr_p_not_implemented(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  char *s0 = abr_parser_to_string(p);
  char *s1 = flu_sprintf("not implemented %s", s0);
  abr_tree *t = abr_tree_malloc(-1, offset, 0, s1, p, NULL);
  free(s0);
  free(s1);
  return t;
}

abr_p_func *abr_p_funcs[] = { // const ?
  abr_p_string,
  abr_p_rep,
  abr_p_alt,
  abr_p_seq,
  abr_p_not_implemented, //abr_p_not,
  abr_p_name,
  abr_p_not_implemented, //abr_p_presence,
  abr_p_not_implemented, //abr_p_absence,
  abr_p_n,
  abr_p_not_implemented, //abr_p_r
  abr_p_not_implemented, //abr_p_q
  abr_p_range,
  abr_p_rex,
  abr_p_error
};

static abr_tree *abr_do_parse(
  const char *input,
  size_t offset, size_t depth,
  abr_parser *p,
  int flags)
{
  //printf("input >%s<\n", input + offset);

  if (depth > MAX_DEPTH)
  {
    return abr_tree_malloc(
      -1, offset, 0, "too much recursion, parser loop?", p, NULL);
  }

  abr_tree *t = abr_p_funcs[p->type](input, offset, depth, p, flags);

  if ((flags & ABR_F_PRUNE) == 0 || t->child == NULL) return t;

  abr_tree *first = t->child;
  t->child = NULL;
  abr_tree **sibling = &t->child;
  abr_tree *next = NULL;
  for (abr_tree *c = first; c != NULL; c = next)
  {
    next = c->sibling;
    c->sibling = NULL;

    if (t->result == 0 || c->result == 0)
    {
      abr_tree_free(c);
    }
    else
    {
      *sibling = c;
      sibling = &c->sibling;
    }
  }

  return t;
}


//
// entry point

abr_tree *abr_parse(const char *input, size_t offset, abr_parser *p)
{
  return abr_parse_f(input, offset, p, ABR_F_PRUNE);
}

abr_tree *abr_parse_all(const char *input, size_t offset, abr_parser *p)
{
  return abr_parse_f(input, offset, p, ABR_F_PRUNE | ABR_F_ALL);
}

abr_tree *abr_parse_f(
  const char *input, size_t offset, abr_parser *p, int flags)
{
  if (p->id == NULL) abr_set_ids(p);

  abr_tree *t = abr_do_parse(input, offset, 0, p, flags);

  if ((flags & ABR_F_ALL) == 0) return t;

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


//
// helper functions

char *abr_error_message(abr_tree *t)
{
  if (t->result == -1 && t->note != NULL) return t->note;

  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    char *s = abr_error_message(c);
    if (s != NULL) return s;
  }

  return NULL;
}

abr_tree *abr_tree_lookup(abr_tree *t, const char *name)
{
  if (t->name != NULL && strcmp(t->name, name) == 0) return t;

  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    abr_tree *r = abr_tree_lookup(c, name);
    if (r != NULL) return r;
  }

  return NULL;
}

static void abr_t_list(flu_list *l, abr_tree *t, abr_tree_func *f)
{
  short r = f(t);

  if (r < 0) { return; }
  if (r > 0) { flu_list_add(l, t); return; }

  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    abr_t_list(l, c, f);
  }
}

flu_list *abr_tree_list(abr_tree *t, abr_tree_func *f)
{
  flu_list *l = flu_list_malloc();

  abr_t_list(l, t, f);

  return l;
}

static void abr_t_list_named(flu_list *l, abr_tree *t, const char *name)
{
  if (t->result != 1) { return; }
  if (t->name && strcmp(t->name, name) == 0) { flu_list_add(l, t); return; }

  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    abr_t_list_named(l, c, name);
  }
}

flu_list *abr_tree_list_named(abr_tree *t, const char *name)
{
  flu_list *l = flu_list_malloc();

  abr_t_list_named(l, t, name);

  return l;
}

abr_tree **abr_tree_collect(abr_tree *t, abr_tree_func *f)
{
  flu_list *l = abr_tree_list(t, f);

  abr_tree **ts = (abr_tree **)flu_list_to_array(l, FLU_F_EXTRA_NULL);
  flu_list_free(l);

  return ts;
}

abr_parser *abr_p_child(abr_parser *p, size_t index)
{
  // expensive, but safer...
  // but well, what's the point the abr_parser struct is public...

  if (p->children) for (size_t i = 0; p->children[i] != NULL; i++)
  {
    if (index == 0) return p->children[i];
    --index;
  }

  return NULL;
}

abr_tree *abr_t_child(abr_tree *t, size_t index)
{
  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (index == 0) return c;
    --index;
  }

  return NULL;
}

//
// abr_rex

static ssize_t abr_find_range_end(const char *s)
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

static ssize_t abr_find_group_end(const char *s)
{
  for (size_t i = 0, stack = 0, range = 0; ; ++i)
  {
    char c = s[i];

    if (c == '\0') break;
    if (c == '\\') continue;

    if (c == '[') { range = 1; continue; };
    if (c == ']') { range = 0; continue; };
    if (range) continue;

    if (stack == 0 && c == ')') return i;
    if (c == ')') --stack;
    if (c == '(') ++stack;
  }

  return -1;
}

static size_t abr_parse_rex_quant(const char *s, abr_parser *p)
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

static abr_parser *abr_error(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  flu_sbuffer *b = flu_sbuffer_malloc();
  flu_sbvprintf(b, format, ap);
  va_end(ap);

  abr_parser *p = abr_parser_malloc(abr_pt_error, NULL);
  p->string = flu_sbuffer_to_string(b);

  return p;
}

static abr_parser *abr_regroup_rex(abr_p_type t, flu_list *children)
{
  abr_parser *p = abr_parser_malloc(t, NULL);

  flu_list *l = flu_list_malloc();

  for (flu_node *n = children->first; n != NULL; n = n->next)
  {
    abr_parser *pp = (abr_parser *)n->item;

    if (pp->type != t) { flu_list_unshift(l, pp); continue; }

    size_t s = 0; while (pp->children[s] != NULL) ++s;
    for (size_t i = s; i > 0; --i) flu_list_unshift(l, pp->children[i - 1]);
    free(pp->children);
    pp->children = NULL;
    abr_parser_free(pp);
  }

  p->children = (abr_parser **)flu_list_to_array(l, FLU_F_EXTRA_NULL);

  flu_list_free(l);

  return p;
}

static abr_parser *abr_decompose_rex_sequence(const char *s, ssize_t n)
{
//printf("adrs(\"%s\", %i) \"%s\"\n", s, n, strndup(s, n));
  size_t sl = strlen(s);

  flu_list *children = flu_list_malloc();

  abr_parser *p = NULL;
  char *ss = NULL;
  size_t ssi = 0;

  for (size_t si = 0; ; ++si)
  {
    char c = (si == n) ? '\0' : s[si];

    if (c == '\0') { break; }

    if (c == '?' || c == '*' || c == '+' || c == '{')
    {
      if (p == NULL && children->size < 1)
      {
        p = abr_error("orphan quantifier >%s<", s + si);
        flu_list_unshift(children, p);
        break;
      }

      abr_parser *r = abr_parser_malloc(abr_pt_rep, NULL);
      si = si - 1 + abr_parse_rex_quant(s + si, r);

      if (p == NULL || p->type != abr_pt_string || strlen(p->string) == 1)
      {
        r->children = abr_single_child((abr_parser *)children->first->item);
        flu_list_shift(children);
      }
      else // have to grab the last char in the current string...
      {
        size_t ci = strlen(p->string) - 1;
        abr_parser *p0 = abr_parser_malloc(abr_pt_string, NULL);
        p0->string = calloc(2, sizeof(char));
        p0->string[0] = p->string[ci];
        p->string[ci] = '\0';
        r->children = abr_single_child(p0);
      }
      flu_list_unshift(children, r);
      p = NULL;
      continue;
    }

    if (c == '[')
    {
      ssize_t ei = abr_find_range_end(s + si + 1);
      if (ei == -1)
      {
        p = abr_error("range not closed >%s<", s + si);
        flu_list_unshift(children, p);
        break;
      }
      abr_parser *r = abr_parser_malloc(abr_pt_range, NULL);
      r->string = strndup(s + si + 1, ei);
      flu_list_unshift(children, r);
      p = NULL;
      si = si + ei + 1;
//printf("post range >%s<\n", s + si + 1);
      continue;
    }

    if (c == '(')
    {
      ssize_t ei = abr_find_group_end(s + si + 1);
//printf("group end for >%s< is at %i\n", s + si + 1, ei);
      if (ei == -1)
      {
        p = abr_error("group not closed >%s<", s + si);
        flu_list_unshift(children, p);
        break;
      }
      abr_parser *g = abr_decompose_rex_group(s + si + 1, ei);
      flu_list_unshift(children, g);
      p = NULL;
      si = si + ei + 1;
//printf("post group >%s<\n", s + si + 1);
      continue;
    }

    if (c == '.' || c == '$')
    {
      abr_parser *r = abr_parser_malloc(abr_pt_range, NULL);
      r->string = calloc(2, sizeof(char));
      r->string[0] = c;
      flu_list_unshift(children, r);
      p = NULL;
      continue;
    }

    if (p == NULL || p->type != abr_pt_string) {
      p = abr_parser_malloc(abr_pt_string, NULL);
      p->string = calloc(sl - si + 1, sizeof(char));
      flu_list_unshift(children, p);
      ss = p->string;
      ssi = 0;
    }

    if (c == '\\') { ss[ssi++] = s[++si]; continue; }

    ss[ssi++] = c;
  }

  if (children->size > 1)
    p = abr_regroup_rex(abr_pt_seq, children);
  else /*if (children->size == 1)*/
    p = (abr_parser *)children->first->item;
  //else
    //p = NULL;

  flu_list_free(children);

  return p;
}

static abr_parser *abr_decompose_rex_group(const char *s, ssize_t n)
{
//printf("adrG(\"%s\", %i) \"%s\"\n", s, n, strndup(s, n));
  flu_list *children = flu_list_malloc();

  for (size_t i = 0, j = 0, stack = 0, range = 0; ; j++)
  {
    char c = (j == n) ? '\0' : s[j];

    if (c == '\\') continue;

    if (range && c != ']' && c != '\0') continue;
    if (range && c == ']') { range = 0; continue; }
    //
    if (c == '[') { range = 1; continue; }

    if (c == '(') { ++stack; continue; }
    if (c == ')') { --stack; continue; }

    if (c == '|' && stack > 0) continue;

    if (c == '\0' || c == '|')
    {
      abr_parser *p = abr_decompose_rex_sequence(s + i, j - i);
      flu_list_unshift(children, p);
      i = j + 1;
      if (c == '\0') break;
    }
  }

  abr_parser *p = NULL;

  if (children->size > 1)
    p = abr_regroup_rex(abr_pt_alt, children);
  else /*if (children->size == 1)*/
    p = (abr_parser *)children->first->item;
  //else
    //p = NULL;

  flu_list_free(children);

  return p;
}

