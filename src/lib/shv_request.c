
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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "flutil.h"
#include "flutim.h"
#include "aabro.h"
#include "shv_protected.h"


static fabr_tree *_sp(fabr_input *i) { return fabr_str(NULL, i, " "); }
static fabr_tree *_col(fabr_input *i) { return fabr_str(NULL, i, ":"); }
static fabr_tree *_crlf(fabr_input *i) { return fabr_str(NULL, i, "\r\n"); }

static fabr_tree *_lws(fabr_input *i)
{ return fabr_rex(NULL, i, "(\r\n)?[ \t]+"); }

static fabr_tree *_text(fabr_input *i)
{ return fabr_rex(NULL, i, "[^\x01-\x1F\x7F]+"); }

static fabr_tree *_token(fabr_input *i)
{ return fabr_rex(NULL, i, "[^\x01-\x1F\x7F()<>@,;:\\\\\"/[\\]?={} \t]+"); }

//  fabr_parser *method =
//    fabr_n_alt(
//      "method",
//      fabr_rex("GET|POST|PUT|DELETE|HEAD|OPTIONS|TRACE|CONNECT|LINK|UNLINK"),
//      fabr_name("extension_method", token),
//      NULL);
//
static fabr_tree *_method(fabr_input *i)
{ return fabr_rex("method", i, "GET|POST|PUT|DELETE|HEAD|OPTIONS"); }

static fabr_tree *_request_uri(fabr_input *i)
{ return fabr_rex("request_uri", i, "[^ \t\r\n]{1,2048}"); }
  // arbitrary limit

static fabr_tree *_http_version(fabr_input *i)
{ return fabr_rex("http_version", i, "HTTP/[0-9]+\\.[0-9]+"); }

static fabr_tree *_request_line(fabr_input *i)
{ return fabr_seq(NULL, i,
    _method, _sp, _request_uri, _sp, _http_version, _crlf,
    NULL); }

static fabr_tree *_fv(fabr_input *i)
{ return fabr_alt(NULL, i, _text, _lws, NULL); }

static fabr_tree *_field_value(fabr_input *i)
{ return fabr_rep("field_value", i, _fv, 0, 0); }

static fabr_tree *_field_name(fabr_input *i)
{ return fabr_rename("field_name", i, _token); }

static fabr_tree *_mh(fabr_input *i)
{ return fabr_seq("message_header", i, _field_name, _col, _field_value, NULL); }

static fabr_tree *_message_header(fabr_input *i)
{ return fabr_seq(NULL, i, _mh, _crlf, NULL); }

static fabr_tree *_request_head(fabr_input *i)
{ return fabr_seq(NULL, i,
    _request_line, _message_header, fabr_star, _crlf,
    NULL); } // does not include the message body


fshv_request *fshv_parse_request_head(char *s)
{
  //printf("fshv_parse_request_head() >[1;33m%s[0;0m<\n", s);

  //fabr_tree *tt = fabr_parse_f(s, _request_head, FABR_F_ALL);
  //printf("fshv_parse_request_head():\n"); fabr_puts_tree(tt, s, 1);
  //fabr_tree_free(tt);

  fabr_tree *r = fabr_parse_all(s, _request_head);
  //printf("fshv_parse_request_head() (pruned):\n"); fabr_puts(t, input, 3);

  if (r->result != 1) { fabr_tree_free(r); return NULL; }

  fshv_request *req = calloc(1, sizeof(fshv_request));
  //req->startus = flu_gets('u');

  fabr_tree *t = NULL;

  // method

  t = fabr_tree_lookup(r, "method");
  req->method = fshv_method_to_char(fabr_tree_str(s, t));

  // uri

  t = fabr_tree_lookup(r, "request_uri");
  req->u = fabr_tree_string(s, t);
    //
  while (1)
  {
    char *last = strrchr(req->u, '/');
    if (last && last != req->u && *(last + 1) == 0) *last = 0;
    else break;
  }
    //
    // discard final slashes/

  // version

    // reject when not 1.0 or 1.1?

  // headers

  flu_list *hs = fabr_tree_list_named(r, "message_header");

  req->headers = flu_list_malloc();
  for (flu_node *h = hs->first; h != NULL; h = h->next)
  {
    fabr_tree *th = (fabr_tree *)h->item;
    fabr_tree *tk = fabr_tree_lookup(th, "field_name");
    fabr_tree *tv = fabr_tree_lookup(th, "field_value");

    char *sk = fabr_tree_string(s, tk);
    for (char *kk = sk; *kk; ++kk) *kk = tolower(*kk);

    char *sv = fabr_tree_string(s, tv);

    flu_list_set(req->headers, sk, flu_strtrim(sv));

    free(sk);
    free(sv);
  }

  flu_list_free(hs);

  req->uri =
    fshv_parse_host_and_path(
      flu_list_get(req->headers, "host"),
      req->u);

  // over.

  fabr_tree_free(r);

  return req;
}

ssize_t fshv_request_content_length(fshv_request *r)
{
  char *cl = flu_list_get(r->headers, "content-length");

  return (cl == NULL) ? -1 : atol(cl);
}

void fshv_request_free(fshv_request *r)
{
  if (r == NULL) return;

  free(r->u);
  fshv_uri_free(r->uri);
  flu_list_free_all(r->headers);
  free(r->body);
  free(r);
}

int fshv_request_is_https(fshv_request *r)
{
  if (strcmp(r->uri->scheme, "https") == 0) return 1;

  char *s = flu_list_getd(r->headers, "forwarded", "");
  if (strstr(s, "proto=https")) return 1;

  s = flu_list_getd(r->headers, "x-forwarded-proto", "");
  if (strstr(s, "https")) return 1;

  return 0;
}


//
// spec helpers (the specs of the projects using shervin)

fshv_request *fshv_parse_request_head_f(const char *s, ...)
{
  va_list ap; va_start(ap, s);
  char *ss = flu_svprintf(s, ap);
  va_end(ap);

  fshv_request *r = fshv_parse_request_head(ss);
  free(ss);

  return r;
}

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
