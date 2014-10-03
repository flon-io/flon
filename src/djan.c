
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

#include "djan.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>

#include "aabro.h"
#include "flutil.h"


//
// fdja_value malloc/free

static fdja_value *fdja_value_malloc(
  char type, char *input, size_t off, size_t len)
{
  fdja_value *v = calloc(1, sizeof(fdja_value));
  v->key = NULL;
  v->type = type;
  v->source = input;
  v->soff = off;
  v->slen = len;
  v->sibling = NULL;
  v->child = NULL;

  return v;
}

void fdja_value_free(fdja_value *v)
{
  if (v->key != NULL) free(v->key);
  if (v->slen == 0 && v->source != NULL) free(v->source);

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
      "\"("
        //"\\\\." "|"
        "\\\\[\"\\/\\\\bfnrt]" "|"
        "\\\\u[0-9a-fA-F]{4}" "|"
        "[^\"\\\\]"
      ")*\"");
  fabr_parser *sqstring =
    fabr_n_rex("sqstring", "'(\\\\'|[^'])*'");

  fabr_parser *symbol =
    fabr_n_rex("symbol", "[a-zA-Z_][a-zA-Z_0-9]*");

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
    fabr_alt(
      string,
      sqstring,
      fabr_n_rex("number", "-?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?"),
      object,
      array,
      fabr_n_string("true", "true"),
      fabr_n_string("false", "false"),
      fabr_n_string("null", "null"),
      symbol,
      NULL);

  fdja_parser =
    fabr_n_seq("value", blanks, pure_value, blanks, NULL);

  // symbol

  fdja_symbol_parser = symbol;

  // radial

  fabr_parser *spaces = fabr_rex("[ \t]*");

  fabr_parser *rad_i = fabr_name("rad_i", spaces);
  fabr_parser *rad_n = fabr_name("rad_n", symbol);

  fabr_parser *rad_a =
    fabr_n_seq("rad_a", spaces, pure_value, fabr_n_r("", "?"));

  fabr_parser *rad_e =
    fabr_n_seq(
      "rad_e",
      fabr_seq(spaces, fabr_rex(",?"), blanks, NULL),
      fabr_n_alt("key", string, sqstring, symbol, NULL),
      spaces,
      fabr_string(":"),
      fabr_n_seq("val", blanks, pure_value, NULL),
      NULL);

  fabr_parser *rad_as =
    fabr_n_rep("rad_as", rad_e, 0, -1);

  fabr_parser *rad_eol =
    fabr_rex("[ \t]*(#[^\n\r]*)?");

  fabr_parser *rad_l =
    fabr_n_seq("rad_l", rad_i, rad_n, rad_a, rad_as, NULL);

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
  fabr_parser *key = fabr_n_rex("key", "[a-zA-Z_][a-zA-Z_0-9]*");
  fabr_parser *node = fabr_n_alt("node", index, key, NULL);

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
  char *r = calloc(n + 1, sizeof(char));
  for (size_t i = 0, j = 0; i < n; i++)
  {
    char c = s[i];
    if (c == '\0') break;
    if (c != '\\') r[j++] = c;
  }
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
    return fdja_sq_unescape(input + c->offset + 1, c->length - 2);

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
  //printf("%s\n", fabr_tree_to_string(t));

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
  //printf("fdja_extract_v() %s\n", fabr_tree_to_string(t, input));
  //printf("fdja_extract_v() %s\n", fabr_tree_to_str(t, input));

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

  fdja_value *v = fdja_value_malloc(ty, input, t->offset, t->length);

  if (ty == 'o') v->child = fdja_extract_entries(input, fabr_t_child(t, 1));
  else if (ty == 'a') v->child = fdja_extract_values(input, fabr_t_child(t, 1));

  return v;
}

static fdja_value *fdja_extract_value(char *input, fabr_tree *t)
{
  //printf("fdja_extract_value() %s\n", fabr_tree_to_string(t, input));
  //printf("fdja_extract_value() %s\n", fabr_tree_to_str(t, input));

  if (t->result != 1) return NULL;

  t = fabr_t_child(t, 1);

  //printf("fdja_extract_value() child1 %s\n", fabr_tree_to_str(t, input));

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    if (c->result == 1) return fdja_extract_v(input, c);
  }

  return NULL;
}

fdja_value *fdja_parse(char *input)
{
  if (fdja_parser == NULL) fdja_parser_init();

  fabr_tree *t = fabr_parse_all(input, 0, fdja_parser);

  //printf(">%s<\n", input);
  //puts(fabr_parser_to_string(t->parser));
  //puts(fabr_tree_to_string(t, input));

  fdja_value *v = fdja_extract_value(input, t);
  fabr_tree_free(t);

  return v;
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
  fabr_tree *rada = fabr_tree_lookup(radl, "rad_a");
  fabr_tree *radas = fabr_tree_lookup(radl, "rad_as");

  size_t i = radi->length; // indentation

  // [ "sequence", {}, [] ]
  fdja_value *v = fdja_value_malloc('a', NULL, i, 0);
  fdja_value *vname = fdja_extract_v(input, radn->child);
  fdja_value *vatts = fdja_value_malloc('o', NULL, 0, 0);
  fdja_value *vchildren = fdja_value_malloc('a', NULL, 0, 0);

  // single "_a" attribute
  if (rada != NULL)
  {
    fdja_value *va = fdja_extract_value(input, rada);
    if (va != NULL) va->key = strdup("_a");
    vatts->child = va;
  }

  // attributes
  fdja_value **anext = &vatts->child;
  if (vatts->child != NULL) anext = &vatts->child->sibling;
  flu_list *as = fabr_tree_list_named(radas, "rad_e");
  for (flu_node *n = as->first; n != NULL; n = n->next)
  {
    fabr_tree *ak = fabr_t_child(n->item, 1);
    fabr_tree *av = fabr_t_child(n->item, 4);
    fdja_value *va = fdja_extract_value(input, av);
    va->key = fabr_tree_string(input, ak);
    *anext = va;
    anext = &va->sibling;
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

  return root;
}

fdja_value *fdja_parse_obj(char *input)
{
  if (fdja_parser == NULL) fdja_parser_init();

  fabr_tree *t = fabr_parse_all(input, 0, fdja_obj_parser);

  if (t->result != 1) { fabr_tree_free(t); return NULL; }

  //printf(">%s<\n", input);
  //puts(fabr_parser_to_string(t->parser));
  //puts(fabr_tree_to_string(t, input));

  fabr_tree *tt = fabr_t_child(t, 1);

  fdja_value *v = fdja_extract_v(input, tt);

  fabr_tree_free(t);

  return v;
}

fdja_value *fdja_parse_obj_f(const char *path)
{
  char *s = flu_readall(path);

  if (s == NULL) return NULL;

  fdja_value *v = fdja_parse_obj(s);
  v->slen = 0; // so that the root value "owns" the source

  return v;
}


//
// outputting

static void fdja_to_j(FILE *f, fdja_value *v)
{
  if (v->key != NULL)
  {
    fprintf(f, "\"%s\":", v->key);
  }

  if (v->type == 'q')
  {
    char *s = fdja_to_string(v);
    fprintf(f, "\"%s\"", s);
    free(s);
  }
  else if (v->type == 'y')
  {
    char *s = fdja_string(v);
    fprintf(f, "\"%s\"", s);
    free(s);
  }
  else if (v->type == 'a')
  {
    fputc('[', f);
    for (fdja_value *c = v->child; c != NULL; c = c->sibling)
    {
      fdja_to_j(f, c);
      if (c->sibling != NULL) fputc(',', f);
    }
    fputc(']', f);
  }
  else if (v->type == 'o')
  {
    fputc('{', f);
    for (fdja_value *c = v->child; c != NULL; c = c->sibling)
    {
      fdja_to_j(f, c);
      if (c->sibling != NULL) fputc(',', f);
    }
    fputc('}', f);
  }
  else if (v->slen == 0) fputs(v->source + v->soff, f);
  else fwrite(v->source + v->soff, sizeof(char), v->slen, f);
}

char *fdja_to_json(fdja_value *v)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fdja_to_j(b->stream, v);

  return flu_sbuffer_to_string(b);
}

static int fdja_is_symbol(char *s)
{
  if (fdja_parser == NULL) fdja_parser_init();

  return fabr_match(s, fdja_symbol_parser);
}

static void fdja_s_to_d(FILE *f, char *s, int do_free)
{
  if (fdja_is_symbol(s)) fputs(s, f); else fprintf(f, "\"%s\"", s);
  if (do_free) free(s);
}

static void fdja_to_d(FILE *f, fdja_value *v)
{
  if (v->key) { fdja_s_to_d(f, v->key, 0); fputs(": ", f); }

  if (v->type == 'q' || v->type == 's')
  {
    fdja_s_to_d(f, fdja_to_string(v), 1);
  }
  else if (v->type == 'y')
  {
    char *s = fdja_string(v); fputs(s, f); free(s);
  }
  else if (v->type == 'a')
  {
    fputc('[', f);
    for (fdja_value *c = v->child; c != NULL; c = c->sibling)
    {
      fputc(' ', f); fdja_to_d(f, c);
      if (c->sibling) fputc(',', f);
    }
    if (v->child) fputc(' ', f);
    fputc(']', f);
  }
  else if (v->type == 'o')
  {
    fputc('{', f);
    for (fdja_value *c = v->child; c != NULL; c = c->sibling)
    {
      fputc(' ', f); fdja_to_d(f, c);
      if (c->sibling != NULL) fputc(',', f);
    }
    if (v->child) fputc(' ', f);
    fputc('}', f);
  }
  else if (v->slen == 0) fputs(v->source + v->soff, f);
  else fwrite(v->source + v->soff, sizeof(char), v->slen, f);
}

char *fdja_to_djan(fdja_value *v)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fdja_to_d(b->stream, v);

  return flu_sbuffer_to_string(b);
}


//
// extracting stuff out of fdja_value items

char *fdja_string(fdja_value *v)
{
  if (v->slen == 0) return strdup(v->source + v->soff);
  return strndup(v->source + v->soff, v->slen);
}

char *fdja_to_string(fdja_value *v)
{
  if (v->type == 's')
  {
    return flu_n_unescape(v->source + v->soff + 1, v->slen - 2);
  }
  if (v->type == 'q')
  {
    return fdja_sq_unescape(v->source + v->soff + 1, v->slen - 2);
  }
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

fdja_value *fdja_lookup(fdja_value *v, const char *path)
{
  if (fdja_path_parser == NULL) fdja_path_parser_init();

  fabr_tree *t = fabr_parse_all(path, 0, fdja_path_parser);

  if (t->result != 1) { fabr_tree_free(t); return NULL; }

  //printf("path >%s<\n", path);
  //puts(fabr_tree_to_string(t, path));

  fdja_value *vv = v;

  flu_list *l = fabr_tree_list_named(t, "node");

  for (flu_node *n = l->first; n; n = n->next)
  {
    fabr_tree *tt = ((fabr_tree *)n->item)->child;
    //puts(fabr_tree_to_string(tt, path));
    char ltype = tt->name[0];

    if (ltype == 'i' && vv->type != 'a') { vv = NULL; break; }
    if (ltype == 'k' && vv->type != 'o') { vv = NULL; break; }

    char *s = fabr_tree_string(path, tt);

    if (ltype == 'i')
    {
      vv = fdja_value_at(vv, atol(s));
    }
    else // if (ltype == 'k')
    {
      for (vv = vv->child; vv; vv = vv->sibling)
      {
        if (strcmp(vv->key, s) == 0) break;
      }
    }

    free(s);

    if (vv == NULL) break;
  }

  flu_list_free(l);
  fabr_tree_free(t);

  return vv;
}

char *fdja_lookup_string(fdja_value *v, const char *path, char *def)
{
  fdja_value *vv = fdja_lookup(v, path);

  return vv ? fdja_to_string(vv) : def;
}

long long fdja_lookup_int(fdja_value *v, const char *path, long long def)
{
  fdja_value *vv = fdja_lookup(v, path);

  return vv ? fdja_to_int(vv) : def;
}

int fdja_lookup_boolean(fdja_value *v, const char *path, int def)
{
  fdja_value *vv = fdja_lookup(v, path);

  if (vv == NULL) return def;
  if (vv->type == 't') return 1;
  if (vv->type == 'f') return 0;
  return def;
}

int fdja_lookup_bool(fdja_value *v, const char *path, int def)
{
  fdja_value *vv = fdja_lookup(v, path);

  if (vv == NULL) return def;

  char *s = vv->source + vv->soff;
  if (strncasecmp(s, "true", vv->slen) == 0) return 1;
  if (strncasecmp(s, "yes", vv->slen) == 0) return 1;
  if (strncasecmp(s, "false", vv->slen) == 0) return 0;
  if (strncasecmp(s, "no", vv->slen) == 0) return 0;
  return def;
}

int fdja_push(fdja_value *array, fdja_value *v)
{
  if (array->type != 'a') return 0;

  for (fdja_value **s = &array->child; ; s = &(*s)->sibling)
  {
    if (*s == NULL) { *s = v; break; }
  }
  return 1;
}

int fdja_set(fdja_value *object, const char *key, fdja_value *v)
{
  if (object->type != 'o') return 0;

  if (v != NULL)
  {
    if (v->key) free(v->key);
    v->key = strdup(key);
  }

  for (fdja_value **link = &object->child; ; link = &(*link)->sibling)
  {
    fdja_value *child = *link;

    if (child == NULL) { *link = v; break; }

    if (strcmp(key, child->key) == 0)
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

  return 1;
}

int fdja_splice(fdja_value *array, size_t start, size_t count, ...)
{
  if (array->type != 'a') return 0;

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

