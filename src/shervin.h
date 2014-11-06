
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

// shervin.h

#ifndef FLON_SHERVIN_H
#define FLON_SHERVIN_H

#include "flutil.h"


#define SHV_VERSION "1.0.0"

// request

typedef struct shv_request {
  long long startus; // microseconds since the Epoch
  char method;
  char *uri;
  flu_dict *uri_d;
  flu_dict *headers;
  char *body;
  short status_code; // 4xx code set by shervin, 200 else
} shv_request;

// response

typedef struct shv_response {
  short status_code; // 200, 404, 500, ...
  flu_dict *headers;
  flu_list *body;
} shv_response;

// route

typedef int shv_handler(
  shv_request *req, flu_dict *rod, shv_response *res, flu_dict *params);

typedef struct shv_route {
  shv_handler *guard;
  shv_handler *handler;
  flu_dict *params;
} shv_route;

shv_route *shv_route_malloc(shv_handler *guard, shv_handler *handler, ...);
#define shv_r(...) shv_route_malloc(__VA_ARGS__)

shv_route *shv_rp(char *path, shv_handler *handler, ...);


// guards

/* Merely a marker function, corresponding handlers are called as filters.
 */
int shv_filter_guard(
  shv_request *req, flu_dict *rod, shv_response *res, flu_dict *params);

int shv_any_guard(
  shv_request *req, flu_dict *rod, shv_response *res, flu_dict *params);
int shv_path_guard(
  shv_request *req, flu_dict *rod, shv_response *res, flu_dict *params);

// handlers

int shv_dir_handler(
  shv_request *req, flu_dict *guard, shv_response *res, flu_dict *params);

// serving

void shv_serve(int port, shv_route **routes);
  // NULL terminated route array

// TODO: shv_stop_serve?

#endif // FLON_SHERVIN_H

