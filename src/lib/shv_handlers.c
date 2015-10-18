
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

// handlers

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "shervin.h"
#include "shv_protected.h"


int fshv_serve_files(fshv_env *env, char *root)
{
  int r = 0;
  char *path = NULL;

  char *p = flu_list_get(env->bag, "**");
  if (p == NULL) p = env->req->uri->path;

  //fgaj_d("p: %s", p);

  if (strstr(p, "..")) { env->res->status_code = 403; goto _over; }

  path = flu_path("%s/%s", root, p);

  ssize_t s = fshv_serve_file(env, path);

  if (s < 0) goto _over;
  if (s == 0 && flu_list_get(env->res->headers, "fshv_file") == NULL)
  {
    char *i = flu_list_getod(env->conf, "index", "index.html");
    flu_list *is = flu_split(i, ",");

    for (flu_node *n = is->first; n; n = n->next)
    {
      char *p = flu_path("%s/%s", path, (char *)n->item);
      s = fshv_serve_file(env, p);
      free(p);
      if (s > 0) break;
    }

    flu_list_free_all(is);
  }

  r = (s > 0 || flu_list_get(env->res->headers, "fshv_file") != NULL);

_over:

  free(path);

  if (env->res->status_code < 0) env->res->status_code = 404;

  return r;
}


int fshv_mirror(fshv_env *env, short do_log)
{
  env->res->status_code = 200;
  //flu_list_set(res->headers, "content-type", "text/plain; charset=utf-8");

  char *suri = fshv_uri_to_s(env->req->uri);

  // prepare response body

  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbprintf(
    b, "%s %s HTTP/1.1\r\n",
    fshv_char_to_method(env->req->method), env->req->u);

  for (flu_node *fn = env->req->headers->first; fn; fn = fn->next)
  {
    flu_sbprintf(b, "%s: %s\r\n", fn->key, fn->item);
  }
  flu_sbprintf(b, "method: %s\r\n", fshv_char_to_method(env->req->method));
  flu_sbprintf(b, "path: %s\r\n", env->req->u);
  flu_sbprintf(b, "uri: %s\r\n", suri);
  flu_sbputs(b, "\r\n");
  if (env->req->body) flu_sbputs(b, env->req->body);

  flu_list_add(env->res->body, flu_sbuffer_to_string(b));

  // log request

  long long us = flu_gets('u') % 1000000;

  fgaj_d(
    "|%05x| %s %s HTTP/1.1\r\n",
    us, fshv_char_to_method(env->req->method), env->req->u);
  fgaj_d(
    "|%05x| uri: %s",
    us, suri);

  for (flu_node *fn = env->req->headers->first; fn; fn = fn->next)
  {
    fgaj_d("|%05x|  * %s: \"%s\"", us, fn->key, fn->item);
  }

  ssize_t l = env->req->body ? strlen(env->req->body) : -1;
  size_t maxl = 35;
  if (l > maxl)
    fgaj_d("|%05x| body: >%.*s...< l%lli", us, maxl, env->req->body, l);
  else
    fgaj_d("|%05x| body: >%s< l%lli", us, env->req->body, l);

  // done

  free(suri);

  return 1;
}

int fshv_status(fshv_env *env, int status)
{
  env->res->status_code = status;

  return 1;
}

//commit 4f600185cfdd86e14d35ea326de3121ffa4ea561
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Oct 18 15:19:12 2015 +0900
//
//    implement fshv_malloc_x()
