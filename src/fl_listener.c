
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
#include <dirent.h>

#include "flutim.h"
#include "gajeta.h"
#include "djan.h"
#include "shervin.h"
#include "shv_protected.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_listener.h"


#define FLON_RELS "http://flon.io/rels.html"

static void expand_rels(fdja_value *doc)
{
  // add FLON_RELS to #links

  if (fdja_l(doc, "_links") == NULL) fdja_set(doc, "_links", fdja_v("{}"));

  for (fdja_value *v = fdja_l(doc, "_links")->child; v; v = v->sibling)
  {
    if (*v->key != '#') continue;
    char *s = flu_sprintf("%s%s", FLON_RELS, v->key);
    free(v->key);
    v->key = s;
  }
}

static fdja_value *link(shv_request *req, char meth, const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *p = flu_svprintf(path, ap);
  va_end(ap);

  char *uri = shv_abs(0, req->uri_d);
  char *i = strstr(uri, "/i/");
  if (i) *(i + 2) = 0;

  fdja_value *r = fdja_v("{ href: \"%s/%s\" }", uri, p);

  if (meth != 'g') fdja_psetv(r, "method", shv_char_to_method(meth));
  if (strchr(p, '{')) fdja_psetv(r, "templated", "true");

  free(uri);
  free(p);

  return r;
}

static int respond(shv_request *req, shv_response *res, fdja_value *r)
{
  flu_list_set(
    res->headers, "content-type", strdup("application/json; charset=UTF-8"));

  fdja_set(r, "tstamp", fdja_sym(flu_tstamp(NULL, 1, 's')));

  expand_rels(r);

  // add home and self links

  char *uri = shv_abs(0, req->uri_d);
  fdja_pset(r, "_links.self", fdja_v("{ href: \"%s\" }", uri));
  if (req->method != 'g') {
    fdja_psetv(r, "_links.self.method", shv_char_to_method(req->method));
  }
  free(uri);

  fdja_pset(r, "_links.home", link(req, 'g', ""));

  flu_list_add(res->body, fdja_to_json(r));

  fdja_free(r);

  return 1;
}

static void in_handle_launch(
  shv_request *req, fdja_value *v, char *dom,
  shv_response *res, fdja_value *r)
{
  if ( ! flon_may_r('l', req, dom))
  {
    res->status_code = 403;
    fdja_set(r, "message", fdja_s("not authorized to launch in that domain"));

    return;
  }

  char *i = flon_generate_exid(dom);

  fdja_set(v, "exid", fdja_s(i));

  if (flon_lock_write(v, "var/spool/dis/exe_%s.json", i) != 1)
  {
    res->status_code = 500;
    fdja_set(r, "message", fdja_s("couldn't pass msg to dispatcher"));
  }
  else
  {
    fdja_set(r, "exid", fdja_s(i));
    fdja_set(r, "message", fdja_s("launched"));

    fdja_pset(r, "_links.#execution", link(req, 'g', "executions/%s", i));
  }

  free(i);
}

int flon_in_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  char *dom = NULL;

  res->status_code = 200;
  fdja_value *r = fdja_v("{ message: ok, _links: {} }");

  // handle incoming message

  fdja_value *v = NULL;
  if (req->body) v = fdja_parse(req->body);
  if (v) v->sowner = 0; // since the string is owned by the req

  if (v == NULL || v->type != 'o')
  {
    res->status_code = 400;
    fdja_set(r, "message", fdja_s("not json"));
    goto _respond;
  }

  //flu_putf(fdja_todc(v));

  dom = fdja_ls(v, "domain", NULL);
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
    fdja_psetv(r, "message", "payload must be a json object");
    goto _respond;
  }

  if (dom && exe && exe->type == 'a' && pl && exid == NULL && nid == NULL)
  {
    in_handle_launch(req, v, dom, res, r); goto _respond;
  }

  // no fit, reject...

  res->status_code = 400;
  fdja_psetv(r, "message", "rejected");
  goto _respond;

_respond:

  free(dom);
  if (v) fdja_free(v);

  return respond(req, res, r);
}

//
// /i/executions

// TODO ?archived=true or =1 or =yes
//  OR
//      ?archive=true or =1 or =yes

static void add_execution_dirs(flu_list *l, const char *path, const char *dom)
{
  char *d0 = flu_sprintf("%s/%s", path, dom);
  DIR *dir0 = opendir(d0);
  struct dirent *ep0; while ((ep0 = readdir(dir0)) != NULL)
  {
    if (*ep0->d_name == '.' || ep0->d_type != 4) continue;
    char *d1 = flu_sprintf("%s/%s", d0, ep0->d_name);
    DIR *dir1 = opendir(d1);
    struct dirent *ep1; while ((ep1 = readdir(dir1)) != NULL)
    {
      if (*ep1->d_name == '.' || ep1->d_type != 4) continue;
      flu_list_add(l, flu_path("%s/%s", d1, ep1->d_name));
    }
    closedir(dir1);
    free(d1);
  }
  closedir(dir0);
  free(d0);
}

flu_list *flon_list_executions(
  const char *user, const char *path, const char *dom)
{
  flu_list *r = flu_list_malloc();

  DIR *dir = opendir(path);

  struct dirent *ep; while ((ep = readdir(dir)) != NULL)
  {
    if (*ep->d_name == '.' || ep->d_type != 4) continue;

    if (dom && ! flon_is_subdomain(dom, ep->d_name)) continue;

    if (flon_may('r', user, ep->d_name))
    {
      add_execution_dirs(r, path, ep->d_name);
    }
  }

  closedir(dir);

  return r;
}

static fdja_value *embed_exe(shv_request *req, const char *path, fdja_value *r)
{
  if (r == NULL) r = fdja_v("{}");
  if (fdja_l(r, "_links") == NULL) fdja_set(r, "_links", fdja_v("{}"));

  char *exid = strrchr(path, '/');
  if (exid == NULL) return r;
  exid = exid + 1;

  fdja_psetv(r, "exid", exid);

  fdja_pset(r, "_links.self", link(req, 'g', "executions/%s", exid));
  fdja_pset(r, "_links.#log", link(req, 'g', "executions/%s/log", exid));
  fdja_pset(r, "_links.#msg-log", link(req, 'g', "executions/%s/msg-log", exid));
  fdja_pset(r, "_links.#msgs", link(req, 'g', "executions/%s/msgs", exid));

  expand_rels(r);

  return r;
}

static int list_exes(
  shv_request *req, shv_response *res, flu_dict *params, const char *dom)
{
  res->status_code = 200;
  fdja_value *r = fdja_v("{ _links: {}, _embedded: { executions: [] } }");

  // list running executions

  flu_list *l =
    flon_list_executions(
      flu_list_get(req->routing_d, "_user"), "var/run", dom);

  for (flu_node *n = l->first; n; n = n->next)
  {
    fdja_pset(
      r, "_embedded.executions.]", embed_exe(req, (char *)n->item, NULL));
  }

  // return result

  int result = respond(req, res, r);

  flu_list_free_all(l);

  return result;
}

int flon_exes_handler(
  shv_request *req, shv_response *res, flu_dict *params)
{
  return list_exes(req, res, params, NULL);
}

//
// /i/executions/:domain or /:exid

static int exe_handler_dom(
  shv_request *req, shv_response *res, flu_dict *params, const char *dom)
{
  //if ( ! flon_may_r('r', req, dom)) return 0;
    // the user might be allowed to see some subdomain of dom, so let it go

  return list_exes(req, res, params, dom);
}

static int exe_handler_exid(
  shv_request *req, shv_response *res, fdja_value *nid)
{
  char *dom = fdja_ls(nid, "domain", NULL);
  char *path = NULL;

  if ( ! flon_may_r('r', req, dom)) goto _over;

  path = flon_nid_path(nid);

  fdja_value *r = fdja_parse_f("var/run/%s/run.json", path);

  if (r == NULL) {
    fgaj_r("couldn't read var/run/%s/run.json", path);
    goto _over;
  }

  res->status_code = 200;
  embed_exe(req, path, r);

_over:

  free(dom);
  free(path);

  if (res->status_code == 200) return respond(req, res, r);
  return 0; // goes 404
}

int flon_exe_handler(
  shv_request *req, shv_response *res, flu_dict *params)
{
  char *id = flu_list_get(req->routing_d, "id");
  fdja_value *vid = flon_parse_nid(id);

  if (vid == NULL) return exe_handler_dom(req, res, params, id);

  int r = exe_handler_exid(req, res, vid);

  fdja_free(vid);

  return r;
}

//
// /i/executions/:exid/log /msg-log /msgs

static int sub_handler_msgs(
  shv_request *req, shv_response *res, flu_dict *params, char *exid, char *sub)
{
  char *pa = flon_exid_path(exid);
  char *d = flu_path("var/run/%s/processed", pa);
  free(pa);

  DIR *dir = opendir(d);

  if (dir == NULL) { fgaj_r("couldn't read %s", d); free(d); return 0; }

  res->status_code = 200;
  fdja_value *r = fdja_v("{ _links: {}, _embedded: { msgs: [] } }");

  struct dirent *ep; while ((ep = readdir(dir)) != NULL)
  {
    //fgaj_i("ep: >%s< %i", ep->d_name, ep->d_type);
    if (*ep->d_name == '.' || ep->d_type != 8) continue;

    fdja_psetv(
      r, "_embedded.msgs.]",
      "{ _links: {} }");
    fdja_pset(
      r, "_embedded.msgs.-1._links.self",
      link(req, 'g', "msgs/%s", ep->d_name));
  }

  closedir(dir);
  free(d);

  return respond(req, res, r);
}

static int sub_handler_log(
  shv_request *req, shv_response *res, flu_dict *params, char *exid, char *sub)
{
  char *path = flon_exid_path(exid);

  char *file = "exe.log";
  if (strcmp(sub, "msg-log") == 0) file = "msgs.log";

  ssize_t s = shv_serve_file(res, params, "var/run/%s/%s", path, file);

  free(path);

  return s > 0;
}

int flon_exe_sub_handler(
  shv_request *req, shv_response *res, flu_dict *params)
{
  char *exid = flu_list_get(req->routing_d, "id");
  char *sub = flu_list_get(req->routing_d, "sub");

  if ( ! flon_may_r('r', req, exid)) return 0;

  return strcmp(sub, "msgs") == 0 ?
    sub_handler_msgs(req, res, params, exid, sub) :
    sub_handler_log(req, res, params, exid, sub);
}

//
// /i/msgs/:id

int flon_msg_handler(
  shv_request *req, shv_response *res, flu_dict *params)
{
  char *id = flu_list_get(req->routing_d, "id");
  char *exid = flon_parse_exid(id);

  if (exid == NULL) return 0;
  if ( ! flon_may_r('r', req, exid)) { free(exid); return 0; }

  char *path = flon_exid_path(exid);
  char *fpath = flu_path("var/run/%s/processed/%s", path, id);

  ssize_t s = shv_serve_file(res, params, fpath);

  free(path);
  free(fpath);
  free(exid);

  return s > 0;
}

//
// /i/metrics

int flon_metrics_handler(
  shv_request *req, shv_response *res, flu_dict *params)
{
  return 1;
}

//
// /i

int flon_i_handler(shv_request *req, shv_response *res, flu_dict *params)
{
  res->status_code = 200;
  fdja_value *r = fdja_v("{ _links: {} }");

  fdja_pset(r, "_links.#in", link(req, 'p', "in"));

  fdja_pset(
    r, "_links.#executions", link(req, 'g', "executions"));
  fdja_pset(
    r, "_links.#domain-executions", link(req, 'g', "executions/{domain}"));
  fdja_pset(
    r, "_links.#execution", link(req, 'g', "executions/{exid}"));
  fdja_pset(
    r, "_links.#metrics", link(req, 'g', "metrics"));

  return respond(req, res, r);
}

