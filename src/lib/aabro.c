
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

// https://github.com/flon-io/aabro

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "aabro.h"


fabr_tree *fabr_tree_malloc(
  char *name, char *parter, fabr_input *i, size_t rexlen)
{
  fabr_tree *t = calloc(1, sizeof(fabr_tree));

  t->name = name ? strdup(name) : NULL;
  t->result = 1;
  t->parter = parter;
  t->offset = i->offset;
  t->rexlen = rexlen;
  t->length = 0;
  t->note = NULL;
  t->sibling = NULL;
  t->child = NULL;

  return t;
}

static void fabr_tree_free_children(fabr_tree *t)
{
  for (fabr_tree *c = t->child; c != NULL; )
  {
    fabr_tree *s = c->sibling;
    fabr_tree_free(c);
    c = s;
  }
  t->child = NULL;
}

void fabr_tree_free(fabr_tree *t)
{
  if (t == NULL) return;

  free(t->name);
  free(t->note);

  fabr_tree_free_children(t);

  free(t);
}

void fabr_prune(fabr_tree *t)
{
  fabr_tree **next = &t->child;

  for (fabr_tree *c = t->child; c; )
  {
    fabr_tree *s = c->sibling;

    if (c->result != 0)
    {
      *next = c;
      next = &c->sibling;
      *next = NULL;
    }
    else
    {
      fabr_tree_free(c);
    }

    c = s;
  }
}

char *fabr_tree_string(const char *input, fabr_tree *t)
{
  return strndup(input + t->offset, t->length);
}

char *fabr_tree_str(const char *input, fabr_tree *t)
{
  return (char *)input + t->offset;
}

long long fabr_tree_llong(const char *input, fabr_tree *t, int base)
{
  char *s = fabr_tree_string(input, t);
  long long r = strtoll(s, NULL, base);
  free(s);

  return r;
}

static void fabr_t_to_s(
  fabr_tree *t, const char *input,
  flu_sbuffer *b, size_t indent, short children, short color)
{
  for (size_t i = 0; i < indent; i++) flu_sbprintf(b, "  ");

  if (t == NULL) { flu_sbprintf(b, "null"); return; }

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
    "%s[ %s, %d, %d, %d, %s, \"%s\", %d, ",
    resultc, name, t->result, t->offset, t->length, note, t->parter, t->rexlen);
  //
  if (t->name) free(name);
  if (t->note) free(note);

  if (children == 0 && (input == NULL || t->result != 1 || t->child))
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

void fabr_puts_tree(fabr_tree *t, const char *input, short color)
{
  char *s = fabr_tree_to_string(t, input, color); puts(s); free(s);
}

char *fabr_tree_to_str(fabr_tree *t, const char *input, short color)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fabr_t_to_s(t, input, b, 0, 0, color);

  return flu_sbuffer_to_string(b);
}

void fabr_tree_puts(fabr_tree *t, const char *input, short flags)
{
  flu_sbuffer *b = flu_sbuffer_malloc();
  fabr_t_to_s(t, input, b, 0, flags & 2, flags & 1);
  char *s = flu_sbuffer_to_string(b);

  puts(s);

  free(s);
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

char *fabr_lookup_string(const char *input, fabr_tree *t, const char *name)
{
  fabr_tree *tt = fabr_tree_lookup(t, name);

  return tt ? fabr_tree_string(input, tt) : NULL;
}

static void fabr_t_list(
  flu_list *l, fabr_tree *t, fabr_tree_func *f, int skip)
{
  if ( ! skip)
  {
    short r = f(t);

    if (r < 0) { return; }
    if (r > 0) { flu_list_add(l, t); return; }
  }

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    fabr_t_list(l, c, f, 0);
  }
}

flu_list *fabr_tree_list(fabr_tree *t, fabr_tree_func *f)
{
  if (t == NULL) return NULL;

  flu_list *l = flu_list_malloc();
  fabr_t_list(l, t, f, 0);

  return l;
}

flu_list *fabr_tree_list_cn(fabr_tree *t, fabr_tree_func *f)
{
  if (t == NULL) return NULL;

  flu_list *l = flu_list_malloc();
  fabr_t_list(l, t, f, 1);

  return l;
}

static void fabr_t_list_named(
  flu_list *l, fabr_tree *t, const char *name, int skip)
{
  if ( ! skip)
  {
    if (t->result != 1) { return; }
    if (t->name && name == NULL) { flu_list_add(l, t); return; }
    if (t->name && strcmp(t->name, name) == 0) { flu_list_add(l, t); return; }
  }

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    fabr_t_list_named(l, c, name, 0);
  }
}

flu_list *fabr_tree_list_named(fabr_tree *t, const char *name)
{
  if (t == NULL) return NULL;

  flu_list *l = flu_list_malloc();
  fabr_t_list_named(l, t, name, 0);

  return l;
}

flu_list *fabr_tree_list_named_cn(fabr_tree *t, const char *name)
{
  if (t == NULL) return NULL;

  flu_list *l = flu_list_malloc();
  fabr_t_list_named(l, t, name, 1);

  return l;
}

fabr_tree **fabr_tree_collect(fabr_tree *t, fabr_tree_func *f)
{
  flu_list *l = fabr_tree_list(t, f);

  fabr_tree **ts = (fabr_tree **)flu_list_to_array(l, FLU_F_EXTRA_NULL);
  flu_list_free(l);

  return ts;
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

fabr_tree *fabr_t_path(fabr_tree *t, size_t index, ...)
{
  t = fabr_t_child(t, index);

  if (t == NULL) return NULL;

  va_list ap; va_start(ap, index);
  while (t)
  {
    int i = va_arg(ap, int);
    if (i < 0) break;
    t = fabr_t_child(t, i);
  }
  va_end(ap);

  return t;
}


//
// parters (partial parsers)

static fabr_tree *str(fabr_input *i, char *rx, size_t rxn)
{
  //printf("str() i+o>%s< vs >%.*s<\n", i->string + i->offset, (int)rxn, rx);

  fabr_tree *r = fabr_tree_malloc(NULL, "str", i, rxn);

  if (strncmp(i->string + i->offset, rx, rxn) != 0)
  {
    r->result = 0;
  }
  else
  {
    i->offset += rxn;
    r->length = rxn;
  }

  return r;
}

fabr_tree *fabr_str(
  char *name, fabr_input *i, char *s)
{
  //printf("str()  >[0;34m%s[0;0m<\n", s);
  //printf("i+o    >[1;33m%s[0;0m<\n", i->string + i->offset);

  fabr_tree *r = str(i, s, strlen(s));

  r->name = name ? strdup(name) : NULL;

  //printf("str()  result %d %zu\n", r->result, r->length);

  return r;
}

//static size_t mm = 0;

fabr_tree *fabr_qmark(fabr_input *i) { return NULL; }
fabr_tree *fabr_star(fabr_input *i) { return NULL; }
fabr_tree *fabr_plus(fabr_input *i) { return NULL; }

fabr_tree *fabr_seq(
  char *name, fabr_input *i, fabr_parser *p, ...)
{
  //size_t m = mm++; printf("S %zu fabr_seq() \"%s\"\n", m, name);

  size_t off = i->offset;

  fabr_tree *r = fabr_tree_malloc(name, "seq", i, 0);

  fabr_tree **next = &r->child;

  va_list ap; va_start(ap, p);
  while (1)
  {
    if (r->result != 1) break;

    if (p == NULL) break;

    if (p == fabr_qmark || p == fabr_star || p == fabr_plus)
    {
      r->result = -1;
      r->note = strdup("bad position for fabr_qmark, _star or _plus");
      break;
    }

    fabr_parser *np = va_arg(ap, fabr_parser *);

    for (size_t count = 0; ; count++)
    {
      size_t ffo = i->offset;

      fabr_tree *t = p(i);

      if (t->result == -1) { r->result = -1; break; }

      if (t->result == 0 && i->flags & FABR_F_PRUNE)
      {
        fabr_tree_free(t);
        t = NULL;
      }
      else
      {
        *next = t;
        next = &t->sibling;
        r->length += t->length;
      }

      if (np == fabr_qmark) // ?
      {
        break;
      }
      /* else */ if (np == fabr_star) // *
      {
        if (t == NULL || t->result == 0) break;
        if (i->offset == ffo) break; // no progress
      }
      else if (np == fabr_plus) // +
      {
        if (t == NULL || t->result == 0)
        {
          if (count == 0) r->result = 0;
          break;
        }
        if (i->offset == ffo) break; // no progress
      }
      else
      {
        if (t == NULL || t->result == 0) r->result = 0;
        break;
      }
    }

    p = np; // well...

    if (p == fabr_qmark || p == fabr_star || p == fabr_plus)
    {
      p = va_arg(ap, fabr_parser *);
    }
  }
  va_end(ap);

  if (r->result != 1) { r->length = 0; i->offset = off; }

  //printf(
  //  "  %zu fabr_seq() \"%s\" res %d len %zu\n", m, name, r->result, r->length);

  return r;
}

fabr_tree *fabr_altgr(
  char *name, fabr_input *i, short greedy, fabr_parser *p, ...)
{
  //size_t m = mm++; printf("A %zu fabr_altg() %d \"%s\"\n", m, greedy, name);

  size_t off = i->offset;

  fabr_tree *r = fabr_tree_malloc(name, greedy ? "altg" : "alt", i, 0);
  r->result = 0;

  fabr_tree **next = &r->child;

  fabr_tree *winner = NULL;

  va_list ap; va_start(ap, p);
  while (1)
  {
    fabr_tree *t = p(i);
    *next = t;

    if (t->result == -1) { winner = t; break; }

    if (t->result == 1)
    {
      if ( ! greedy) { winner = t; break; }
      if (winner == NULL || t->length >= winner->length)
      {
        if (winner) { winner->result = 0; winner->length = 0; }
        winner = t;
      }
      else
      {
        t->result = 0;
      }
    }

    p = va_arg(ap, fabr_parser *); if (p == NULL) break;

    i->offset = off;
    next = &(t->sibling);
  }
  va_end(ap);

  if (winner)
  {
    r->result = winner->result;
    r->length = winner->length;
    i->offset = off + winner->length;
  }

  //fabr_puts(r, i->string, 3);

  if (r->result == 1 && (i->flags & FABR_F_PRUNE)) fabr_prune(r);

  //printf(
  //  "  %zu fabr_alt() %d \"%s\" res %d len %zu\n",
  //  m, greedy, name, r->result, r->length);

  return r;
}

fabr_tree *fabr_rep(
  char *name, fabr_input *i, fabr_parser *p, size_t min, size_t max)
{
  size_t off = i->offset;

  fabr_tree *r = fabr_tree_malloc(name, "rep", i, 0);

  fabr_tree **next = &r->child;
  size_t count = 0;

  while (1)
  {
    size_t ffo = i->offset;

    fabr_tree *t = p(i);

    if (t->result == 0)
    {
      if (i->flags & FABR_F_PRUNE) fabr_tree_free(t); else *next = t;
      break;
    }

    *next = t;

    if (t->result == -1) { r->result = -1; break; }

    r->length += t->length;

    if (++count == max) break;

    if (ffo == i->offset) break; // no progress

    next = &(t->sibling);
  }

  if (r->result == 1 && count < min) r->result = 0;
  if (r->result != 1) { r->length = 0; i->offset = off; }

  return r;
}

static char rx_at(char *rx, size_t rxn, size_t index)
{
  return rxn > index ? rx[index] : 0;
}

static char *rx_chr(char *rx, size_t rxn, int c)
{
  char *r = strchr(rx, c);

  return r - rx >= rxn ? NULL : r;
}

typedef fabr_tree *fabr_rex_parser(fabr_input *i, char *rx, size_t rxn);

static void rng_next(char *rx, size_t rxn, char *next)
{
  size_t b_index = 1;

  char a = rx_at(rx, rxn, 0);
  if (a == '\\') { a = rx_at(rx, rxn, 1); b_index = 2; }
  if (a == '\0') { next[0] = 0; next[1] = 0; next[2] = 0; return; }

  char b = rx_at(rx, rxn, b_index);
  char c = (b != '\0') ? rx_at(rx, rxn, b_index + 1) : 'X'; // don't go over
  if (b != '-' || c == '\0') { next[0] = 1; next[1] = a; next[2] = a; return; }

  b = rx_at(rx, rxn, ++b_index);
  if (b == '\\') b = rx_at(rx, rxn, ++b_index);
  next[0] = 2; next[1] = a; next[2] = b;
}

static fabr_tree *rng(fabr_input *i, char *rx, size_t rxn)
{
  //printf("        rng() >[0;34m%.*s[0;0m<\n", (int)rxn, rx);
  //printf("        i+o   >[1;33m%s[0;0m<\n", i->string + i->offset);

  fabr_tree *r = fabr_tree_malloc(NULL, "rng", i, rxn);

  char c = (i->string + i->offset)[0];
  char irc = rx_at(rx, rxn, 0);

  if (irc == '$') { r->result = (c == '\0'); return r; }
  if (c == '\0') { r->result = 0; return r; }

  if (irc == '.')
  {
    if (c == '\n') r->result = 0; else r->length = 1;
    return r;
  }

  r->result = 0;

  short not = (irc == '^'); if (not) { rx++; rxn--; }

  char next[] = { 0, 0, 0 };
  while (1)
  {
    rng_next(rx, rxn, next);
    if (next[0] == 0) break;
    if (c >= next[1] && c <= next[2]) { r->result = 1; break; }
    rx += next[0]; rxn -= next[0];
  }

  if (not) r->result = ( ! r->result);

  if (r->result == 1)
  {
    r->length = 1;
    i->offset += 1;
  }

  //printf("        rng result: %d %zu\n", r->result, r->length);

  return r;
}

fabr_tree *fabr_rng(
  char *name, fabr_input *i, char *range)
{
  fabr_tree *r = rng(i, range, strlen(range));

  r->name = name ? strdup(name) : NULL;

  return r;
}

static fabr_tree *ferr(fabr_input *i, char *parter, char *format, ...)
{
  fabr_tree *r = fabr_tree_malloc(NULL, parter, i, 0);
  r->result = -1;

  va_list ap; va_start(ap, format);
  r->note = flu_svprintf(format, ap);
  va_end(ap);

  return r;
}

static ssize_t quantify(char *rx, size_t rxn, size_t *reps)
{
  char c = rx_at(rx, rxn, 0);

  if (c == '?') { reps[0] = 0; reps[1] = 1; return 1; }
  if (c == '*') { reps[0] = 0; reps[1] = 0; return 1; }
  if (c == '+') { reps[0] = 1; reps[1] = 0; return 1; }
  if (c != '{') return 0;

  // find the }

  size_t z = 0;

  for (size_t i = 1, comma = 0; ; i++) {

    char cc = rx_at(rx, rxn, i);

    if (cc == '\0') { break; }
    if (cc == ',') { comma++; if (comma > 1) break; /*else*/ continue; }
    if (cc == '}') { z = i; break; }
    if (cc < '0' || cc > '9') { break; }
  }

  if (z == 0) return -1; // error

  // } found

  reps[0] = strtol(rx + 1, NULL, 10);

  char *comma = rx_chr(rx, rxn, ',');

  reps[1] = comma == NULL ? reps[0] : strtol(comma + 1, NULL, 10);
  return z + 1;
}

static size_t find_range_end(char *rx, size_t rxn)
{
  for (size_t i = 1; ; i++)
  {
    char c = rx_at(rx, rxn, i);

    if (c == '\0') return 0;
    if (c == '\\') { i++; continue; }
    if (c != ']') continue;
    return i;
  }

  return 0;
}

static size_t find_group_end(char *rx, size_t rxn)
{
  for (size_t i = 1, range = 0, groups = 0; ; i++)
  {
    char c = rx_at(rx, rxn, i);

    if (c == '\0') break;
    if (c == '\\') { i++; continue; }

    if (range == 0 && c == '[') { range = 1; continue; }
    if (range == 1 && c == ']') { range = 0; continue; }
    if (range == 1) continue;

    if (c == '(') { groups++; continue; }
    if (c != ')') continue;
    if (groups == 0) return i;
    groups--;
  }

  return 0;
}

static ssize_t find_str_end(char *rx, size_t rxn)
{
  //printf("fse >%s<%zu\n", rx, rxn);
  //printf("fse >%.*s<%zu\n", (int)rxn, rx, rxn);

  for (size_t i = 0; ; i++)
  {
    char c = rx_at(rx, rxn, i);

    //printf("fse c'%c'\n", c);

    if (c == '\0') return i;
    if (c == '[' || c == '(') return i;
    if (c == '\\') { i++; continue; }

    if (c != '?' && c != '*' && c != '+' && c != '{') continue;

    if (i == 0) break;

    if (i == 1) return i; // a*
    //if (i == 2 && rx_at(rx, rxn, 0) == '\\') return 0; // \a*

    //if (rx_at(rx, rxn, i - 2) == '\\') return i - 2;
    if (rx_at(rx, rxn, i - 2) == '\\') return i - 1;
    return i - 1; // \a\a*
  }

  return -1; // error
}

static fabr_tree *rex_str(fabr_input *i, char *rx, size_t rxn)
{
  //printf(
  //  "      * rex_str() >[0;34m%.*s[0;0m<\n", (int)rxn, rx);
  //printf(
  //  "        i+o       >[1;33m%s[0;0m<\n", i->string + i->offset);

  fabr_tree *r = fabr_tree_malloc(NULL, "rex_str", i, rxn);

  size_t ii = 0;
  size_t ri = 0;

  while (1)
  {
    char rc = rx_at(rx, rxn, ri++);
    char prc = rc;

    if (rc == '\\') rc = rx_at(rx, rxn, ri++);
    //printf(". rc >%c<\n", rc);
    if (rc == '\0') break;

    char ic = *(i->string + i->offset + ii++);
    //printf("  ic >%c<\n", ic);

    if (rc == 'Z' && prc == '\\') // EOS
    {
      if (ic != '\0') r->result = 0;
      ii--; break;
    }
    if (rc == '$' && prc == rc) // unescaped dollar
    {
      if (ic != '\n' && ic != '\r' && ic != '\0') r->result = 0;
      ii--; break;
    }

    if (ic == '\0') { r->result = 0; break; }

    if (rc == '.' && prc == rc && ic != '\n' && ic != '\r') continue;

    if (ic != rc) { r->result = 0; break; }
  }

  if (r->result == 1) {
    i->offset += ii;
    r->length = ii;
  }
  if (r->length == 0) r->result = 0; // ...

  //printf(
  //  "        rex_str() result: %d %zu\n", r->result, r->length);

  return r;
}

static fabr_tree *rex_alt(fabr_input *i, char *rx, size_t rxn);
  // forward declaration

static fabr_tree *rex_rep(fabr_input *i, char *rx, size_t rxn)
{
  //size_t m = mm++;
  //printf(
  //  "    * %zu rex_rep() >[0;34m%.*s[0;0m<\n", m, (int)rxn, rx);
  //printf(
  //  "    * %zu i+o       >[1;33m%s[0;0m<\n", m, i->string + i->offset);

  char rc = rx_at(rx, rxn, 0);

  fabr_rex_parser *p = NULL;
  ssize_t z = 0;
  size_t off = 0;

  if (rc == '[')
  {
    p = rng;
    z = find_range_end(rx, rxn);
    //printf("      %zu fre >[0;34m%s[0;0m< --> %zu\n", m, (int)rxn, rx, z);
    if (z == 0) return ferr(i, "rex_rep", "range not closed >%s<%zu", rx, rxn);
    off = 1;
  }
  else if (rc == '(')
  {
    p = rex_alt;
    z = find_group_end(rx, rxn);
    //printf("      %zu fge >[0;34m%s[0;0m< --> %zu\n", m, (int)rxn, rx, z);
    if (z == 0) return ferr(i, "rex_rep", "group not closed >%s<%zu", rx, rxn);
    off = 1;
  }
  else
  {
    p = rex_str;
    z = find_str_end(rx, rxn);
    //printf("      %zu fse >[0;34m%.*s[0;0m< --> %zd\n", m, (int)rxn, rx, z);
    if (z == -1) return ferr(i, "rex_rep", "lone quantifier >%s<%zu", rx, rxn);
  }

  size_t mm[] = { 0, 0 };
  ssize_t mml = quantify(rx + z + off, rxn - z - off, mm);

  //printf(
  //  "      %zu qtf >%s<%zu mml %zd mm[%zu, %zu]\n",
  //  //m, rx + z + off, rxn - z - off, mml, mm[0], mm[1]);
  //  0, rx + z + off, rxn - z - off, mml, mm[0], mm[1]);

  if (mml == -1)
  {
    return ferr(i, "rex_rep", "invalid {min[,max]} >%s<%zu", rx, rxn);
  }
  if (mml == 0)
  {
    fabr_tree *r = p(i, rx + off, z - off);
    r->rexlen = z + off + mml;
    return r;
  }

  // yes, the following code is a repetition of what comes in fabr_rep()

  fabr_tree *r = fabr_tree_malloc(NULL, "rex_rep", i, z + off + mml);

  fabr_tree **next = &r->child;
  size_t count = 0;

  while (1)
  {
    size_t ffo = i->offset;

    fabr_tree *t = p(i, rx + off, z - off);
    *next = t;

    if (t->result == -1) { r->result = -1; break; }
    if (t->result == 0) break;

    r->length += t->length;

    if (++count == mm[1]) break;

    if (ffo == i->offset) break; // no progress

    next = &(t->sibling);
  }

  if (r->result == 1 && count < mm[0]) r->result = 0;
  if (r->result != 1) r->length = 0;

  return r;
}

static fabr_tree *rex_seq(fabr_input *i, char *rx, size_t rxn)
{
  //size_t m = mm++;
  //printf("  * %zu rex_seq() >[0;34m%.*s[0;0m<\n", m, (int)rxn, rx);
  //printf("  * %zu i+o       >[1;33m%s[0;0m<\n", m, i->string + i->offset);

  size_t off = i->offset;

  fabr_tree *r = fabr_tree_malloc(NULL, "rex_seq", i, rxn);

  fabr_tree *prev = NULL;
  fabr_tree **next = &r->child;

  char *crx = rx;
  size_t crxn = rxn;

  while (1)
  {
    if (rx_at(crx, crxn, 0) == '\0') break;

    *next = rex_rep(i, crx, crxn);
    prev = *next;
    next = &(prev->sibling);

    //printf(
    //  "    %zu prev %s r%d l%zu rl%zu\n",
    //  m, prev->parter, prev->result, prev->length, prev->rexlen);

    if (prev->result != 1) break;

    r->length += prev->length;

    crx += prev->rexlen; crxn -= prev->rexlen;

    //printf(
    //  "    %zu post i+o>%s< crx>%s<%zu\n",
    //  m, i->string + i->offset, crx, crxn);
  }

  r->result = prev ? prev->result : 0;

  if (r->result != 1) { r->length = 0; i->offset = off; }

  //printf("    %zu rex_seq() result: %d %zu\n", m, r->result, r->length);

  return r;
}

static fabr_tree *rex_alt(fabr_input *i, char *rx, size_t rxn)
{
  fabr_tree *r = fabr_tree_malloc(NULL, "rex_alt", i, rxn);

  // TODO/WARNING: greedy, the wants to have the longest match...

  fabr_tree *prev = NULL;
  fabr_tree **next = &r->child;

  char c = 'a';
  char *crx = rx;
  size_t crxn = rxn;

  //size_t m = mm++; size_t n = 0;

  do
  {
    //printf(
    //  "* %zu.%zu rex_alt() >[0;34m%.*s[0;0m< c%i\n",
    //  m, n, (int)crxn, crx, c);
    //printf(
    //  "  %zu.%zu i+o       >[1;33m%s[0;0m<\n",
    //  m, n, i->string + i->offset);

    for (size_t j = 0, range = 0, groups = 0; ; j++)
    {
      c = j >= crxn ? 0 : crx[j];

      if (c == '\0')
      {
        *next = rex_seq(i, crx, j);
        prev = *next;
        break;
      }

      if (c == '\\') { j++; continue; }

      if (range == 0 && c == '[') { range = 1; continue; }
      if (range == 1 && c == ']') { range = 0; continue; }
      if (range == 1) continue;

      if (c == '(') { groups++; continue; }
      if (groups > 0) { if (c == ')') groups--; continue; }

      if (c == '|')
      {
        *next = rex_seq(i, crx, j);
        prev = *next; next = &prev->sibling;
        crx = crx + j + 1; crxn = crxn - j - 1;
        break;
      }

      // else continue
    }

    //printf(
    //  "  %zu.%zu rex_alt() result: %d %zu\n",
    //  m, n, prev->result, prev->length);
    //n++;

    if (prev->result != 0) break;

  } while (c != 0);

  if (prev->result != 1)
    r->result = prev->result;
  else
    r->length = prev->length;

  return r;
}

fabr_tree *fabr_rex(
  char *name, fabr_input *i, char *regex)
{
  fabr_tree *r = rex_alt(i, regex, strlen(regex));

  r->parter = "rex";
  r->name = name ? strdup(name) : NULL;

  if ((i->flags & FABR_F_PRUNE) && r->result == 1) fabr_tree_free_children(r);

  return r;
}

fabr_tree *fabr_eseq(
  char *name, fabr_input *i,
  fabr_parser *startp, fabr_parser *eltp, fabr_parser *sepp, fabr_parser *endp)
{
  size_t off = i->offset;

  short prune = i->flags & FABR_F_PRUNE;
  short jseq = (startp == NULL && endp == NULL);

  fabr_tree *r = fabr_tree_malloc(name, "eseq", i, 0);
  fabr_tree **next = &r->child;

  if (startp)
  {
    fabr_tree *t = startp(i);
    *next = t;

    if (t->result != 1) { r->result = t->result; return r; }

    r->length += t->length;
    next = &t->sibling;
  }

  for (size_t j = 0; ; j++)
  {
    short over = 0;
    fabr_tree *st = NULL;
    fabr_tree *et = NULL;

    if (j > 0) st = sepp(i);
    if (st == NULL || st->result == 1) et = eltp(i);

    // determine r->result

    if (st && st->result == 0)
      over = 1;
    else if (st && st->result == -1)
      r->result = -1;
    else if (st && st->result == 1 && st->length == 0 && et->result == 0)
      over = 2;
    else if (st && st->result == 1 && st->length == 0 && et->result == 1 && et->length == 0)
      over = 2;
    else if (j == 0 && et && et->result == 0 && jseq == 0)
      over = 1;
    else if (et && et->result != 1)
      r->result = et->result;

    // add or free

    if (over != 2 && st && (st->result != 0 || prune == 0))
    {
      *next = st; next = &st->sibling; r->length += st->length;
      st = NULL;
    }

    if (over != 2 && et && (et->result != 0 || prune == 0))
    {
      *next = et; next = &et->sibling; r->length += et->length;
      et = NULL;
    }

    fabr_tree_free(st);
    fabr_tree_free(et);

    // break or continue

    if (over || r->result != 1) break;
  }

  if (r->result == 1 && endp)
  {
    fabr_tree *t = endp(i);
    *next = t;

    r->result = t->result;
    r->length += t->length;
  }

  if (r->result != 1) { r->length = 0; i->offset = off; }

  return r;
}

fabr_tree *fabr_rename(char *name, fabr_input *i, fabr_parser *p)
{
  fabr_tree *r = p(i);

  if (r->name) { free(r->name); r->name = NULL; }
  if (name) r->name = strdup(name);

  return r;
}

fabr_tree *fabr_eos(char *name, fabr_input *i)
{
  fabr_tree *r = fabr_tree_malloc(name, "eos", i, 0);
  r->result = *(i->string + i->offset) == '\0';

  return r;
}

fabr_tree *fabr_all(
  char *name, fabr_input *i, fabr_parser *p)
{
  fabr_tree *r = fabr_tree_malloc(name, "all", i, 0);

  char *start = i->string + i->offset;

  r->child = p(i);

  if (r->child->result == 1)
  {
    r->result = r->child->length == strlen(start);
    r->length = r->result == 1 ? r->child->length : 0;
  }
  else
  {
    r->result = r->child->result;
  }

  return r;
}


//
// helpers

fabr_tree *fabr_parse(const char *input, fabr_parser *p)
{
  return fabr_parse_f(input, p, FABR_F_PRUNE);
}

fabr_tree *fabr_parse_all(const char *input, fabr_parser *p)
{
  return fabr_parse_f(input, p, FABR_F_PRUNE | FABR_F_ALL);
}

fabr_tree *fabr_parse_f(const char *input, fabr_parser *p, int flags)
{
  fabr_input i = { (char *)input, 0, flags };

  if (flags & FABR_F_ALL) return fabr_all(NULL, &i, p);
  return p(&i);
}

int fabr_match(const char *input, fabr_parser *p)
{
  fabr_tree *t =
    fabr_parse_f(input, p, FABR_F_ALL | FABR_F_PRUNE | FABR_F_MATCH);

  int r = t->result;

  fabr_tree_free(t);

  return r;
}

//commit 7198da86d267f621942f9bbcbcc3bbc24df1497b
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Wed Aug 26 06:38:57 2015 +0900
//
//    implement fabr_tree_llong()
