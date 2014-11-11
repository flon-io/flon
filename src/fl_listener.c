
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

#include <string.h>

#include "djan.h"
#include "shervin.h"
#include "shv_protected.h"


static int respond(shv_response *res, fdja_value *val)
{
  res->status_code = 200;

  flu_list_set(
    res->headers, "content-type", strdup("application/json; charset=UTF-8"));

  flu_list_add(res->body, fdja_to_json(val));

  return 1;
}

int flon_in_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  // handle incoming message

  if (req->body) flu_writeall("var/spool/dis/%s", "x.json", req->body);

  // respond

  fdja_value *r = fdja_v("{ _links: {} }");
  fdja_value *l = fdja_l(r, "_links");

  fdja_set(
    l, "self",
    fdja_v("{ href: \"%s\", method: POST }", shv_abs(0, req->uri_d)));
  fdja_set(
    l, "home",
    fdja_v("{ href: \"%s\" }", shv_rel(0, req->uri_d, "..")));

  return respond(res, r);
}

int flon_i_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  fdja_value *r = fdja_v("{ _links: {} }");
  fdja_value *l = fdja_l(r, "_links");

  fdja_set(
    l, "self",
    fdja_v("{ href: \"%s\" }", shv_abs(0, req->uri_d)));
  fdja_set(
    l, "home",
    fdja_v("{ href: \"%s\" }", shv_abs(0, req->uri_d)));

  fdja_set(
    l, "http://flon.io/rels.html#in",
    fdja_v(
      "{ href: \"%s\", method: POST }",
      shv_rel(0, req->uri_d, "in")));

  fdja_set(
    l, "http://flon.io/rels.html#executions",
    fdja_v(
      "{ href: \"%s\", templated: true }",
      shv_rel(0, req->uri_d, "executions")));
  fdja_set(
    l, "http://flon.io/rels.html#domain-executions",
    fdja_v(
      "{ href: \"%s\", templated: true }",
      shv_rel(0, req->uri_d, "executions/{domain}")));
  fdja_set(
    l, "http://flon.io/rels.html#execution",
    fdja_v(
      "{ href: \"%s\", templated: true }",
      shv_rel(0, req->uri_d, "executions/{exid}")));

  fdja_set(
    l, "http://flon.io/rels.html#metrics",
    fdja_v("{ href: \"%s\" }", shv_rel(0, req->uri_d, "metrics")));

  return respond(res, r);
}

