
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

#include <stdlib.h>
#include <string.h>

#include "djan.h"
#include "shervin.h"
#include "shv_protected.h"
#include "fl_ids.h"


#define FLON_RELS "http://flon.io/rels.html"


static int respond(shv_response *res, fdja_value *r)
{
  flu_list_set(
    res->headers, "content-type", strdup("application/json; charset=UTF-8"));

  // add FLON_RELS to #links

  for (fdja_value *v = fdja_l(r, "_links")->child; v; v = v->sibling)
  {
    if (*v->key != '#') continue;
    char *s = flu_sprintf("%s%s", FLON_RELS, v->key);
    free(v->key);
    v->key = s;
  }

  flu_list_add(res->body, fdja_to_json(r));

  fdja_free(r);

  return 1;
}

static void in_handle_launch(
  shv_request *req, fdja_value *v, char *dom,
  shv_response *res, fdja_value *r)
{
    char *i = flon_generate_exid(dom);
      // TODO: fetch domain from body and/or domain[s].json

    fdja_set(v, "exid", fdja_s(i));

    if (fdja_to_json_f(v, "var/spool/dis/exe_%s.json", i) != 1)
    {
      res->status_code = 500;
      fdja_set(r, "message", fdja_s("couldn't pass msg to dispatcher"));
    }
    else
    {
      fdja_set(r, "exid", fdja_s(i));
      fdja_set(r, "message", fdja_s("launched"));

      char *s = shv_rel(0, req->uri_d, "./execution/%s", i);
      fdja_pset(r, "_links.#execution", fdja_s(s));
      free(s);
    }

    free(i);
}

int flon_in_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  res->status_code = 200;
  fdja_value *r = fdja_v("{ message: ok, _links: {} }");

  // handle incoming message

  //if (req->body) flu_writeall("var/spool/dis/%s", "x.json", req->body);

  fdja_value *v = fdja_parse(req->body);
  if (v) v->sowner = 0; // since the string is owned by the req

  if (v == NULL || v->type != 'o')
  {
    res->status_code = 400;
    fdja_set(r, "message", fdja_s("not json"));
    goto _respond;
  }

  //flu_putf(fdja_todc(v));

  char *dom = fdja_ls(v, "domain", NULL);
  //
  fdja_value *exe = fdja_l(v, "execute");
  fdja_value *inv = fdja_l(v, "invoke");
  fdja_value *rec = fdja_l(v, "receive");
  //
  fdja_value *pl = fdja_l(v, "payload");
  //
  fdja_value *exid = fdja_l(v, "exid");
  fdja_value *nid = fdja_l(v, "nid");

  if (pl && pl->type != 'o')
  {
    res->status_code = 400;
    fdja_psetf(r, "message", "payload must be a json object");
    goto _respond;
  }

  if (dom && exe && exe->type == 'a' && pl && exid == NULL && nid == NULL)
  {
    in_handle_launch(req, v, dom, res, r); goto _respond;
  }

  // no fit, reject...

  res->status_code = 400;
  fdja_psetf(r, "message", "rejected");
  goto _respond;

_respond:

  if (v) fdja_free(v);

  char *s = NULL;

  s = shv_abs(0, req->uri_d);
  fdja_pset(r, "_links.self", fdja_v("{ href: \"%s\", method: POST }", s));
  free(s);

  s = shv_rel(0, req->uri_d, "..");
  fdja_pset(r, "_links.home", fdja_v("{ href: \"%s\" }", s));
  free(s);

  return respond(res, r);
}

int flon_i_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  char *s = NULL;

  res->status_code = 200;
  fdja_value *r = fdja_v("{ _links: {} }");

  s = shv_abs(0, req->uri_d);
  fdja_pset(r, "_links.self", fdja_v("{ href: \"%s\" }", s));
  fdja_pset(r, "_links.home", fdja_v("{ href: \"%s\" }", s));
  free(s);

  s = shv_rel(0, req->uri_d, "in");
  fdja_pset(
    r, "_links.#in",
    fdja_v("{ href: \"%s\", method: POST }", s));
  free(s);

  s = shv_rel(0, req->uri_d, "executions");
  fdja_pset(
    r, "_links.#executions",
    fdja_v("{ href: \"%s\", templated: true }", s));
  free(s);

  s = shv_rel(0, req->uri_d, "executions/{domain}");
  fdja_pset(
    r, "_links.#domain-executions",
    fdja_v("{ href: \"%s\", templated: true }", s));
  free(s);

  s = shv_rel(0, req->uri_d, "executions/{exid}");
  fdja_pset(
    r, "_links.#execution",
    fdja_v("{ href: \"%s\", templated: true }", s));
  free(s);

  s = shv_rel(0, req->uri_d, "metrics");
  fdja_pset(
    r, "_links.#metrics",
    fdja_v("{ href: \"%s\" }", s));
  free(s);

  return respond(res, r);
}

