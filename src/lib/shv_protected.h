
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

#ifndef FLON_SHV_PROTECTED_H
#define FLON_SHV_PROTECTED_H

//#include <netinet/in.h>
#include <ev.h>

#include "flutil.h"
#include "shervin.h"


//
// misc

char fshv_method_to_char(char *s);
char *fshv_char_to_method(char c);

ssize_t fshv_serve_file(fshv_env *env, const char *path, ...);


//
// request

char *fshv_uri_to_s(fshv_uri *u);

fshv_request *fshv_parse_request_head(char *s);
fshv_request *fshv_parse_request_head_f(const char *s, ...);

ssize_t fshv_request_content_length(fshv_request *r);
int fshv_request_is_https(fshv_request *r);

//void fshv_handle(struct ev_loop *l, struct ev_io *eio);

fshv_uri *fshv_uri_malloc();
void fshv_uri_free(fshv_uri *u);
void fshv_request_free(fshv_request *r);


//
// response

fshv_response *fshv_response_malloc();
void fshv_response_free(fshv_response *r);

void fshv_respond(struct ev_loop *l, struct ev_io *eio);

//
// env

fshv_env *fshv_env_malloc(char *req_head, flu_dict *conf);
void fshv_env_free(fshv_env *e);


//
// con

typedef struct {

  struct sockaddr_in *client;
  long long startus;

  fshv_handler *handler;
  flu_dict *conf;

  flu_sbuffer *head;
  short hend;

  flu_sbuffer *body;
  size_t blen;

  ssize_t req_count;
  fshv_env *env;

  char *hout;
  size_t houtlen;
  size_t houtoff;

  FILE *bout;
} fshv_con;

fshv_con *fshv_con_malloc(
  struct sockaddr_in *client, fshv_handler *handler, flu_dict *conf);
void fshv_con_reset(fshv_con *c);
void fshv_con_free(fshv_con *c);


//
// uri

fshv_uri *fshv_parse_uri(char *uri);
fshv_uri *fshv_parse_host_and_path(char *host, char *path);

/* Renders the uri_d as an absolute URI. When ssl is set to 1, the
 * scheme will be "https://".
 */
char *fshv_absolute_uri(int ssl, fshv_uri *u, const char *rel, ...);

#define fshv_abs(ssl, uri) fshv_absolute_uri(ssl, uri, NULL)
#define fshv_rel(ssl, uri, ...) fshv_absolute_uri(ssl, uri, __VA_ARGS__)

#endif // FLON_SHV_PROTECTED_H

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
