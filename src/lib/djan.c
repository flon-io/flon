
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

// https://github.com/flon-io/djan

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
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

fdja_value *fdja_value_malloc(
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
  if (v == NULL) return; // like free()

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

fdja_value *fdja_array_malloc()
{
  return fdja_value_malloc('a', NULL, 0, 0, 0);
}

fdja_value *fdja_object_malloc()
{
  return fdja_value_malloc('o', NULL, 0, 0, 0);
}


//
// parsing

// DJAN PARSERS

static fabr_tree *_ws(fabr_input *i) { return fabr_rng(NULL, i, " \t"); }
static fabr_tree *_rn(fabr_input *i) { return fabr_rng(NULL, i, "\r\n"); }
static fabr_tree *_comma(fabr_input *i) { return fabr_str(NULL, i, ","); }

static fabr_tree *_shacom(fabr_input *i)
{
  return fabr_rex(NULL, i, "#[^\r\n]*");
}
static fabr_tree *_slacom(fabr_input *i)
{
  return fabr_rex(NULL, i, "//[^\r\n]*");
}
static fabr_tree *_com(fabr_input *i)
{
  return fabr_alt(NULL, i, _shacom, _slacom, NULL);
}
static fabr_tree *_eol(fabr_input *i)
{
  return fabr_seq(NULL, i,
    _ws, fabr_star, _com, fabr_qmark, _rn, fabr_star,
    NULL);
}

static fabr_tree *_postval(fabr_input *i)
{
  return fabr_seq(NULL, i, _eol, fabr_star, NULL);
}

static fabr_tree *_sep(fabr_input *i)
{
  return fabr_seq(NULL, i, _comma, fabr_qmark, _postval, NULL);
}

static fabr_tree *_value(fabr_input *i); // forward

static fabr_tree *_string(fabr_input *i)
{
  return fabr_rex("string", i,
    "\""
      "("
        "\\\\[\"\\/\\\\bfnrt]" "|"
        "\\\\u[0-9a-fA-F]{4}" "|"
        "[^"
          "\"" "\\\\" /*"\\/"*/ "\b" "\f" "\n" "\r" "\t"
        "]"
      ")*"
    "\"");
}
static fabr_tree *_sqstring(fabr_input *i)
{
  return fabr_rex("sqstring", i,
    "'"
      "("
        "\\\\['\\/\\\\bfnrt]" "|"
        "\\\\u[0-9a-fA-F]{4}" "|"
        "[^"
          "'" "\\\\" /*"\\/"*/ "\b" "\f" "\n" "\r" "\t"
        "]"
      ")*"
    "'");
}
static fabr_tree *_rxstring(fabr_input *i)
{
  return fabr_rex("rxstring", i,
    "/"
      "("
        "\\\\['\\/\\\\bfnrt]" "|"
        "\\\\u[0-9a-fA-F]{4}" "|"
        "[^"
          "/" "\\\\" "\b" "\f" "\n" "\r" "\t"
        "]"
      ")*"
    "/i?");
}

static fabr_tree *_colon(fabr_input *i) { return fabr_str(NULL, i, ":"); }
static fabr_tree *_dolstart(fabr_input *i) { return fabr_str(NULL, i, "$("); }
static fabr_tree *_pstart(fabr_input *i) { return fabr_str(NULL, i, "("); }
static fabr_tree *_pend(fabr_input *i) { return fabr_str(NULL, i, ")"); }

static fabr_tree *_symcore(fabr_input *i)
{
  return fabr_rex(NULL, i, "[^: \b\f\n\r\t\"',\\(\\)\\[\\]\\{\\}#\\\\]+");
}

static fabr_tree *_dol(fabr_input *i)
{
  return fabr_rex(NULL, i, "[^ \r\n\t\\)]+");
}

static fabr_tree *_symdol(fabr_input *i)
{
  return fabr_seq(NULL, i, _dolstart, _dol, _pend, NULL);
}

static fabr_tree *_symeltk(fabr_input *i)
{
  return fabr_alt(NULL, i, _symdol, _symcore, NULL);
}
static fabr_tree *_symelt(fabr_input *i)
{
  return fabr_alt(NULL, i, _symdol, _symcore, _colon, NULL);
}

static fabr_tree *_symbolk(fabr_input *i)
{
  return fabr_rep("symbolk", i, _symeltk, 1, 0);
}
static fabr_tree *_symbol(fabr_input *i)
{
  return fabr_rep("symbol", i, _symelt, 1, 0);
}

static fabr_tree *_number(fabr_input *i)
{
  return fabr_rex("number", i, "-?[0-9]+(\\.[0-9]+)?([eE][+-]?[0-9]+)?");
}

static fabr_tree *_val(fabr_input *i)
{
  return fabr_seq(NULL, i, _value, _postval, NULL);
}
static fabr_tree *_val_qmark(fabr_input *i)
{
  return fabr_rep(NULL, i, _val, 0, 1);
}

static fabr_tree *_key(fabr_input *i)
{
  return fabr_alt("key", i, _string, _sqstring, _symbolk, NULL);
}

static fabr_tree *_entry(fabr_input *i)
{
  return fabr_seq("entry", i,
    _key, _postval, _colon, _postval, _value, _postval,
    NULL);
}
static fabr_tree *_entry_qmark(fabr_input *i)
{
  return fabr_rep(NULL, i, _entry, 0, 1);
}

static fabr_tree *_pbstart(fabr_input *i) { return fabr_str(NULL, i, "{"); }
static fabr_tree *_pbend(fabr_input *i) { return fabr_rex(NULL, i, "}"); }

static fabr_tree *_object(fabr_input *i)
{
  return fabr_eseq("object", i, _pbstart, _entry_qmark, _sep, _pbend);
}
static fabr_tree *_bjec(fabr_input *i)
{
  return fabr_jseq("object", i, _entry_qmark, _sep);
}
static fabr_tree *_ob(fabr_input *i)
{
  return fabr_alt(NULL, i, _object, _bjec, NULL);
}
static fabr_tree *_obj(fabr_input *i)
{
  return fabr_seq(NULL, i, _postval, _ob, _postval, NULL);
}

static fabr_tree *_sbstart(fabr_input *i) { return fabr_str(NULL, i, "["); }
static fabr_tree *_sbend(fabr_input *i) { return fabr_str(NULL, i, "]"); }

static fabr_tree *_array(fabr_input *i)
{
  return fabr_eseq("array", i, _sbstart, _val_qmark, _sep, _sbend);
}

static fabr_tree *_true(fabr_input *i) { return fabr_str("true", i, "true"); }
static fabr_tree *_false(fabr_input *i) { return fabr_str("false", i, "false"); }
static fabr_tree *_null(fabr_input *i) { return fabr_str("null", i, "null"); }

static fabr_tree *_v(fabr_input *i)
{
  return fabr_alt(NULL, i,
    _string, _sqstring, _number, _object, _array, _true, _false, _null,
    NULL);
}

static fabr_tree *_value(fabr_input *i)
{
  return fabr_altg(NULL, i, _symbol, _v, NULL);
}

static fabr_tree *_djan(fabr_input *i)
{
  return fabr_seq(NULL, i, _postval, _val, NULL);
}

// radial

static fabr_tree *_rad_g(fabr_input *i); // forward

static fabr_tree *_rad_p(fabr_input *i)
{
  return fabr_seq("rad_p", i,
    _pstart, _eol, _ws, fabr_star, _rad_g, _eol, _pend,
    NULL);
}

static fabr_tree *_rad_v(fabr_input *i)
{
  return fabr_alt("rad_v", i, _rxstring, _rad_p, _value, NULL);
}

static fabr_tree *_rad_k(fabr_input *i)
{
  return fabr_alt("rad_k", i, _string, _sqstring, _symbolk, NULL);
}

static fabr_tree *_rad_kcol(fabr_input *i)
{
  return fabr_seq(NULL, i,
    _rad_k, _ws, fabr_star, _colon, _eol, _ws, fabr_star,
    NULL);
}

static fabr_tree *_rad_e(fabr_input *i)
{
  return fabr_seq("rad_e", i,
    //_rad_k, _ws, fabr_star, _colon, _eol, _rad_v,
    _rad_kcol, fabr_qmark, _rad_v,
    NULL);
}

static fabr_tree *_rad_com(fabr_input *i)
{
  return fabr_seq(NULL, i, _comma, _eol, NULL);
}
static fabr_tree *_rad_comma(fabr_input *i)
{
  return fabr_seq(NULL, i,
    _ws, fabr_star, _rad_com, fabr_qmark, _ws, fabr_star,
    NULL);
}

static fabr_tree *_rad_ce(fabr_input *i)
{
  return fabr_seq(NULL, i, _rad_comma, _rad_e, NULL);
}

static fabr_tree *_rad_h(fabr_input *i)
{
  return fabr_seq("rad_h", i, _rad_v, NULL);
}

static fabr_tree *_rad_es(fabr_input *i)
{
  return fabr_rep(NULL, i, _rad_ce, 0, 0);
}

static fabr_tree *_rad_g(fabr_input *i)
{
  return fabr_seq("rad_g", i, _rad_h, _rad_es, NULL);
}

static fabr_tree *_rad_i(fabr_input *i)
{
  return fabr_rex("rad_i", i, "[ \t]*");
}

static fabr_tree *_rad_l(fabr_input *i)
{
  return fabr_seq("rad_l", i, _rad_i, _rad_g, NULL);
}

static fabr_tree *_rad_eol(fabr_input *i)
{
  return fabr_rex(NULL, i, "[ \t]*(#[^\n\r]*)?[\n\r]?");
}

static fabr_tree *_rad_line(fabr_input *i)
{
  return fabr_seq(NULL, i, _rad_l, fabr_qmark, _rad_eol, NULL);
}

static fabr_tree *_radial(fabr_input *i)
{
  return fabr_rep(NULL, i, _rad_line, 0, 0);
}

// path parser

static fabr_tree *_pa_key(fabr_input *i)
{
  return fabr_rex("key", i, "(\\\\.|[^\n\r\t\\.])+");
}
static fabr_tree *_pa_index(fabr_input *i)
{
  return fabr_rex("index", i, "-?[0-9]+");
}
static fabr_tree *_pa_node(fabr_input *i)
{
  return fabr_altg("node", i, _pa_key, _pa_index, NULL);
}
static fabr_tree *_pa_dot(fabr_input *i)
{
  return fabr_str(NULL, i, ".");
}

static fabr_tree *_path(fabr_input *i)
{
  return fabr_jseq(NULL, i, _pa_node, _pa_dot);
}

// forward declarations
static fdja_value *fdja_extract_value(char *input, fabr_tree *t);
static fdja_value *fdja_extract_v(char *input, fabr_tree *t);

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
  //printf("dek() "); fabr_puts(t, input, 3);

  //while (t->result != 1) t = t->sibling; // unpruned trees are ok too

  if (strcmp(t->name, "string") == 0)
    return flu_n_unescape(input + t->offset + 1, t->length - 2);

  if (strcmp(t->name, "sqstring") == 0)
    return flu_n_unescape(input + t->offset + 1, t->length - 2);

  //if (strcmp(t->name, "symbol") == 0)
  return strndup(input + t->offset, t->length);
}

static fdja_value *fdja_extract_entries(char *input, fabr_tree *t)
{
  //printf("dees() "); fabr_puts(t, input, 3);

  flu_list *ts = fabr_tree_list_named(t, "entry");

  fdja_value *first = NULL;
  fdja_value *child = NULL;

  for (flu_node *n = ts->first; n != NULL; n = n->next)
  {
    fabr_tree *tt = (fabr_tree *)n->item;

    //printf("dees() ent "); fabr_puts(tt, input, 3);

    fdja_value *v =
      fdja_extract_value(input, tt->child->sibling->sibling->sibling->sibling);
    v->key =
      fdja_extract_key(input, tt->child->child);

    //printf("dees() ent key >%s<\n", v->key);

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

  //flu_list *ts = fabr_tree_list_named(t, "value");
  flu_list *ts = fabr_tree_list_named_cn(t, NULL);

  fdja_value *first = NULL;
  fdja_value *child = NULL;

  for (flu_node *n = ts->first; n != NULL; n = n->next)
  {
    //printf("** "); fabr_puts_tree(n->item, input, 1);
    fdja_value *v = fdja_extract_v(input, n->item);
    if (first == NULL) first = v;
    if (child != NULL) child->sibling = v;
    child = v;
  }

  flu_list_free(ts);

  return first;
}

static fdja_value *fdja_extract_v(char *input, fabr_tree *t)
{
  //printf("de_v() t %p\n", t);
  if (t == NULL) return NULL;

  //printf("de_v() "); fabr_puts(t, input, 3);
  //printf("de_v() t->name >%s<\n", t->name);

  char ty = '-';

  if (strchr("tfao", *t->name)) ty = *t->name;
  else if (*(t->name + 1) == 't') ty = 's'; // string
  else if (*(t->name + 1) == 'q') ty = 'q'; // sqstring ('string')
  else if (*(t->name + 1) == 'x') ty = 'y'; // rxstring (/string/)
  else if (*(t->name + 1) == 'y') ty = 'y'; // symbol
  else if (*(t->name + 2) == 'm') ty = 'n'; // number
  else if (*(t->name + 2) == 'l') ty = '0'; // null

  if (ty == '-') return NULL;

  fdja_value *v = fdja_value_malloc(ty, input, t->offset, t->length, 0);

  if (ty == 'o')
    v->child = fdja_extract_entries(input, t);
  else if (ty == 'a')
    v->child = fdja_extract_values(input, t);

  return v;
}

static fdja_value *fdja_extract_value(char *input, fabr_tree *t)
{
  //printf("fdja_extract_value() "); fabr_puts(t, input, 3);

  if (t->result != 1) return NULL;

  return fdja_extract_v(input, fabr_subtree_lookup(t, NULL));
}

fdja_value *fdja_parse(char *input)
{
  //printf("fdja_parse() >[1;33m%s[0;0m<\n", input);

  //fabr_tree *tt = fabr_parse_f(input, _djan, FABR_F_ALL);
  //printf("fdja_parse():\n"); fabr_puts_tree(tt, input, 1);
  //fabr_tree_free(tt);

  fabr_tree *t = fabr_parse_all(input, _djan);
  //printf("fdja_parse() (pruned):\n"); fabr_puts(t, input, 3);

  fdja_value *v = fdja_extract_value(input, t);
  fabr_tree_free(t);

  if (v) v->sowner = 1;

  return v;
}

fdja_value *fdja_dparse(char *input)
{
  return fdja_parse(strdup(input));
}

static fdja_value *fdja_do_parse(
  FILE *f, const char *path, va_list ap, char mode)
{
  char *p = (char *)path; if (path && ap) p = flu_svprintf(path, ap);
  char *s = f ? flu_freadall(f) : flu_readall(p);

  if (s == NULL) { free(p); return NULL; }

  fdja_value *v = NULL;
  if (mode == 'o')
    v = fdja_parse_obj(s);
  else if (mode == 'r')
    v = fdja_parse_radial(s, p);
  else
    v = fdja_parse(s);

  if (p != path) free(p);
  if (v == NULL) free(s);

  return v;
}

fdja_value *fdja_fparse(FILE *f)
{
  return fdja_do_parse(f, NULL, NULL, 'j');
}

fdja_value *fdja_parse_f(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *v = fdja_do_parse(NULL, path, ap, 'j');
  va_end(ap);

  return v;
}

fdja_value *fdja_v(const char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *s = flu_svprintf(format, ap);
  va_end(ap);

  fdja_value *r = fdja_parse(s);

  if (r == NULL) free(s);

  return r;
}

char *fdja_vj(const char *format, ...)
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

fdja_value *fdja_s(const char *format, ...)
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

fdja_value *fdja_a(fdja_value *v0, ...)
{
  fdja_value *r = fdja_array_malloc();

  //if (v0 == NULL) return r;

  va_list ap; va_start(ap, v0);
  for (fdja_value *v = v0; v; v = va_arg(ap, fdja_value *)) fdja_push(r, v);
  va_end(ap);

  return r;
}

fdja_value *fdja_o(char *k0, ...)
{
  fdja_value *r = fdja_object_malloc();

  va_list ap; va_start(ap, k0);
  for (char *kf = k0; kf; kf = va_arg(ap, char *))
  {
    char *k = flu_svprintf(kf, ap);
    fdja_value *v = va_arg(ap, fdja_value *);
    fdja_set(r, k, v);
    free(k);
  }
  va_end(ap);

  return r;
}

static void fdja_add_radc(fdja_value *parent, fdja_value *child)
{
  parent = fdja_value_at(parent, 3);

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
  fdja_value *current = NULL;
  ssize_t ci = -1;
  if (values->size > 0)
  {
    current = (fdja_value *)values->first->item;
    ci = current->ind;
  }

  if (v->ind < ci)
  {
    // go closer to the root
    flu_list_shift(values);
    fdja_stack_radl(values, v);
  }
  else if (v->ind == ci)
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

static fdja_value *parse_radg(char *input, ssize_t ind, fabr_tree *radg);
  // forward...

static fdja_value *parse_radv(char *input, fabr_tree *radv)
{
  //printf("parse_radv() "); fabr_puts(radv, input, 3);

  fabr_tree *c = fabr_tree_lookup(radv, NULL);

  //printf("parse_radv() c "); fabr_puts(c, input, 3);

  if (strcmp(c->name, "rad_p") != 0) return fdja_extract_v(input, c);
  return parse_radg(input, -1, fabr_tree_lookup(c, "rad_g"));
}

static ssize_t count_lines(char *input, size_t offset)
{
  size_t r = 0;

  for (size_t i = 0; i <= offset; ++i)
  {
    char c = input[i];
    if (c == 0) return -1;
    if (c == '\n' || c == '\r') ++r;
  }

  return r + 1;
}

static fdja_value *parse_radg(char *input, ssize_t ind, fabr_tree *radg)
{
  // rad_h rad_e*

  //printf("\nparse_radg() "); fabr_puts(radg, input, 3);

  fdja_value *h = parse_radv(input, radg->child->child->child);

  flu_list *es = fabr_tree_list_named(radg->child->sibling, "rad_e");

  fdja_value *r = fdja_value_malloc('a', NULL, 0, 0, 0);
  fdja_value *vname = NULL;
  fdja_value *vatts = fdja_value_malloc('o', NULL, 0, 0, 0);
  fdja_value *vline = fdja_v("%zu", count_lines(input, radg->offset));
  fdja_value *vchildren = fdja_value_malloc('a', NULL, 0, 0, 0);

  if ((es == NULL || es->first == NULL) && ! fdja_is_stringy(h))
  {
    // single value

    vname = fdja_s("val");

    fdja_set(vatts, "_0", h);
  }
  else
  {
    // vanilla tree node

    vname = h;

    // attributes

    size_t j = 0;
    if (es) for (flu_node *n = es->first; n; n = n->next)
    {
      fabr_tree *ak = fabr_subtree_lookup(n->item, "rad_k");
      fabr_tree *av = fabr_subtree_lookup(n->item, "rad_v");
      fdja_value *va = parse_radv(input, av->child);
      //fdja_push(vatts, va);
      char *k = ak ? fabr_tree_string(input, ak) : flu_sprintf("_%zu", j);
      fdja_set(vatts, k, va);
      free(k);
      j++;
    }
  }

  r->child = vname;
  vname->sibling = vatts;
  vatts->sibling = vline;
  vline->sibling = vchildren;

  flu_list_free(es);

  r->ind = ind;

  return r;
}

static void fdja_parse_radl(char *input, fabr_tree *radl, flu_list *values)
{
  //flu_putf(fabr_tree_to_string(radl, input, 1));

  fabr_tree *radi = fabr_tree_lookup(radl, "rad_i");
  fabr_tree *radg = fabr_tree_lookup(radl, "rad_g");

  fdja_value *v = parse_radg(input, radi->length, radg);

  fdja_stack_radl(values, v);
}

fdja_value *fdja_parse_radial(char *input, const char *origin)
{
  //printf("fdja_parse_radial() >[1;33m%s[0;0m< \"%s\"\n", input, origin);

  fabr_tree *t = fabr_parse_all(input, _radial);
  // todo: deal with errors (t->result < 0) (well, it's cared for downstream)

  //fabr_puts(t, input, 3);
    //
    // debugging input and cleaned up tree

  //fabr_tree *tt = fabr_parse_f(input, _radial, FABR_F_ALL);
  //flu_putf(fabr_tree_to_string(tt, input, 1));
  //fabr_tree_free(tt);
    //
    // debugging whole tree (no pruning)

  flu_list *ls = fabr_tree_list_named(t, "rad_l");
  flu_list *vs = flu_list_malloc();

  for (flu_node *n = ls->first; n; n = n->next)
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

  if (origin) fdja_push(root, fdja_s(origin));

  return root;
}

fdja_value *fdja_dparse_radial(char *input)
{
  return fdja_parse_radial(strdup(input), NULL);
}

fdja_value *fdja_fparse_radial(FILE *f, const char *origin)
{
  return fdja_do_parse(f, origin, NULL, 'r');
}

fdja_value *fdja_parse_radial_f(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *v = fdja_do_parse(NULL, path, ap, 'r');
  va_end(ap);

  return v;
}

static void renumber(fdja_value *t, size_t lnumber)
{
  if (t == NULL) return;

  fdja_replace(fdja_at(t, 2), fdja_v("%zu", lnumber));

  for (fdja_value *c = fdja_at(t, 3)->child; c; c = c->sibling)
  {
    renumber(c, lnumber + 1);
  }
}

fdja_value *fdja_parse_r(const char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *input = flu_svprintf(format, ap);
  size_t lnumber = va_arg(ap, size_t);
  va_end(ap);

  fdja_value *r = fdja_parse_radial(input, NULL);
  renumber(r, lnumber);

  return r;
}

fdja_value *fdja_parse_obj(char *input)
{
  //printf("fdja_parse_obj() >[1;33m%s[0;0m<\n", input);

  //fabr_tree *at = fabr_parse_f(input, _obj, FABR_F_ALL);
  //fabr_puts(at, input, 3);
  //fabr_tree_free(at);

  fabr_tree *t = fabr_parse_all(input, _obj);
  //fabr_puts(t, input, 3);

  if (t->result != 1) { fabr_tree_free(t); return NULL; }

  fabr_tree *tobj = fabr_tree_lookup(t, "object");

  fdja_value *v = fdja_extract_v(input, tobj);
  if (v) v->sowner = 1;

  fabr_tree_free(t);

  return v;
}

fdja_value *fdja_dparse_obj(char *input)
{
  return fdja_parse_obj(strdup(input));
}

fdja_value *fdja_fparse_obj(FILE *f)
{
  return fdja_do_parse(f, NULL, NULL, 'o');
}

fdja_value *fdja_parse_obj_f(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *v = fdja_do_parse(NULL, path, ap, 'o');
  va_end(ap);

  return v;
}

fdja_value *fdja_c(const char *format, ...)
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

int fdja_is_stringy(fdja_value *v)
{
  return v->type == 's' || v->type == 'q' || v->type == 'y';
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

static int fdja_is_symbol(char *s) { return fabr_match(s, _symbol); }
static int fdja_is_number(char *s) { return fabr_match(s, _number); }

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

  if (v->key && (depth > 0 || flags & FDJA_F_OBJ))
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
    short br = ! (v->type == 'o' && depth == 0 && flags & FDJA_F_OBJ);

    if (ol)
    {
      fputs(infrac, f);
      if (v->type == 'a') fputc('[', f); else if (br) fputc('{', f);
      fputs(clearc, f);
      for (fdja_value *c = v->child; c != NULL; c = c->sibling)
      {
        if ( ! cp) fputc(' ', f);
        fdja_to_d(f, c, flags, depth + 1);
        if (c->sibling) fputc(',', f);
      }
      if ( ! cp && v->child) fputc(' ', f);
      fputs(infrac, f);
      if (v->type == 'a') fputc(']', f); else if (br) fputc('}', f);
      fputs(clearc, f);
    }
    else
    {
      char *d = fdja_to_djan(v, FDJA_F_ONELINE); size_t dl = strlen(d);

      if (dl < 3 || depth * 2 + dl < 80)
      {
        char *dd = fdja_to_djan(
          v,
          FDJA_F_ONELINE | (flags & FDJA_F_COLOR) | (flags & FDJA_F_COMPACT));
        if (br)
        {
          fputs(dd, f);
        }
        else
        {
          int off = dd[1] == ' ' ? 2 : 1;
          fwrite(dd + off, sizeof(char), strlen(dd) - (2 + off), f);
        }
        free(dd);
      }
      else
      {
        fputs(infrac, f);
        if (v->type == 'a') fputs("[\n", f); else if (br) fputs("{\n", f);
        fputs(clearc, f);
        for (fdja_value *c = v->child; c != NULL; c = c->sibling)
        {
          fdja_to_d(f, c, flags, depth + (br ? 1 : 0)); fputc('\n', f);
        }
        fputs(infrac, f);
        fputs(indent, f);
        if (v->type == 'a') fputc(']', f); else if (br) fputc('}', f);
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
  if (v == NULL)
  {
    if ((flags & FDJA_F_NULL) == 0) return NULL;
    if (flags & FDJA_F_COLOR) return strdup("[0;31mNULL[0;0m");
    return strdup("NULL");
  }

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

char *fdja_src(fdja_value *v)
{
  return v->source + v->soff;
}

char *fdja_srk(fdja_value *v)
{
  if (v->type == 's' || v->type == 'q') return v->source + v->soff + 1;
  return v->source + v->soff;
}

int fdja_strncmp(fdja_value *v, const char *s, ssize_t n)
{
  if (s == NULL) return -1;
  if (v == NULL) return -1;
  if ( ! fdja_is_stringy(v)) return -1;

  char *sv = fdja_to_string(v);
  int r = n < 0 ? strcmp(sv, s) : strncmp(sv, s, n);
  free(sv);
    // a bit costly but easier to follow

  return r;
}

int fdja_strcmp(fdja_value *v, const char *s)
{
  return fdja_strncmp(v, s, -1);
}

char *fdja_value_to_s(fdja_value *v)
{
  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbputc(b, '(');
  if (v->key) flu_sbprintf(b, "\"%s\": ", v->key);
  flu_sbprintf(b, "t'%c' ", v->type);
  flu_sbprintf(b, "%sown ", v->sowner ? "" : "!");
  flu_sbprintf(b, "off%zu ", v->soff);
  flu_sbprintf(b, "len%zu ", v->slen);
  flu_sbprintf(b, "sib%p ", v->sibling);
  flu_sbprintf(b, "chl%p ", v->child);
  char *s = strndup(v->source + v->soff, v->slen < 40 ? v->slen : 40);
  flu_sbprintf(b, ">%s<", s);
  free(s);
  flu_sbputc(b, ')');

  return flu_sbuffer_to_string(b);
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

int fdja_cmp(fdja_value *a, fdja_value *b)
{
  if (a == NULL && b == NULL) return 0;
  if (a == NULL || b == NULL) return -1;
  if (a == b) return 0;

  char *ja = fdja_to_json(a);
  char *jb = fdja_to_json(b);

  int r = strcmp(ja, jb);

  free(ja);
  free(jb);

  return r;
}

fdja_value *fdja_value_at(fdja_value *v, long long n)
{
  if (n < 0)
  {
    n = fdja_size(v) + n;
    if (n < 0) return NULL;
  }

  size_t i = 0; for (fdja_value *c = v->child; c; c = c->sibling)
  {
    if (i++ == n) return c;
  }

  return NULL;
}

static void unslash(char *s)
{
  for (size_t i = 0, j = 0; ; ++i)
  {
    if (s[i] == '\\' && s[i + 1] == '.') { s[j++] = '.'; ++i; continue; }
    s[j++] = s[i];
    if (s[i] == 0) break;
  }
}

fdja_value *fdja_vlookup(fdja_value *v, const char *path, va_list ap)
{
  //char *s = fdja_value_to_s(v); printf("fdja_vlookup() %s\n", s); free(s);

  char *p = flu_svprintf(path, ap);

  //printf("fdja_vlookup() >[1;33m%s[0;0m<\n", p);

  //fabr_tree *at = fabr_parse_f(p, _path, FABR_F_ALL);
  //fabr_puts(at, p, 3);
  //fabr_tree_free(at);

  fabr_tree *t = fabr_parse_all(p, _path);
  //fabr_puts(t, p, 3);

  if (t->result != 1) { fabr_tree_free(t); free(p); return NULL; }

  fdja_value *vv = v;

  flu_list *l = fabr_tree_list_named(t, "node");

  for (flu_node *n = l->first; n; n = n->next)
  {
    fabr_tree *tt = ((fabr_tree *)n->item)->child;
    char ltype = tt->name[0];
    //printf("ltype '%c' / vv->type '%c'\n", ltype, vv->type);

    //fabr_puts_tree(tt, p, 1);

    if (ltype == 'i' && vv->type == 'o') { ltype = 'k'; }
    else if (ltype == 'i' && vv->type != 'a') { vv = NULL; break; }
    else if (ltype == 'k' && vv->type != 'o') { vv = NULL; break; }

    char *s = fabr_tree_string(p, tt);
    //printf("s >%s<\n", s);

    if (ltype == 'i')
    {
      vv = fdja_value_at(vv, atol(s));
    }
    else // if (ltype == 'k')
    {
      unslash(s);
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

ssize_t fdja_lookup_size(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *vv = fdja_vlookup(v, path, ap);
  va_end(ap);

  return vv ? fdja_size(vv) : -1;
}

char fdja_lk(fdja_value *v, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  fdja_value *r = fdja_vlookup(v, path, ap);
  va_end(ap);

  return r ? *fdja_srk(r) : 0;
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
  if (v == NULL) return NULL;
  if (array->type != 'a') return NULL;

  free(v->key); v->key = NULL;

  for (fdja_value **l = &array->child; ; l = &(*l)->sibling)
  {
    if (*l == NULL) { *l = v; v->sibling = NULL; break; }
  }

  return v;
}

fdja_value *fdja_unshift(fdja_value *array, fdja_value *v)
{
  if (array->type != 'a') return NULL;

  fdja_value *c = array->child;
  array->child = v;
  v->sibling = c;

  return v;
}

int fdja_unpush(fdja_value *array, const char* val, ...)
{
  if (array->type != 'a') return 0;

  int r = -1; // "couldn't parse" for now
  fdja_value *v = NULL;

  va_list ap; va_start(ap, val); char *s = flu_svprintf(val, ap); va_end(ap);

  v = fdja_v(s); if (v == NULL) goto _over;

  r = 0; // "not found" for now

  for (fdja_value **l = &array->child; ; l = &(*l)->sibling)
  {
    if (*l == NULL) goto _over;
    if (fdja_cmp(*l, v) != 0) continue;
    fdja_value *rem = *l;
    *l = (*l)->sibling;
    fdja_free(rem);
    break;
  }

  r = 1; // "success"

_over:

  fdja_free(v);
  free(s);

  return r;
}

fdja_value *fdja_set(fdja_value *object, const char *key, ...)
{
  if (object == NULL) return NULL;
  if (object->type != 'o') return NULL;

  va_list ap; va_start(ap, key);
  char *k = flu_svprintf(key, ap); char *ok = k;
  fdja_value *val = va_arg(ap, fdja_value *);
  va_end(ap);

  fdja_value *v = val;

  short start = (val != NULL && *k == '\b');

  if (*k == '\b') k = k + 1;
  if (start) v = NULL;

  if (val != NULL)
  {
    free(val->key);
    val->key = strdup(k);
  }

  for (fdja_value **link = &object->child; ; link = &(*link)->sibling)
  {
    fdja_value *child = *link;

    if (child == NULL) // add at the end
    {
      *link = v;
      if (v) v->sibling = NULL;

      break;
    }

    if (strcmp(k, child->key) == 0) // found previous entry
    {
      if (v == NULL) // remove
      {
        *link = child->sibling;
        fdja_value_free(child);
      }
      else // add
      {
        *link = v;
        v->sibling = child->sibling;
        fdja_value_free(child);
      }

      break;
    }
  }

  if (start) // add at start
  {
    val->sibling = object->child;
    object->child = val;
  }

  free(ok); // free 'original key'

  return val;
}

fdja_value *fdja_oset(fdja_value *object, const char *key, ...)
{
  if (object->type != 'o') return NULL;

  va_list ap; va_start(ap, key);
  char *k = flu_svprintf(key, ap);
  fdja_value *val = va_arg(ap, fdja_value *);
  va_end(ap);

  free(val->key); val->key = k;

  for (fdja_value **link = &object->child; ; link = &(*link)->sibling)
  {
    fdja_value *child = *link;

    if (child == NULL) { *link = val; val->sibling = NULL; break; }

    int cmp = strcmp(k, child->key);

    if (cmp > 0) continue;

    *link = val;

    if (cmp == 0)
    {
      val->sibling = child->sibling;
      fdja_value_free(child);
    }
    else if (cmp < 0)
    {
      val->sibling = child;
    }
    break;
  }

  return val;
}

int fdja_merge(fdja_value *dst, fdja_value *src)
{
  if (dst == NULL || src == NULL) return 0;
  if (dst->type != 'o' || src->type != 'o') return 0;

  for (fdja_value *n = src->child; n; n = n->sibling)
  {
    fdja_set(dst, n->key, fdja_clone(n));
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

  char *key = p;
  char *pat = NULL;

  char *lastdot = strrchr(p, '.');
  if (lastdot)
  {
    key = lastdot + 1;
    pat = strndup(p, lastdot - p);
  }

  fdja_value *target = start;
  if (pat) target = fdja_lookup(start, pat);

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

fdja_value *fdja_psetv(fdja_value *start, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *p = flu_svprintf(path, ap);
  char *f = va_arg(ap, char *);
  char *s = flu_svprintf(f, ap);
  va_end(ap);

  fdja_value *r = NULL;

  fdja_value *v = fdja_parse(s);
  if (v == NULL) { free(s); goto _over; }

  r = fdja_pset(start, p, v);
  if (r == NULL) fdja_free(v);

_over:

  free(p);

  return r;
}

void fdja_replace(fdja_value *old, fdja_value *new)
{
  old->type = new->type;

  if (old->sowner) free(old->source);
  old->source = new->source;
  old->sowner = new->sowner; new->sowner = 0;
  old->soff = new->soff;
  old->slen = new->slen;

  for (fdja_value *c = old->child; c; )
  {
    fdja_value *sibling = c->sibling; fdja_free(c); c = sibling;
  }
  old->child = new->child; new->child = NULL;

  fdja_free(new);
}

//commit 08a6159f2ebe5828f0e5ab357fbc45187de46085
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Thu Jul 16 06:38:24 2015 +0900
//
//    enable comments within rad_p groups (radial ()s)
