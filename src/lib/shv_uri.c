
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

// https://github.com/flon-io/shervin

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "aabro.h"
#include "shv_protected.h"


static fabr_tree *_amp(fabr_input *i) { return fabr_str(NULL, i, "&"); }
static fabr_tree *_qmark(fabr_input *i) { return fabr_str(NULL, i, "?"); }
static fabr_tree *_equal(fabr_input *i) { return fabr_str(NULL, i, "="); }
static fabr_tree *_sharp(fabr_input *i) { return fabr_rex(NULL, i, "#"); }
static fabr_tree *_colslasla(fabr_input *i) { return fabr_str(NULL, i, "://"); }
static fabr_tree *_colon(fabr_input *i) { return fabr_str(NULL, i, ":"); }

static fabr_tree *_qkey(fabr_input *i)
{ return fabr_rex("qkey", i, "[^ \t=&#]+"); }

static fabr_tree *_val(fabr_input *i)
{ return fabr_rex("qval", i, "[^ \t&#]+"); }

static fabr_tree *_qval(fabr_input *i)
{ return fabr_seq(NULL, i, _equal, _val, NULL); }

static fabr_tree *_qentry(fabr_input *i)
{ return fabr_seq("qentry", i, _qkey, _qval, fabr_qmark, NULL); }

static fabr_tree *_query(fabr_input *i)
{ return fabr_eseq("query", i, _qmark, _qentry, _amp, NULL); }

static fabr_tree *_ragment(fabr_input *i)
{ return fabr_rex("fragment", i, ".+"); }

static fabr_tree *_fragment(fabr_input *i)
{ return fabr_seq(NULL, i, _sharp, _ragment, NULL); }

static fabr_tree *_path(fabr_input *i)
{ return fabr_rex("path", i, "[^\\?#]+"); }

static fabr_tree *_scheme(fabr_input *i)
{ return fabr_rex("scheme", i, "https?"); }

static fabr_tree *_host(fabr_input *i)
{ return fabr_rex("host", i, "[^:/]+"); }

static fabr_tree *_ort(fabr_input *i)
{ return fabr_rex("port", i, "[1-9][0-9]*"); }

static fabr_tree *_port(fabr_input *i)
{ return fabr_seq(NULL, i, _colon, _ort, NULL); }

static fabr_tree *_shp(fabr_input *i)
{ return fabr_seq(NULL, i,
    _scheme, _colslasla, _host, _port, fabr_qmark,
    NULL); }

static fabr_tree *_uri(fabr_input *i)
{ return fabr_seq(NULL, i,
    _shp, fabr_qmark, _path, _query, fabr_qmark, _fragment, fabr_qmark,
    NULL); }

fshv_uri *fshv_parse_uri(char *uri)
{
  //printf("fshv_parse_uri() >[1;33m%s[0;0m<\n", uri);

  //fabr_tree *tt = fabr_parse_f(uri, _uri, FABR_F_ALL);
  //printf("fshv_parse_uri():\n"); fabr_puts_tree(tt, uri, 1);
  //fabr_tree_free(tt);

  fabr_tree *r = fabr_parse_all(uri, _uri);
  //printf("fshv_parse_uri() (pruned):\n"); fabr_puts(r, uri, 3);

  fabr_tree *t = NULL;

  fshv_uri *u = fshv_uri_malloc();

  t = fabr_tree_lookup(r, "scheme");
  if (t) u->scheme = fabr_tree_string(uri, t);
  t = fabr_tree_lookup(r, "host");
  if (t) u->host = fabr_tree_string(uri, t);
  t = fabr_tree_lookup(r, "port");
  if (t) u->port = fabr_tree_llong(uri, t, 10); // base 10

  t = fabr_tree_lookup(r, "path");
  u->path = fabr_tree_string(uri, t);

  t = fabr_tree_lookup(r, "query");
  if (t) u->query = fabr_tree_string(uri, t);

  flu_list *l = fabr_tree_list_named(r, "qentry");
  for (flu_node *n = l->first; n != NULL; n = n->next)
  {
    t = fabr_tree_lookup((fabr_tree *)n->item, "qkey");
    char *k = fabr_tree_string(uri, t);

    char *v = NULL; char *vv = NULL;
    t = fabr_tree_lookup((fabr_tree *)n->item, "qval");
    if (t) { v = fabr_tree_string(uri, t); vv = flu_urldecode(v, -1); }
    else { vv = strdup(""); }

    flu_list_set(u->qentries, k, vv);

    free(k); // since flu_list_set() copies it
    free(v);
  }
  flu_list_free(l);

  t = fabr_tree_lookup(r, "fragment");
  if (t) u->fragment = fabr_tree_string(uri, t);

  fabr_tree_free(r);

  return u;
}

fshv_uri *fshv_parse_host_and_path(char *host, char *path)
{
  if (host == NULL) return fshv_parse_uri(path);

  char *s = NULL;

  if (strncmp(host, "http://", 7) == 0 || strncmp(host, "https://", 8) == 0)
    s = flu_sprintf("%s%s", host, path);
  else
    s = flu_sprintf("http://%s%s", host, path);

  fshv_uri *u = fshv_parse_uri(s);

  free(s);

  return u;
}

char *fshv_absolute_uri(int ssl, fshv_uri *u, const char *rel, ...)
{
  char *scheme = u->scheme;
  if (ssl) scheme = "https";
  if (scheme == NULL) scheme = "http";

  char *host =
    strdup(u->host ? u->host : "127.0.0.1"); // what about IPv6?

  char *port =
    (u->port > -1 && u->port != 80) ? flu_sprintf(":%d", u->port) : "";

  char *frag =
    u->fragment ? flu_sprintf("#%s", u->fragment) : "";

  char *query =
    u->query ? strdup(u->query) : "";

  char *path =
    strdup((u->path && strlen(u->path) > 0) ? u->path : "/");

  char *rl = NULL;
  if (rel)
  {
    va_list ap; va_start(ap, rel);
    rl = flu_svprintf(rel, ap);
    va_end(ap);
  }

  if (rl && *rl == '/')
  {
    free(path);
    path = rl;
  }
  else if (rl)
  {
    char *end = strrchr(path, '/');
    if (strchr(end, '.')) *end = 0;
    char *s = flu_canopath("%s/%s", path, rl);
    free(path);
    free(rl);
    path = s;
  }

  char *s = flu_sprintf(
    "%s://%s%s%s%s%s",
    scheme, host, port, path, query, frag);

  free(host);
  if (*port != 0) free(port);
  if (*query != 0) free(query);
  if (*frag != 0) free(frag);
  free(path);

  return s;
}

fshv_uri *fshv_uri_malloc()
{
  fshv_uri *u = calloc(1, sizeof(fshv_uri));
  u->port = 80;
  u->qentries = flu_list_malloc();

  return u;
}

char *fshv_uri_to_s(fshv_uri *u)
{
  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbputs(b, "(uri");
  flu_sbprintf(b, " s\"%s\"", u->scheme);
  flu_sbprintf(b, " h\"%s\"", u->host);
  flu_sbprintf(b, " p%i", u->port);
  flu_sbprintf(b, " p\"%s\"", u->path);
  if (u->query) flu_sbprintf(b, " q\"%s\"", u->query);
  if (u->fragment) flu_sbprintf(b, " f\"%s\"", u->fragment);

  if (u->query)
  {
    char *qes = flu_list_to_s(u->qentries);
    flu_sbprintf(b, " q%s", qes);
    free(qes);
  }

  flu_sbputs(b, ")");

  return flu_sbuffer_to_string(b);
}

void fshv_uri_free(fshv_uri *u)
{
  if (u == NULL) return;

  free(u->scheme);
  free(u->host);
  free(u->path);
  free(u->query);
  free(u->fragment);
  flu_list_free_all(u->qentries);
  free(u);
}

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
