
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

#include "djan.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "aabro.h"
#include "flutil.h"


//
// dja_value malloc/free

static dja_value *dja_value_malloc(
  char type, char *input, size_t off, size_t len)
{
  dja_value *v = calloc(1, sizeof(dja_value));
  v->key = NULL;
  v->type = type;
  v->source = input;
  v->soff = off;
  v->slen = len;
  v->sibling = NULL;
  v->child = NULL;

  return v;
}

void dja_value_free(dja_value *v)
{
  if (v->key != NULL) free(v->key);
  if (v->slen == 0 && v->source != NULL) free(v->source);

  for (dja_value *c = v->child, *next = NULL; c != NULL; )
  {
    next = c->sibling;
    dja_value_free(c);
    c = next;
  }

  free(v);
}


//
// parsing

static abr_parser *dja_parser = NULL;
static abr_parser *dja_radial_parser = NULL;

static void dja_parser_init()
{
  if (dja_parser != NULL) return;

  // djan (JSON & co)

  abr_parser *blanks = // blanks and comments
    abr_rex("([ \t]*((#[^\r\n]*)?([\r\n]|$))?)*");

  abr_parser *string =
    abr_n_rex(
      "string",
      "\"("
        //"\\\\." "|"
        "\\\\[\"\\/\\\\bfnrt]" "|"
        "\\\\u[0-9a-fA-F]{4}" "|"
        "[^\"\\\\]"
      ")*\"");
  abr_parser *sqstring =
    abr_n_rex("sqstring", "'(\\\\'|[^'])*'");

  abr_parser *symbol =
    abr_n_rex("symbol", "[a-zA-Z_][a-zA-Z_0-9]*");

  abr_parser *entry =
    abr_n_seq(
      "entry",
      blanks,
      abr_n_alt("key", string, sqstring, symbol, NULL),
      blanks,
      abr_string(":"),
      abr_n("value"),
      NULL);

  abr_parser *entries =
    abr_n_seq(
      "entries",
      entry,
      abr_seq(abr_rex(",?"), entry, abr_r("*")),
      abr_r("?")
    );

  abr_parser *object =
    abr_n_seq("object", abr_string("{"), entries, abr_string("}"), NULL);

  abr_parser *values =
    abr_n_seq(
      "values",
      abr_n("value"),
      abr_seq(abr_rex(",?"), abr_n("value"), abr_r("*")),
      abr_r("?")
    );

  abr_parser *array =
    abr_n_seq("array", abr_string("["), values, abr_string("]"), NULL);

  abr_parser *pure_value =
    abr_alt(
      string,
      sqstring,
      abr_n_rex("number", "-?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?"),
      object,
      array,
      abr_n_string("true", "true"),
      abr_n_string("false", "false"),
      abr_n_string("null", "null"),
      symbol,
      NULL);

  dja_parser =
    abr_n_seq("value", blanks, pure_value, blanks, NULL);

  // radial

  abr_parser *spaces = abr_rex("[ \t]*");

  abr_parser *rad_i = abr_name("rad_i", spaces);
  abr_parser *rad_n = abr_name("rad_n", symbol);

  abr_parser *rad_a =
    abr_n_seq("rad_a", spaces, pure_value, abr_n_r("", "?"));

  abr_parser *rad_e =
    abr_n_seq(
      "rad_e",
      abr_seq(spaces, abr_rex(",?"), blanks, NULL),
      abr_n_alt("key", string, sqstring, symbol, NULL),
      spaces,
      abr_string(":"),
      abr_n_seq("val", blanks, pure_value, NULL),
      NULL);

  abr_parser *rad_as =
    abr_n_rep("rad_as", rad_e, 0, -1);

  abr_parser *rad_eol =
    abr_rex("[ \t]*(#[^\n\r]*)?");

  abr_parser *rad_l =
    abr_n_seq("rad_l", rad_i, rad_n, rad_a, rad_as, NULL);

  abr_parser *rad_line =
    abr_seq(rad_l, abr_q("?"), rad_eol, NULL);

  dja_radial_parser =
    abr_seq(
      rad_line,
      abr_seq(
        abr_rex("[\n\r]+"),
        rad_line,
        abr_r("*")),
      NULL);
}

// forward declarations
static dja_value *dja_extract_value(char *input, abr_tree *t);

static char *dja_sq_unescape(const char *s, size_t n)
{
  char *r = calloc(n + 1, sizeof(char));
  for (size_t i = 0, j = 0; i < n; i++)
  {
    char c = s[i];
    if (c == '\0') break;
    if (c != '\\') r[j++] = c;
  }
  return r;
}

static char *dja_extract_key(char *input, abr_tree *t)
{
  //printf("dek()\n%s\n", abr_tree_to_string_with_leaves(input, t));

  abr_tree *c = t->child;
  while (c->result != 1) c = c->sibling; // unpruned trees are ok too

  if (strcmp(c->name, "string") == 0)
    return flu_n_unescape(input + c->offset + 1, c->length - 2);

  if (strcmp(c->name, "sqstring") == 0)
    return dja_sq_unescape(input + c->offset + 1, c->length - 2);

  //if (strcmp(c->name, "symbol") == 0)
  return strndup(input + c->offset, c->length);
}

static dja_value *dja_extract_entries(char *input, abr_tree *t)
{
  //printf("%s\n", abr_tree_to_string_with_leaves(input, t));

  flu_list *ts = abr_tree_list_named(t, "entry");

  dja_value *first = NULL;
  dja_value *child = NULL;

  for (flu_node *n = ts->first; n != NULL; n = n->next)
  {
    abr_tree *tt = (abr_tree *)n->item;
    //printf("**\n%s\n", abr_tree_to_string_with_leaves(input, tt));
    dja_value *v = dja_extract_value(input, abr_t_child(tt, 4));
    v->key = dja_extract_key(input, abr_t_child(tt, 1));
    if (first == NULL) first = v;
    if (child != NULL) child->sibling = v;
    child = v;
  }

  flu_list_free(ts);

  return first;
}

static dja_value *dja_extract_values(char *input, abr_tree *t)
{
  //printf("%s\n", abr_tree_to_string(t));

  flu_list *ts = abr_tree_list_named(t, "value");

  dja_value *first = NULL;
  dja_value *child = NULL;

  for (flu_node *n = ts->first; n != NULL; n = n->next)
  {
    //printf("** %s\n", abr_tree_to_string(ts[i]));
    dja_value *v = dja_extract_value(input, (abr_tree *)n->item);
    if (first == NULL) first = v;
    if (child != NULL) child->sibling = v;
    child = v;
  }

  flu_list_free(ts);

  return first;
}

static dja_value *dja_extract_v(char *input, abr_tree *t)
{
  //printf("de_v() %s\n", abr_tree_to_string_with_leaves(input, t));

  char ty = '-';

  if (strcmp(t->name, "string") == 0) ty = 's';
  else if (strcmp(t->name, "sqstring") == 0) ty = 'q';
  else if (strcmp(t->name, "symbol") == 0) ty = 'y';
  else if (strcmp(t->name, "number") == 0) ty = 'n';
  else if (strcmp(t->name, "true") == 0) ty = 't';
  else if (strcmp(t->name, "false") == 0) ty = 'f';
  else if (strcmp(t->name, "null") == 0) ty = '0';
  else if (strcmp(t->name, "array") == 0) ty = 'a';
  else if (strcmp(t->name, "object") == 0) ty = 'o';

  if (ty == '-') return NULL;

  dja_value *v = dja_value_malloc(ty, input, t->offset, t->length);

  if (ty == 'o') v->child = dja_extract_entries(input, abr_t_child(t, 1));
  else if (ty == 'a') v->child = dja_extract_values(input, abr_t_child(t, 1));

  return v;
}

static dja_value *dja_extract_value(char *input, abr_tree *t)
{
  //printf("dev() %s\n", abr_tree_to_string_with_leaves(input, t));

  if (t->result != 1) return NULL;

  t = abr_t_child(t, 1);

  for (abr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (c->result == 1) return dja_extract_v(input, c);
  }

  return NULL;
}

dja_value *dja_parse(char *input)
{
  dja_parser_init();

  abr_tree *t = abr_parse_all(input, 0, dja_parser);
  // TODO: deal with errors (t->result < 0)

  //printf(">%s<\n", input);
  //puts(abr_parser_to_string(t->parser));
  //puts(abr_tree_to_string_with_leaves(input, t));

  dja_value *v = dja_extract_value(input, t);
  abr_tree_free(t);

  return v;
}

static void dja_add_radc(dja_value *parent, dja_value *child)
{
  parent = dja_value_at(parent, 2);

  if (parent->child == NULL)
  {
    parent->child = child;
  }
  else {
    for (dja_value *c = parent->child; ; c = c->sibling)
    {
      if (c->sibling == NULL) { c->sibling = child; break; }
    }
  }
}

static void dja_stack_radl(flu_list *values, dja_value *v)
{
  long i = (long)v->soff; // indentation

  dja_value *current = NULL;
  long ci = -1;
  if (values->size > 0)
  {
    current = (dja_value *)values->first->item;
    ci = current->soff;
  }

  if (i < ci)
  {
    // go closer to the root
    flu_list_shift(values);
    dja_stack_radl(values, v);
  }
  else if (i == ci)
  {
    // replace current
    flu_list_shift(values);
    dja_add_radc((dja_value *)values->first->item, v);
    flu_list_unshift(values, v);
  }
  else
  {
    // add here
    if (current != NULL) dja_add_radc(current, v);
    flu_list_unshift(values, v);
  }
}

static void dja_parse_radl(char *input, abr_tree *radl, flu_list *values)
{
  //printf("%s\n", abr_tree_to_string_with_leaves(input, radl));

  abr_tree *radi = abr_tree_lookup(radl, "rad_i");
  abr_tree *radn = abr_tree_lookup(radl, "rad_n");
  abr_tree *rada = abr_tree_lookup(radl, "rad_a");
  abr_tree *radas = abr_tree_lookup(radl, "rad_as");

  size_t i = radi->length; // indentation

  // [ "sequence", {}, [] ]
  dja_value *v = dja_value_malloc('a', NULL, i, 0);
  dja_value *vname = dja_extract_v(input, radn->child);
  dja_value *vatts = dja_value_malloc('o', NULL, 0, 0);
  dja_value *vchildren = dja_value_malloc('a', NULL, 0, 0);

  // single "_a" attribute
  if (rada != NULL)
  {
    dja_value *va = dja_extract_value(input, rada);
    if (va != NULL) va->key = strdup("_a");
    vatts->child = va;
  }

  // attributes
  dja_value **anext = &vatts->child;
  if (vatts->child != NULL) anext = &vatts->child->sibling;
  flu_list *as = abr_tree_list_named(radas, "rad_e");
  for (flu_node *n = as->first; n != NULL; n = n->next)
  {
    abr_tree *ak = abr_t_child(n->item, 1);
    abr_tree *av = abr_t_child(n->item, 4);
    dja_value *va = dja_extract_value(input, av);
    va->key = abr_tree_string(input, ak);
    *anext = va;
    anext = &va->sibling;
  }
  flu_list_free(as);

  v->child = vname;
  vname->sibling = vatts;
  vatts->sibling = vchildren;

  dja_stack_radl(values, v);
}

dja_value *dja_parse_radial(char *input)
{
  dja_parser_init();

  abr_tree *t = abr_parse_all(input, 0, dja_radial_parser);
  // TODO: deal with errors (t->result < 0)

  //printf(">%s<\n", input);
  //puts(abr_tree_to_string_with_leaves(input, t));

  flu_list *ls = abr_tree_list_named(t, "rad_l");
  flu_list *vs = flu_list_malloc();

  if (ls->size > 0) for (flu_node *n = ls->first; n != NULL; n = n->next)
  {
    dja_parse_radl(input, (abr_tree *)n->item, vs);
  }

  flu_list_free(ls);
  abr_tree_free(t);

  dja_value *root = NULL;
  if (vs->size > 0) root = (dja_value *)vs->last->item;
  flu_list_free(vs);

  return root;
}


//
// outputting

static void dja_to_j(flu_sbuffer *b, dja_value *v)
{
  if (v->key != NULL)
  {
    flu_sbprintf(b, "\"%s\":", v->key);
  }

  if (v->type == 'q')
  {
    char *s = dja_to_string(v);
    flu_sbprintf(b, "\"%s\"", s);
    free(s);
  }
  else if (v->type == 'y')
  {
    char *s = dja_string(v);
    flu_sbprintf(b, "\"%s\"", s);
    free(s);
  }
  else if (v->type == 'a')
  {
    flu_sbputc(b, '[');
    for (dja_value *c = v->child; c != NULL; c = c->sibling)
    {
      dja_to_j(b, c);
      if (c->sibling != NULL) flu_sbputc(b, ',');
    }
    flu_sbputc(b, ']');
  }
  else if (v->type == 'o')
  {
    flu_sbputc(b, '{');
    for (dja_value *c = v->child; c != NULL; c = c->sibling)
    {
      dja_to_j(b, c);
      if (c->sibling != NULL) flu_sbputc(b, ',');
    }
    flu_sbputc(b, '}');
  }
  else if (v->slen == 0) flu_sbputs(b, v->source + v->soff);
  else flu_sbputs_n(b, v->source + v->soff, v->slen);
}

char *dja_to_json(dja_value *v)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  dja_to_j(b, v);

  return flu_sbuffer_to_string(b);
}


//
// extracting stuff out of dja_value items

char *dja_string(dja_value *v)
{
  if (v->slen == 0) return strdup(v->source + v->soff);
  return strndup(v->source + v->soff, v->slen);
}

char *dja_to_string(dja_value *v)
{
  if (v->type == 's')
  {
    return flu_n_unescape(v->source + v->soff + 1, v->slen - 2);
  }
  if (v->type == 'q')
  {
    return dja_sq_unescape(v->source + v->soff + 1, v->slen - 2);
  }
  return dja_string(v);
}

int dja_to_int(dja_value *v)
{
  if (v->type == 't') return 1;
  if (v->type == 'f') return 0;
  if (v->type != 'n') return -1;
  return atoi(v->source + v->soff);
}

double dja_to_double(dja_value *v)
{
  if (v->type != 'n') return 0.0;
  return atof(v->source + v->soff);
}

size_t dja_size(dja_value *v)
{
  size_t i = 0; for (dja_value *c = v->child; ; c = c->sibling)
  {
    if (c == NULL || i == SIZE_MAX) break;
    i++;
  }
  return i;
}

dja_value *dja_value_at(dja_value *v, long n)
{
  if (n < 0)
  {
    n = dja_size(v) + n;
    if (n < 0) return NULL;
  }

  size_t i = 0; for (dja_value *c = v->child; c != NULL; c = c->sibling)
  {
    if (i++ == n) return c;
  }

  return NULL;
}

dja_value *dja_lookup(dja_value *v, const char *path)
{
  long index = atol(path);

  return dja_value_at(v, index);
}

