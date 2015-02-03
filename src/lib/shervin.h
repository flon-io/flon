
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

// shervin.h

#ifndef FLON_SHERVIN_H
#define FLON_SHERVIN_H

#include "flutil.h"


#define FSHV_VERSION "1.0.0"

#define FSHV_BUFFER_SIZE 4096


// request

typedef struct {
  long long startus; // microseconds since the Epoch
  char method;
  char *uri;
  flu_dict *uri_d;
  flu_dict *headers;
  char *body;
  short status_code; // 4xx code set by shervin, 200 else
  flu_dict *routing_d; // used by guards to pass info to handlers
} fshv_request;

char fshv_method_to_char(char *s);
char *fshv_char_to_method(char c);

// response

typedef struct {
  short status_code; // 200, 404, 500, ...
  flu_dict *headers;
  flu_list *body;
} fshv_response;

// route

typedef int fshv_handler(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);

typedef struct {
  fshv_handler *guard;
  fshv_handler *handler;
  flu_dict *params;
} fshv_route;

fshv_route *fshv_route_malloc(fshv_handler *guard, fshv_handler *handler, ...);
#define fshv_r(...) fshv_route_malloc(__VA_ARGS__)

fshv_route *fshv_rp(char *path, fshv_handler *handler, ...);


enum // flags guards and handlers "mode"
{
  FSHV_F_NULL_GUARD = 1 << 0, // only for handlers, set when guard was NULL
  FSHV_F_HANDLED    = 1 << 1, // set when a previous "handled" the request
};

// guards

/* Merely a marker function, corresponding handlers are called as filters.
 */
int fshv_filter_guard(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);

int fshv_any_guard(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);
int fshv_path_guard(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);


// handlers

/* Used by shv_dir_handler(), public since it could get useful on its own.
 */
ssize_t fshv_serve_file(
  fshv_response *res, flu_dict *params, const char *path, ...);

int fshv_dir_handler(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);


// filters

typedef int fshv_authenticate(
  const char *user, const char *pass, fshv_request *req, flu_dict *params);

void fshv_set_user(fshv_request *req, const char *auth, const char *user);
char *fshv_get_user(fshv_request *req, const char *auth);

int fshv_basic_auth_filter(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);

int fshv_session_auth_filter(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params);

/* Used by login endpoints to start a session.
 */
void fshv_start_session(
  fshv_request *req, fshv_response *res, flu_dict *params, const char *user);

/* Used by logout endpoints to leave a session.
 */
void fshv_stop_session(
  fshv_request *req, fshv_response *res, flu_dict *params, const char *sid);


// serving

void fshv_serve(int port, fshv_route **routes);
  // NULL terminated route array

// TODO: fshv_stop_serve?

#endif // FLON_SHERVIN_H

//commit c80c5037e9f15d0e454d23cfd595b8bcc72d87a7
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Tue Jan 27 14:27:01 2015 +0900
//
//    add support for "application/pdf"
