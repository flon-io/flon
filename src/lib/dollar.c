
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

// https://github.com/flon-io/dollar

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "aabro.h"

#include "dollar.h"


static fabr_parser *fdol_parser = NULL;
static fabr_parser *fdol_pipe_parser = NULL;

static void fdol_parser_init()
{
  // parser

  fabr_parser *dol =
    fabr_n_seq(
      "d", fabr_string("$("), fabr_n("p"), fabr_string(")"), NULL);
  fabr_parser *str =
    fabr_n_rex(
      "s",
      "("
        "\\\\\\)" "|"
        "[^\\$\\)]" "|"
        "\\$[^\\(]"
      ")+");

  //fabr_parser *span =
    fabr_n_rep(
      "p", fabr_alt(dol, str, NULL), 0, -1);

  fabr_parser *outerstr =
    fabr_n_rex(
      "s",
      "("
        "[^\\$]" "|" // doesn't mind ")"
        "\\$[^\\(]"
      ")+");

  fdol_parser =
    fabr_n_rep(
      "r", fabr_alt(dol, outerstr, NULL), 0, -1);

  // TODO: give possibility to escape )

  // pipe parser

  fabr_parser *not_pipe = fabr_n_rex("s", "[^|]+");
  fabr_parser *pipe = fabr_n_rex("p", "\\|\\|?");

  fdol_pipe_parser =
    fabr_seq(not_pipe, fabr_seq(pipe, not_pipe, fabr_r("*")), NULL);
}


//
// fdol_expand()

static char *fun_reverse(char *s)
{
  size_t l = strlen(s);
  char *r = calloc(l + 1, sizeof(char));

  for (ssize_t i = l - 1, j = 0; i > -1; ) r[j++] = s[i--];

  return r;
}

static char *fun_case(int (* f)(int), char *s)
{
  size_t l = strlen(s);
  char *r = calloc(l + 1, sizeof(char));

  for (size_t i = 0; i < l; ++i) r[i] = f(s[i]);

  return r;
}

static char *fun_capitalize(char *s)
{
  char *r = fun_case(tolower, s);
  short docap = 1;

  for (char *rr = r; *rr != 0; rr++)
  {
    if (strchr(" \t\r\n.", *rr)) { docap = 1; }
    else { if (docap) { *rr = toupper(*rr); docap = 0; } }
  }

  return r;
}

static char *fun_range(char *func, char *s)
{
  char *r = NULL;
  size_t l = strlen(s);
  char *dots = strstr(func, "..");
  if (dots)
  {
    long long a = strtoll(func, NULL, 10);
    long long b = strtoll(dots + 2, NULL, 10);
    while (a < 0) a = l + a;
    while (b < 0) b = l + b;
    size_t n = 0; if (b > a) n = b + 1 - a;
    r = strndup(s + a, n);
  }
  else
  {
    long long i = strtoll(func, NULL, 10);
    while (i < 0) i = l + i;
    r = strndup(s + i, 1);
  }

  return r;
}

static char *fun_length_filter(char *f, char *s)
{
  size_t sl = strlen(s);
  size_t off = 2; if (f[2] == '=' || f[2] == '>') off = 3;
  size_t l = strtoll(f + off, NULL, 10);

  //printf("flf() f \"%s\" s \"%s\" (%zu)\n", f, s, sl);

  int t = 0;

  if (f[1] == '=')
  {
    t = (sl == l);
  }
  else if (f[1] == '!' || (f[1] == '<' && f[2] == '>'))
  {
    t = (sl != l);
  }
  else if (off == 2)
  {
    if (f[1] == '<') t = (sl < l);
    else t = (sl > l);
  }
  else // if (off == 3) // <= >=
  {
    if (f[1] == '<') t = (sl <= l);
    else t = (sl >= l);
  }

  return t ? s : NULL;
}

static char *fun_quote(char *f, char *s)
{
  if (*f == 'q' && s[0] == '"' && s[strlen(s) - 1] == '"') return s;

  char *ss = flu_escape(s);
  size_t l = strlen(ss);
  char *r = calloc(l + 3, sizeof(char));
  r[0] = '"'; strcpy(r + 1, ss); r[l + 1] = '"';
  free(ss);

  return r;
}

static char *call(char *s, char *f)
{
  //printf("call() >%s< on >%s<\n", f, s);

  if (s == NULL) return NULL;

  if (*f == 'r') return fun_reverse(s);
  if (*f == 'u') return fun_case(toupper, s);
  if (*f == 'd') return fun_case(tolower, s);
  if (*f == 'c') return fun_capitalize(s);
  if (*f == 'l') return fun_length_filter(f, s);
  if (tolower(*f) == 'q') return fun_quote(f, s);

  if (*f == '-' || (*f >= '0' && *f <= '9')) return fun_range(f, s);

  return strdup(s);
}

static char *eval(const char *s, void *data, fdol_lookup *func)
{
  fabr_tree *t = fabr_parse_all(s, 0, fdol_pipe_parser);

  if (t == NULL) return strdup(s);

  //puts(fabr_tree_to_string(t, s, 1));

  char *ss = fabr_tree_string(s, t->child);
  char *r = func(data, ss);
  free(ss);

  if (t->child->sibling->child)
  {
    char mode = 'l'; // 'l'ookup vs 'c'all

    for (fabr_tree *c = t->child->sibling->child; c; c = c->sibling)
    {
      mode = (c->child->length) == 1 ? 'c' : 'l';
      ss = fabr_tree_string(s, c->child->sibling);
      //printf("mode '%c' ss >%s<\n", mode, ss);

      if (mode == 'l')
      {
        if (r == NULL)
        {
          if (*ss == '\'') r = strdup(ss + 1);
          else r = func(data, ss);
        }
      }
      else // (mode == 'c')
      {
        char *or = r;
        r = call(r, ss);
        if (or != r) free(or);
      }

      free(ss);
    }
  }

  fabr_tree_free(t);

  return r;
}

static char *unescape(char *s)
{
  char *r = calloc(strlen(s) + 1, sizeof(char));
  for (size_t i = 0, j = 0; s[i] != 0; ++i)
  {
    if (s[i] != '\\') r[j++] = s[i];
  }
  free(s);

  return r;
}

static char *expand(
  const char *s, fabr_tree *t, int quote, void *data, fdol_lookup *func)
{
  //puts(fabr_tree_to_string(t, s, 1));

  if (*t->name == 's')
  {
    return unescape(fabr_tree_string(s, t));
  }

  if (*t->name == 'd')
  {
    char *d = expand(s, t->child->sibling, 0, data, func);
    char *dd = eval(d, data, func);
    free(d);
    if (quote && dd == NULL) return strdup("\"\"");
    if (quote) { d = fun_quote("q", dd); free(dd); return d; }
    return dd;
  }

  // 'r' or 'p'

  flu_sbuffer *b = flu_sbuffer_malloc();

  for (fabr_tree *c = t->child; c != NULL; c = c->sibling)
  {
    fabr_tree *cc = c->child;
    char *r = expand(s, cc, quote, data, func);
    if (r) { flu_sbputs(b, r); free(r); }
  }

  return flu_sbuffer_to_string(b);
}

static char *do_expand(const char *s, int quote, void *data, fdol_lookup *func)
{
  if (strchr(s, '$') == NULL) return strdup(s);

  if (fdol_parser == NULL) fdol_parser_init();

  fabr_tree *t = fabr_parse_all(s, 0, fdol_parser);
  //puts(fabr_tree_to_string(t, s, 1));
  char *r = expand(s, t, quote, data, func);
  fabr_tree_free(t);

  return r;
}

char *fdol_expand(const char *s, void *data, fdol_lookup *func)
{
  return do_expand(s, 0, data, func);
}

char *fdol_quote_expand(const char *s, void *data, fdol_lookup *func)
{
  return do_expand(s, 1, data, func);
}

char *fdol_dlup(void *data, const char *path)
{
  char *r = flu_list_get(data, path);

  return r ? strdup(r) : NULL;
}

//commit 647ebfb04cb9f4a4ebfe2073ec7c6bb6a7228b3c
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Fri Jan 9 14:37:59 2015 +0900
//
//    upgrade dependencies
