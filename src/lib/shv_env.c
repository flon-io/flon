
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

#include "flutil.h"
#include "shv_protected.h"


fshv_env *fshv_env_malloc(char *req_head, flu_dict *conf)
{
  fshv_env *r = calloc(1, sizeof(fshv_env));
  r->bag = flu_list_malloc();
  r->req = fshv_parse_request_head(req_head);
  r->res = fshv_response_malloc();
  r->conf = conf;

  return r;
}

fshv_env *fshv_env_malloc_f(char *req_head, flu_dict *conf, ...)
{
  va_list ap; va_start(ap, conf);
  char *s = flu_svprintf(req_head, ap);
  va_end(ap);

  fshv_env *r = fshv_env_malloc(s, conf);

  free(s);

  return r;
}

fshv_env *fshv_env_malloc_x(char *req_head, ...)
{
  va_list ap; va_start(ap, req_head);
  char *s = flu_svprintf(req_head, ap);
  flu_dict *conf = va_arg(ap, flu_dict *);
  va_end(ap);

  fshv_env *r = fshv_env_malloc(s, conf);

  free(s);

  return r;
}

void fshv_env_free(fshv_env *e)
{
  if (e == NULL) return;

  e->conf = NULL; // simply unlink
  fshv_request_free(e->req);
  fshv_response_free(e->res);
  flu_list_free_all(e->bag);
  free(e);
}

//commit 4f600185cfdd86e14d35ea326de3121ffa4ea561
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Oct 18 15:19:12 2015 +0900
//
//    implement fshv_malloc_x()
