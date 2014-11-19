
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

// https://github.com/flon-io/shervin

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "flutil.h"
#include "flutim.h"
#include "aabro.h"
#include "shv_protected.h"


fabr_parser *request_parser = NULL;


static void shv_init_parser()
{
  fabr_parser *sp = fabr_string(" ");
  fabr_parser *crlf = fabr_string("\r\n");

  fabr_parser *lws = fabr_rex("(\r\n)?[ \t]+");

  //fabr_parser *text =
  //  fabr_alt(fabr_rex("[^\x01-\x1F\x7F]"), lws, fabr_r("+"));
  fabr_parser *text =
    fabr_rex("[^\x01-\x1F\x7F]+");

  fabr_parser *token =
    fabr_rex("[^\x01-\x1F\x7F()<>@,;:\\\\\"/[\\]?={} \t]+");

  fabr_parser *method =
    fabr_n_alt(
      "method",
      fabr_rex("GET|POST|PUT|DELETE|HEAD|OPTIONS|TRACE|CONNECT|LINK|UNLINK"),
      fabr_name("extension_method", token),
      NULL);
  fabr_parser *request_uri =
    fabr_n_rex("request_uri", "[^ \t\r\n]{1,2048}"); // arbitrary limit
  fabr_parser *http_version =
    fabr_n_rex("http_version", "HTTP/[0-9]+\\.[0-9]+");

  fabr_parser *request_line =
    fabr_seq(method, sp, request_uri, sp, http_version, crlf, NULL);

  fabr_parser *field_content =
    text;

  fabr_parser *field_name =
    fabr_name("field_name", token);
  fabr_parser *field_value =
    fabr_n_rep("field_value", fabr_alt(field_content, lws, NULL), 0, -1);

  fabr_parser *message_header =
    fabr_n_seq("message_header", field_name, fabr_string(":"), field_value, NULL);

  //fabr_parser *message_body =
  //  fabr_n_regex("message_body", "^.+"); // well, the rest

  request_parser =
    fabr_seq(
      request_line,
      fabr_seq(message_header, crlf, NULL), fabr_q("*"),
      crlf,
      //fabr_rep(message_body, 0, 1),
      NULL);
  // do not include the message_body

  //puts(fabr_parser_to_string(request_parser));
}

shv_request *shv_parse_request_head(char *s)
{
  //
  // parse

  if (request_parser == NULL) shv_init_parser();

  fabr_tree *r = fabr_parse(s, 0, request_parser);
  //fabr_tree *r = fabr_parse_f(s, 0, request_parser, ABR_F_ALL);

  //puts(fabr_tree_to_string_with_leaves(s, r));

  shv_request *req = calloc(1, sizeof(shv_request));
  req->startus = flu_gets('u');
  req->status_code = 400; // Bad Request

  if (r->result != 1) { fabr_tree_free(r); return req; }

  req->status_code = 200; // ok, for now

  fabr_tree *t = NULL;

  // method

  t = fabr_tree_lookup(r, "method");
  req->method = shv_method_to_char(fabr_tree_str(s, t));

  // uri

  t = fabr_tree_lookup(r, "request_uri");
  req->uri = fabr_tree_string(s, t);
    //
  while (1)
  {
    char *last = strrchr(req->uri, '/');
    if (last && last != req->uri && *(last + 1) == 0) *last = 0;
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

  req->uri_d =
    shv_parse_host_and_path(
      flu_list_get(req->headers, "host"),
      req->uri);

  req->routing_d =
    flu_list_malloc();

  //
  // over

  fabr_tree_free(r);

  return req;
}

ssize_t shv_request_content_length(shv_request *r)
{
  char *cl = flu_list_get(r->headers, "content-length");

  return (cl == NULL) ? -1 : atol(cl);
}

void shv_request_free(shv_request *r)
{
  if (r->uri) free(r->uri);
  if (r->uri_d) flu_list_free_all(r->uri_d);
  if (r->headers) flu_list_free_all(r->headers);
  if (r->routing_d) flu_list_free_all(r->routing_d);
  free(r);
}


//
// spec helpers (the specs of the projects using shervin)

shv_request *shv_parse_request_head_f(const char *s, ...)
{
  va_list ap; va_start(ap, s);
  char *ss = flu_svprintf(s, ap);
  va_end(ap);

  shv_request *r = shv_parse_request_head(ss);
  free(ss);

  return r;
}

int shv_do_route(char *path, shv_request *req)
{
  flu_dict *params = flu_list_malloc();
  flu_list_set(params, "path", path);

  int r = shv_path_guard(req, NULL, params);

  flu_list_free(params);

  return r;
}

