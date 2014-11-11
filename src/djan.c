
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

// https://github.com/flon-io/djan

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "aabro.h"
#include "flutil.h"

#include "djan.h"


//
// fdja_value malloc/free

static fdja_value *fdja_value_malloc(
  char type, char *input, size_t off, size_t len, short owner)
{
  fdja_value *v = calloc(1, sizeof(fdja_value));
  v->key = NULL;
  v->type = type;
  v->source = input;
  v->soff = off;
  v->slen = len;
  v->sowner = owner;
  v->sibling = NULL;
  v->child = NULL;

  return v;
}

void fdja_value_free(fdja_value *v)
{
  if (v->key != NULL) free(v->key);
  if (v->sowner == 1 && v->source != NULL) free(v->source);

  for (fdja_value *c = v->child, *next = NULL; c != NULL; )
  {
    next = c->sibling;
    fdja_value_free(c);
    c = next;
  }

  free(v);
}


//
// parsing

static fabr_parser *fdja_parser = NULL;
static fabr_parser *fdja_obj_parser = NULL;
static fabr_parser *fdja_number_parser = NULL;
static fabr_parser *fdja_symbol_parser = NULL;
static fabr_parser *fdja_radial_parser = NULL;

static void fdja_parser_init()
{
  // djan (JSON & co)

  fabr_parser *blanks = // blanks and comments
    fabr_rex("([ \t]*((#[^\r\n]*)?([\r\n]|$))?)*");

  fabr_parser *string =
    fabr_n_rex(
      "string",
      "\""
        "("
          "\\\\[\"\\/\\\\bfnrt]" "|"
          "\\\\u[0-9a-fA-F]{4}" "|"
          "[^"
            "\"" "\\\\" /*"\\/"*/ "\b" "\f" "\n" "\r" "\t"
          "]"
        ")*"
      "\"");
  fabr_parser *sqstring =
    fabr_n_rex(
      "sqstring",
      "'"
        "("
          "\\\\['\\/\\\\bfnrt]" "|"
          "\\\\u[0-9a-fA-F]{4}" "|"
          "[^"
            "'" "\\\\" /*"\\/"*/ "\b" "\f" "\n" "\r" "\t"
          "]"
        ")*"
      "'");
  fabr_parser *symbol =
    fabr_n_rex(
      "symbol",
      "[^ \b\f\n\r\t\"':,\\[\\]\\{\\}#\\\\]+");

  fabr_parser *number =
    fabr_n_rex("number", "-?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?");

  fabr_parser *entry =
    fabr_n_seq(
      "entry",
      blanks,
      fabr_n_alt("key", string, sqstring, symbol, NULL),
      blanks,
      fabr_string(":"),
      fabr_n("value"),
      NULL);

  fabr_parser *entries =
    fabr_n_seq(
      "entries",
      entry,
      fabr_seq(fabr_rex(",?"), entry, fabr_q("?"), fabr_r("*")),
      fabr_r("?")
    );

  fabr_parser *object =
    fabr_n_seq(
      "object",
      fabr_rex("\\{[ \t\n\r]*"), entries, fabr_rex("[ \t\n\r]*}"), NULL);

  fabr_parser *values =
    fabr_n_seq(
      "values",
      fabr_n("value"),
      fabr_seq(fabr_rex(",?"), fabr_n("value"), fabr_q("?"), fabr_r("*")),
      fabr_r("?")
    );

  fabr_parser *array =
    fabr_n_seq(
      "array",
      fabr_rex("\\[[ \t\n\r]*"), values, fabr_rex("[ \t\n\r]*]"), NULL);

  fabr_parser *pure_value =
    fabr_altg(
      fabr_alt(
        string,
        sqstring,
        number,
        object,
        array,
        fabr_n_string("true", "true"),
        fabr_n_string("false", "false"),
        fabr_n_string("null", "null"),
        NULL),
      symbol,
      NULL);

  fdja_parser =
    fabr_n_seq("value", blanks, pure_value, blanks, NULL);

  // number & symbol

  fdja_number_parser = number;
  fdja_symbol_parser = symbol;

  // radial

  fabr_parser *word = fabr_n_rex("symbol", "[^ \t\n\r,\\[\\]\\{\\}#]+");
  fabr_parser *spaces = fabr_rex("[ \t]*");

  fabr_parser *rad_i = fabr_name("rad_i", spaces);
  fabr_parser *rad_n = fabr_name("rad_n", word);

  fabr_parser *rad_e =
    fabr_n_seq(
      "rad_e",
      fabr_seq(
        spaces, fabr_seq(fabr_rex(","), blanks, fabr_r("?")), NULL),
      fabr_seq(
        fabr_n_alt("key", string, sqstring, symbol, NULL),
        spaces, fabr_str(":"), blanks,
        NULL), fabr_q("?"),
      fabr_name("val", pure_value),
      NULL);

  fabr_parser *rad_eol =
    fabr_rex("[ \t]*(#[^\n\r]*)?");

  fabr_parser *rad_l =
    fabr_n_seq("rad_l", rad_i, rad_n, rad_e, fabr_q("*"), NULL);

  fabr_parser *rad_line =
    fabr_seq(rad_l, fabr_q("?"), rad_eol, NULL);

  fdja_radial_parser =
    fabr_seq(
      rad_line,
      fabr_seq(
        fabr_rex("[\n\r]+"),
        rad_line,
        fabr_r("*")),
      NULL);

  // obj

  fdja_obj_parser =
    fabr_seq(
      fabr_rex("[ \t]*(#[^\n\r]*[\n\r]+)?"), fabr_q("*"),
      fabr_n_seq("object", fabr_rex("\\{?"), entries, fabr_rex("\\}?"), NULL),
      fabr_rex("[ \t\r\n]*(#[^\n\r]*)?"), fabr_q("*"),
      NULL);
}

static fabr_parser *fdja_path_parser = NULL;

static void fdja_path_parser_init()
{
  fabr_parser *index = fabr_n_rex("index", "-?[0-9]+");
  fabr_parser *key = fabr_n_rex("key", "[^\n\r\t\\.]+");
  fabr_parser *node = fabr_n_altg("node", index, key, NULL);

  fdja_path_parser =
    fabr_seq(
      node,
      fabr_seq(fabr_string("."), node, NULL), fabr_q("*"),
      NULL);
}

// forward declarations
static fdja_value *fdja_extract_value(char *input, fabr_tree *t);

static char *fdja_sq_unescape(const char *s, size_t n)
{
  char *ues = flu_n_unescape(s, n);
  size_t uel = strlen(ues);

  char *r = calloc(uel + 1, sizeof(char));

  for (size_t i = 0, j = 0; i < uel; i++)
  {
    char c = ues[i];
    if (c == '\\' && ues[i + 1] == '\'') { ++i; r[j++] = '\''; }
    else r[j++] = c;
  }

  free(ues);

  return r;
}

static char *fdja_extract_key(char *input, fabr_tree *t)
{
  //printf("dek()\n%s\n", fabr_tree_to_string(t, input));

  fabr_tree *c = t->child;
  while (c->result != 1) c = c->sibling; // unpruned trees are ok too

  if (strcmp(c->name, "string") == 0)
    return flu_n_unescape(input + c->offset + 1, c->length - 2);

  if (strcmp(c->name, "sqstring") == 0)
    //return fdja_sq_unescape(input + c->offset + 1, c->length - 2);
    return flu_n_unescape(input + c->offset + 1, c->length - 2);

  //if (strcmp(c->name, "symbol") == 0)
  return strndup(input + c->offset, c->length);
}

static fdja_value *fdja_extract_entries(char *input, fabr_tree *t)
{
  //printf("%s\n", fabr_tree_to_string(t, input));

  flu_list *ts = fabr_tree_list_named(t, "entry");

  fdja_value *first = NULL;
  fdja_value *child = NULL;

  for (flu_node *n = ts->first; n != NULL; n = n->next)
  {
    fabr_tree *tt = (fabr_tree *)n->item;
    //printf("**\n%s\n", fabr_tree_to_string(tt, input));
    fdja_value *v = fdja_extract_value(input, fabr_t_child(tt, 4));
    v->key = fdja_extract_key(input, fabr_t_child(tt, 1));
    if (first == NULL) first = v;
    if (child != NULL) child->sibling = v;
    child = v;
  }

  flu_list_free(ts);

  return first;
}

static fdja_value *fdja_extract_values(char *input, fabr_tree *t)
{
  //printf("%s\n", fabr_tree_to_string(t, input));

  flu_list *ts = fabr_tree_list_named(t, "value");

  fdja_value *first = NULL;
  fdja_value *child = NULL;

  for (flu_node *n = ts->first; n != NULL; n = n->next)
  {
    //printf("** %s\n", fabr_tree_to_string(ts[i]));
    fdja_value *v = fdja_extract_value(input, (fabr_tree *)n->item);
    if (first == NULL) first = v;
    if (child != NULL) child->sibling = v;
    child = v;
  }

  flu_list_free(ts);

  return first;
}

static fdja_value *fdja_extract_v(char *input, fabr_tree *t)
{
  if (t == NULL) return NULL;

  //printf("fdja_extract_v() %s\n", fabr_tree_to_string(t, input, 1));
  //printf("fdja_extract_v() %s\n", fabr_tree_to_str(t, input, 1));

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

  fdja_value *v = fdja_value_malloc(ty, input, t->offset, t->length, 0);

  if (ty == 'o') v->child = fdja_extract_entries(input, fabr_t_child(t, 1));
  else if (ty == 'a') v->child = fdja_extract_values(input, fabr_t_child(t, 1));

  return v;
}

static fdja_value *fdja_extract_value(char *input, fabr_tree *t)
{
  //printf("fdja_extract_value() %s\n", fabr_tree_to_string(t, input, 1));
  //printf("fdja_extract_value() %s\n", fabr_tree_to_str(t, input, 1));

  if (t->result != 1) return NULL;

  return fdja_extract_v(input, fabr_subtree_lookup(t, NULL));
}

fdja_value *fdja_parse(char *input)
{
  if (fdja_parser == NULL) fdja_parser_init();

  fabr_tree *t = fabr_parse_all(input, 0, fdja_parser);

  //printf(">%s<\n", input);
  //puts("[1;30m"); puts(fabr_parser_to_string(t->parser)); puts("[0;0m");
  //puts(fabr_tree_to_string(t, input, 1));

  fdja_value *v = fdja_extract_value(input, t);
  fabr_tree_free(t);

  if (v) v->sowner = 1;

  return v;
}

fdja_value *fdja_dparse(char *input)
{
  return fdja_parse(strdup(input));
}

fdja_value *fdja_parse_f(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *s = flu_vreadall(path, ap);
  va_end(ap);

  if (s == NULL) return NULL;

  fdja_value *v = fdja_parse(s);

  if (v == NULL) free(s);

  return v;
}

fdja_value *fdja_v(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *s = flu_svprintf(format, ap);
  va_end(ap);

  return fdja_parse(s);
}

char *fdja_vj(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *s = flu_svprintf(format, ap);
  va_end(ap);

  fdja_value *v = fdja_parse(s);

  if (v == NULL) { free(s); return NULL; }

  char *r = fdja_to_json(v);

  fdja_free(v);
  //free(s);

  return r;
}

fdja_value *fdja_s(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *s = flu_svprintf(format, ap);
  va_end(ap);

  char *ss = flu_escape(s);
  free(s);

  return fdja_value_malloc('y', ss, 0, strlen(ss), 1);
}

fdja_value *fdja_sym(char *s)
{
  return fdja_value_malloc('y', s, 0, strlen(s), 1);
}

static void fdja_add_radc(fdja_value *parent, fdja_value *child)
{
  parent = fdja_value_at(parent, 2);

  if (parent->child == NULL)
  {
    parent->child = child;
  }
  else {
    for (fdja_value *c = parent->child; ; c = c->sibling)
    {
      if (c->sibling == NULL) { c->sibling = child; break; }
    }
  }
}

static void fdja_stack_radl(flu_list *values, fdja_value *v)
{
  long i = (long)v->soff; // indentation

  fdja_value *current = NULL;
  long ci = -1;
  if (values->size > 0)
  {
    current = (fdja_value *)values->first->item;
    ci = current->soff;
  }

  if (i < ci)
  {
    // go closer to the root
    flu_list_shift(values);
    fdja_stack_radl(values, v);
  }
  else if (i == ci)
  {
    // replace current
    flu_list_shift(values);
    fdja_add_radc((fdja_value *)values->first->item, v);
    flu_list_unshift(values, v);
  }
  else
  {
    // add here
    if (current != NULL) fdja_add_radc(current, v);
    flu_list_unshift(values, v);
  }
}

static void fdja_parse_radl(char *input, fabr_tree *radl, flu_list *values)
{
  //printf("%s\n", fabr_tree_to_string(radl, input));

  fabr_tree *radi = fabr_tree_lookup(radl, "rad_i");
  fabr_tree *radn = fabr_tree_lookup(radl, "rad_n");

  size_t i = radi->length; // indentation

  // [ "sequence", {}, [] ]
  fdja_value *v = fdja_value_malloc('a', NULL, i, 0, 0);
  fdja_value *vname = fdja_extract_v(input, radn->child);
  fdja_value *vatts = fdja_value_malloc('o', NULL, 0, 0, 0);
  fdja_value *vchildren = fdja_value_malloc('a', NULL, 0, 0, 0);

  // attributes
  fdja_value **anext = &vatts->child;
  if (vatts->child != NULL) anext = &vatts->child->sibling;
  flu_list *as = fabr_tree_list_named(radl, "rad_e");
  size_t j = 0;
  for (flu_node *n = as->first; n != NULL; n = n->next)
  {
    fabr_tree *ak = fabr_subtree_lookup(n->item, "key");
    fabr_tree *av = fabr_subtree_lookup(n->item, "val");
    fdja_value *va = fdja_extract_value(input, av);
    va->key = ak ? fabr_tree_string(input, ak) : flu_sprintf("_%zu", j);
    *anext = va;
    anext = &va->sibling;
    ++j;
  }
  flu_list_free(as);

  v->child = vname;
  vname->sibling = vatts;
  vatts->sibling = vchildren;

  fdja_stack_radl(values, v);
}

fdja_value *fdja_parse_radial(char *input)
{
  if (fdja_parser == NULL) fdja_parser_init();

  fabr_tree *t = fabr_parse_all(input, 0, fdja_radial_parser);
  // TODO: deal with errors (t->result < 0)

  //printf(">%s<\n", input);
  //puts(fabr_tree_to_string(t, input));

  flu_list *ls = fabr_tree_list_named(t, "rad_l");
  flu_list *vs = flu_list_malloc();

  if (ls->size > 0) for (flu_node *n = ls->first; n != NULL; n = n->next)
  {
    fdja_parse_radl(input, (fabr_tree *)n->item, vs);
  }

  flu_list_free(ls);
  fabr_tree_free(t);

  fdja_value *root = NULL;
  if (vs->size > 0) root = (fdja_value *)vs->last->item;
  flu_list_free(vs);

  if (root == NULL) return NULL;

  root->source = input;
  root->sowner = 1;

  return root;
}

fdja_value *fdja_dparse_radial(char *input)
{
  return fdja_parse_radial(strdup(input));
}

fdja_value *fdja_parse_radial_f(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *s = flu_vreadall(path, ap);
  va_end(ap);

  if (s == NULL) return NULL;

  fdja_value *v = fdja_parse_radial(s);

  if (v == NULL) free(s);

  return v;
}

fdja_value *fdja_parse_obj(char *input)
{
  if (fdja_parser == NULL) fdja_parser_init();

  fabr_tree *t = fabr_parse_all(input, 0, fdja_obj_parser);

  //printf(">%s<\n", input);
  //puts("[1;30m"); puts(fabr_parser_to_string(t->parser)); puts("[0;0m");
  //puts(fabr_tree_to_string(t, input, 1));

  if (t->result != 1) { fabr_tree_free(t); return NULL; }

  fabr_tree *tt = fabr_t_child(t, 1);

  fdja_value *v = fdja_extract_v(input, tt);
  if (v) v->sowner = 1;

  fabr_tree_free(t);

  return v;
}

fdja_value *fdja_dparse_obj(char *input)
{
  return fdja_parse_obj(strdup(input));
}

fdja_value *fdja_parse_obj_f(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *s = flu_vreadall(path, ap);
  va_end(ap);

  if (s == NULL) return NULL;

  fdja_value *v = fdja_parse_obj(s);

  if (v == NULL) free(s);

  return v;
}

fdja_value *fdja_c(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *s = flu_svprintf(format, ap);
  va_end(ap);

  fdja_value *v = fdja_parse_obj(s);

  if (v == NULL) free(s);

  return v;
}

fdja_value *fdja_clone(fdja_value *v)
{
  return v ? fdja_parse(fdja_to_json(v)) : NULL;
}


//
// outputting

void fdja_to_j(FILE *f, fdja_value *v, size_t depth)
{
  if (v->key != NULL && depth > 0)
  {
    fprintf(f, "\"%s\":", v->key);
  }

  if (v->type == 'q')
  {
    fputc('"', f);
    for (size_t i = 0; i < v->slen - 2; ++i)
    {
      char *s = v->source + v->soff + 1 + i;
      if (s[0] == '\\' && s[1] == '\'') { ++i; fputc('\'', f); }
      else { fputc(s[0], f); }
    }
    fputc('"', f);
  }
  else if (v->type == 'y')
  {
    fputc('"', f);
    fwrite(v->source + v->soff, sizeof(char), v->slen, f);
    fputc('"', f);
  }
  else if (v->type == 'a')
  {
    fputc('[', f);
    for (fdja_value *c = v->child; c != NULL; c = c->sibling)
    {
      fdja_to_j(f, c, depth + 1);
      if (c->sibling != NULL) fputc(',', f);
    }
    fputc(']', f);
  }
  else if (v->type == 'o')
  {
    fputc('{', f);
    for (fdja_value *c = v->child; c != NULL; c = c->sibling)
    {
      fdja_to_j(f, c, depth + 1);
      if (c->sibling != NULL) fputc(',', f);
    }
    fputc('}', f);
  }
  else fwrite(v->source + v->soff, sizeof(char), v->slen, f);
}

char *fdja_to_json(fdja_value *v)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fdja_to_j(b->stream, v, 0);

  return flu_sbuffer_to_string(b);
}

int fdja_to_json_f(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *spath = flu_svprintf(path, ap);
  va_end(ap);

  FILE *f = fopen(spath, "w");

  free(spath);

  if (f == NULL) return 0;

  fdja_to_j(f, v, 0);

  return (fclose(f) == 0);
}

static int fdja_is_symbol(char *s)
{
  if (fdja_parser == NULL) fdja_parser_init();

  return fabr_match(s, fdja_symbol_parser);
}

static int fdja_is_number(char *s)
{
  if (fdja_parser == NULL) fdja_parser_init();

  return fabr_match(s, fdja_number_parser);
}

void fdja_to_d(FILE *f, fdja_value *v, int flags, size_t depth)
{
  short cl = flags & FDJA_F_COLOR;
  short ol = flags & FDJA_F_ONELINE;
  short cp = flags & FDJA_F_COMPACT;
  if (cp) ol = 1;

  char *infrac = cl ? "[1;30m" : "";
  char *keyc = cl ? "[0;33m" : "";
  char *stringc = cl ? "[1;33m" : "";
  char *truec = cl ? "[0;32m" : "";
  char *falsec = cl ? "[0;31m" : "";
  char *clearc = cl ? "[0;0m" : "";

  char *indent = flu_sprintf("%*s", (ol ? 0 : depth) * 2, "");
  char *s = NULL;

  fputs(indent, f);

  // key (if there is one)

  if (v->key && depth > 0)
  {
    fputs(keyc, f);

    if (fdja_is_symbol(v->key))
      fputs(v->key, f);
    else
      fprintf(f, "\"%s\"", v->key);

    fputs(clearc, f);

    fputc(':', f); if ( ! cp) fputc(' ', f);
  }

  // actual value

  if (v->type == 'q' || v->type == 's' || v->type == 'y')
  {
    fputs(stringc, f);

    s = fdja_str(v);

    if (fdja_is_symbol(s) && ! fdja_is_number(s))
      fputs(s, f);
    else
      fprintf(f, "\"%s\"", s);

    fputs(clearc, f);
  }
  else if (v->type == 'a' || v->type == 'o')
  {
    if (ol)
    {
      fputs(infrac, f);
      if (v->type == 'a') fputc('[', f); else fputc('{', f);
      fputs(clearc, f);
      for (fdja_value *c = v->child; c != NULL; c = c->sibling)
      {
        if ( ! cp) fputc(' ', f);
        fdja_to_d(f, c, flags, depth + 1);
        if (c->sibling) fputc(',', f);
      }
      if ( ! cp && v->child) fputc(' ', f);
      fputs(infrac, f);
      if (v->type == 'a') fputc(']', f); else fputc('}', f);
      fputs(clearc, f);
    }
    else
    {
      char *d = fdja_to_djan(v, FDJA_F_ONELINE); size_t dl = strlen(d);

      if (dl < 3 || depth * 2 + dl < 80)
      {
        char *dd = fdja_to_djan(v, FDJA_F_ONELINE | (flags & FDJA_F_COLOR));
        fputs(dd, f);
        free(dd);
      }
      else
      {
        fputs(infrac, f);
        if (v->type == 'a') fputs("[\n", f); else fputs("{\n", f);
        fputs(clearc, f);
        for (fdja_value *c = v->child; c != NULL; c = c->sibling)
        {
          fdja_to_d(f, c, flags, depth + 1); fputc('\n', f);
        }
        fputs(infrac, f);
        fputs(indent, f); if (v->type == 'a') fputc(']', f); else fputc('}', f);
        fputs(clearc, f);
      }
      free(d);
    }
  }
  else
  {
    if (v->type == 't') fputs(truec, f);
    else if (v->type == 'f') fputs(falsec, f);

    fwrite(v->source + v->soff, sizeof(char), v->slen, f);

    fputs(clearc, f);
  }

  free(indent);
  if (s) free(s);
}

char *fdja_to_djan(fdja_value *v, int flags)
{
  if (v == NULL) return NULL;

  flu_sbuffer *b = flu_sbuffer_malloc();
  fdja_to_d(b->stream, v, flags, 0);

  return flu_sbuffer_to_string(b);
}

char *fdja_f_todc(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *p = flu_svprintf(path, ap);
  va_end(ap);

  fdja_value *v = fdja_parse_f(p);
  if (v == NULL) v = fdja_parse_obj_f(p);

  free(p);

  if (v == NULL) return NULL;

  char *r = fdja_todc(v);

  fdja_free(v);

  return r;
}


//
// extracting stuff out of fdja_value items

char *fdja_string(fdja_value *v)
{
  return strndup(v->source + v->soff, v->slen);
}

char *fdja_str(fdja_value *v)
{
  if (v->type == 's' || v->type == 'q')
    return strndup(v->source + v->soff + 1, v->slen - 2);

  return fdja_string(v);
}

char *fdja_to_string(fdja_value *v)
{
  if (v->type == 's')
    return flu_n_unescape(v->source + v->soff + 1, v->slen - 2);

  if (v->type == 'q')
    return fdja_sq_unescape(v->source + v->soff + 1, v->slen - 2);

  return fdja_string(v);
}

long long fdja_to_int(fdja_value *v)
{
  if (v->type == 't') return 1;
  if (v->type == 'f') return 0;
  if (v->type != 'n') return -1;
  return atoll(v->source + v->soff);
}

long double fdja_to_double(fdja_value *v)
{
  if (v->type != 'n') return 0.0;
  return atof(v->source + v->soff);
}

size_t fdja_size(fdja_value *v)
{
  size_t i = 0; for (fdja_value *c = v->child; ; c = c->sibling)
  {
    if (c == NULL || i == SIZE_MAX) break;
    i++;
  }
  return i;
}

fdja_value *fdja_value_at(fdja_value *v, long n)
{
  if (n < 0)
  {
    n = fdja_size(v) + n;
    if (n < 0) return NULL;
  }

  size_t i = 0; for (fdja_value *c = v->child; c != NULL; c = c->sibling)
  {
    if (i++ == n) return c;
  }

  return NULL;
}

fdja_value *fdja_vlookup(fdja_value *v, const char *path, va_list ap)
{
  if (fdja_path_parser == NULL) fdja_path_parser_init();

  char *p = flu_svprintf(path, ap);

  //printf("p >%s<\n", p);
  //fabr_tree *tt = fabr_parse_f(p, 0, fdja_path_parser, 0);
  //puts(fabr_tree_to_string(tt, p, 1));

  fabr_tree *t = fabr_parse_all(p, 0, fdja_path_parser);

  if (t->result != 1) { fabr_tree_free(t); free(p); return NULL; }

  //puts(fabr_tree_to_string(t, p, 1));

  fdja_value *vv = v;

  flu_list *l = fabr_tree_list_named(t, "node");

  for (flu_node *n = l->first; n; n = n->next)
  {
    fabr_tree *tt = ((fabr_tree *)n->item)->child;
    char ltype = tt->name[0];

    //puts(fabr_tree_to_string(tt, p, 1));

    if (ltype == 'i' && vv->type == 'o') { ltype = 'k'; }
    else if (ltype == 'i' && vv->type != 'a') { vv = NULL; break; }
    else if (ltype == 'k' && vv->type != 'o') { vv = NULL; break; }

    char *s = fabr_tree_string(p, tt);

    if (ltype == 'i')
    {
      vv = fdja_value_at(vv, atol(s));
    }
    else // if (ltype == 'k')
    {
      for (vv = vv->child; vv; vv = vv->sibling)
      {
        //printf("vv->key: %s, s: %s\n", vv->key, s);
        if (strcmp(vv->key, s) == 0) break;
      }
    }

    free(s);

    if (vv == NULL) break;
  }

  flu_list_free(l);
  fabr_tree_free(t);
  free(p);

  return vv;
}

fdja_value *fdja_lookup(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  va_end(ap);

  return r;
}

fdja_value *fdja_lookup_c(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  va_end(ap);

  return r ? fdja_clone(r) : NULL;
}

char *fdja_lookup_string(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  char *def = va_arg(ap, char *);
  va_end(ap);

  return r ? fdja_to_string(r) : def;
}

char *fdja_lookup_string_dup_default(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  char *def = va_arg(ap, char *);
  va_end(ap);

  if (r) return fdja_to_string(r);
  return def == NULL ? NULL : strdup(def);
}

long long fdja_lookup_int(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  long long def = va_arg(ap, long long);
  va_end(ap);

  return r ? fdja_to_int(r) : def;
}

int fdja_lookup_boolean(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  int def = va_arg(ap, int);
  va_end(ap);

  if (r == NULL) return def;
  if (r->type == 't') return 1;
  if (r->type == 'f') return 0;
  return def;
}

int fdja_lookup_bool(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  int def = va_arg(ap, int);
  va_end(ap);

  if (r == NULL) return def;
  char *s = r->source + r->soff;
  if (strncasecmp(s, "true", r->slen) == 0) return 1;
  if (strncasecmp(s, "yes", r->slen) == 0) return 1;
  if (strncasecmp(s, "false", r->slen) == 0) return 0;
  if (strncasecmp(s, "no", r->slen) == 0) return 0;
  return def;
}

char *fdja_lj(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  va_end(ap);

  return r ? fdja_to_json(r) : NULL;
}

char *fdja_ld(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  va_end(ap);

  return r ? fdja_to_djan(r, FDJA_F_ONELINE) : NULL;
}

fdja_value *fdja_push(fdja_value *array, fdja_value *v)
{
  if (array->type != 'a') return NULL;

  for (fdja_value **s = &array->child; ; s = &(*s)->sibling)
  {
    if (*s == NULL) { *s = v; break; }
  }
  return v;
}

fdja_value *fdja_set(fdja_value *object, const char *key, fdja_value *val)
{
  if (object->type != 'o') return NULL;

  fdja_value *v = val;

  char *k = (char *)key;
  short start = (val != NULL && *k == '\b');

  if (*k == '\b') k = k + 1;
  if (start) v = NULL;

  if (val != NULL)
  {
    if (val->key) free(val->key);
    val->key = strdup(k);
  }

  for (fdja_value **link = &object->child; ; link = &(*link)->sibling)
  {
    fdja_value *child = *link;

    if (child == NULL) { *link = v; break; }

    if (strcmp(k, child->key) == 0)
    {
      if (v == NULL)
      {
        *link = child->sibling;
        fdja_value_free(child);
      }
      else
      {
        *link = v;
        v->sibling = child->sibling;
        fdja_value_free(child);
      }
      break;
    }
  }
  if (start)
  {
    val->sibling = object->child;
    object->child = val;
  }

  return val;
}

int fdja_merge(fdja_value *dst, fdja_value *src)
{
  if (dst->type != 'o' || src->type != 'o') return 0;

  fdja_value *last = dst->child;
  while (last && last->sibling) { last = last->sibling; }

  for (fdja_value *c = src->child; c != NULL; c = c->sibling)
  {
    fdja_value *cc = fdja_clone(c);
    cc->key = strdup(c->key);

    if (last) last->sibling = cc;
    else dst->child = cc;

    last = cc;
  }

  return 1;
}

int fdja_splice(fdja_value *array, long long start, size_t count, ...)
{
  if (array->type != 'a') return 0;

  if (start < 0)
  {
    size_t len = fdja_size(array);
    start = len + start;
    if (start < 0) return 0;
  }

  // determine start and end of removal

  size_t off = 0;
  fdja_value **link = &array->child;

  fdja_value **lstart = NULL;
  fdja_value *end = NULL;

  while (1)
  {
    fdja_value *c = *link;

    if (c == NULL && lstart != NULL) break;

    if (lstart == NULL && off == start) { lstart = link; }
    if (lstart != NULL && count == 0) { end = *link; break; }

    if (c == NULL) return 0;

    ++off;
    link = &(*link)->sibling;
    if (lstart) --count;
  }

  // free old elements

  for (fdja_value *c = *lstart, *n = NULL; c != NULL && c != end; )
  {
    n = c->sibling;
    fdja_value_free(c);
    c = n;
  }

  // insert new elements

  va_list ap; va_start(ap, count);
  while (1)
  {
    fdja_value *v = va_arg(ap, fdja_value *);
    if (v == NULL) break;
    *lstart = v;
    lstart = &v->sibling;
  }
  va_end(ap);

  // link back with end of list

  *lstart = end;

  return 1;
}

static int is_a_number(char *s)
{
  if (*s == '-') s = s + 1;
  for (size_t i = 0; s[i]; ++i) if ( ! isdigit(s[i])) return 0;
  return 1;
}

fdja_value *fdja_pset(fdja_value *start, const char *path, ...)
{
  fdja_value *r = NULL;

  va_list ap; va_start(ap, path);
  char *p = flu_svprintf(path, ap);
  fdja_value *v = va_arg(ap, fdja_value *);
  va_end(ap);

  char *lastdot = strrchr(p, '.');

  if (lastdot == NULL)
  {
    r = fdja_set(start, p, v);
    goto _over;
  }

  char *key = lastdot + 1;
  char *pat = strndup(p, lastdot - p);

  fdja_value *target = fdja_lookup(start, pat);
  free(pat);

  if (target == NULL) goto _over;

  if (target->type == 'o')
  {
    r = fdja_set(target, key, v);
    goto _over;
  }

  if (target->type == 'a')
  {
    if (strcmp(key, "]") == 0)
    {
      r = fdja_push(target, v);
    }
    else if (is_a_number(key))
    {
      fdja_splice(target, strtol(key, NULL, 10), 1, v, NULL);
      r = v;
    }
  }

_over:

  free(p); // key depends on it...

  return r;
}

fdja_value *fdja_psetf(fdja_value *start, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *p = flu_svprintf(path, ap);
  char *f = va_arg(ap, char *);
  char *s = flu_svprintf(f, ap);
  va_end(ap);

  fdja_value *r = NULL;

  fdja_value *v = fdja_parse(s);
  if (v == NULL) goto _over;

  r = fdja_pset(start, p, v);
  if (r == NULL) fdja_free(v);

_over:

  free(p);

  return r;
}

