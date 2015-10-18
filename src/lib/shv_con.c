
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

#include "gajeta.h"
#include "shv_protected.h"


//
// fshv_con

static void con_reset(fshv_con *c)
{
  flu_sbuffer_free(c->head);
  c->head = NULL;
  c->hend = 0;

  flu_sbuffer_free(c->body);
  c->body = NULL;
  c->blen = 0;

  fshv_env_free(c->env);
  c->env = NULL;
}

fshv_con *fshv_con_malloc(
  struct sockaddr_in *client, fshv_handler *handler, flu_dict *conf)
{
  fshv_con *c = calloc(1, sizeof(fshv_con));

  c->client = client;
  //c->startus = flu_gets('u');
  c->handler = handler;
  c->conf = conf;
  con_reset(c);
  c->req_count = -1;

  return c;
}

void fshv_con_reset(fshv_con *c)
{
  fgaj_d("con %p", c);

  con_reset(c);
}

void fshv_con_free(fshv_con *c)
{
  fgaj_d("con %p", c);

  if (c == NULL) return;

  con_reset(c);
  free(c->client);
  free(c);
}

//commit 4f600185cfdd86e14d35ea326de3121ffa4ea561
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Oct 18 15:19:12 2015 +0900
//
//    implement fshv_malloc_x()
