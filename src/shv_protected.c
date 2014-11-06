
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

#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "flutim.h"
#include "shv_protected.h"


//
// request

char shv_method_to_char(char *s)
{
  if (strncmp(s, "GET", 3) == 0) return 'g';
  if (strncmp(s, "PUT", 3) == 0) return 'u';
  if (strncmp(s, "POST", 4) == 0) return 'p';
  if (strncmp(s, "HEAD", 4) == 0) return 'h';
  if (strncmp(s, "TRACE", 5) == 0) return 't';
  if (strncmp(s, "DELETE", 6) == 0) return 'd';
  if (strncmp(s, "OPTIONS", 7) == 0) return 'o';
  if (strncmp(s, "CONNECT", 7) == 0) return 'c';
  return '?';
}

char *shv_char_to_method(char c)
{
  if (c == 'g') return "GET";
  if (c == 'u') return "PUT";
  if (c == 'p') return "POST";
  if (c == 'h') return "HEAD";
  if (c == 't') return "TRACE";
  if (c == 'd') return "DELETE";
  if (c == 'o') return "OPTIONS";
  if (c == 'c') return "CONNECT";
  return "???";
}


//
// response


//
// connection

shv_con *shv_con_malloc(struct sockaddr_in *client, shv_route **routes)
{
  shv_con *c = calloc(1, sizeof(shv_con));
  c->client = client;
  c->startus = flu_gets('u');
  c->routes = routes;
  shv_con_reset(c);
  c->rqount = -1;
  return c;
}

void shv_con_reset(shv_con *c)
{
  if (c->head) flu_sbuffer_free(c->head);
  c->head = NULL;
  c->hend = 0;

  if (c->body) flu_sbuffer_free(c->body);
  c->body = NULL;
  c->blen = 0;

  if (c->req) shv_request_free(c->req);
  c->req = NULL;

  if (c->res) shv_response_free(c->res);
  c->res = NULL;
}

void shv_con_free(shv_con *c)
{
  shv_con_reset(c);
  free(c->client);
  free(c);
}

