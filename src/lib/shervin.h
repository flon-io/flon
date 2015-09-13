
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


#define FSHV_VERSION "1.1.0"

#define FSHV_BUFFER_SIZE 4096


//
// request

typedef struct {
  char *scheme;
  char *host;
  int port;
  char *path;
  char *query;
  char *fragment;
  flu_dict *qentries;
} fshv_uri;

typedef struct {
  long long startus; // microseconds since the Epoch
  char method;
  char *u; // raw uri
  fshv_uri *uri;
  flu_dict *headers;
  char *body;
} fshv_request;


//
// response

typedef struct {
  short status_code; // 200, 404, 500, ...
  flu_dict *headers;
  flu_list *body;
} fshv_response;

char *fshv_response_body_to_s(fshv_response *res);


//
// env

typedef struct {
  flu_dict *conf; // application level conf
  fshv_request *req;
  fshv_response *res;
  flu_dict *bag; // req-res scoped databag
} fshv_env;


//
// [root] handler

typedef int fshv_handler(fshv_env *env);


//
// handlers

int fshv_serve_files(fshv_env *env, char *root);
int fshv_mirror(fshv_env *env, short do_log);

int fshv_status(fshv_env *env, int status);


//
// basic auth

void fshv_set_user(fshv_env *e, const char *realm, const char *user);
char *fshv_get_user(fshv_env *e, const char *realm);

/* Given a realm, a user and a pass, returns the (new) username in case
 * of valid user/pass combination or NULL instead.
 * Generally the same username is given back, but who knows, certain
 * systems might be more byzantine.
 *
 * Warning: the returned string must be a newly malloced string
 * (usually strdup(user)), else you're in for memory issues.
 */
typedef char *fshv_user_pass_authentifier(
  fshv_env *e, const char *realm, const char *user, const char *pass);

/* Vanilla basic authentication implementation.
 * Returns 1 in case of successful authentication, 0 else.
 */
int fshv_basic_auth(
  fshv_env *e, const char *realm, fshv_user_pass_authentifier *a);


//
// session auth

typedef struct {
  char *sid;
  char *user;
  char *id;
  long long expus; // microseconds, expiration point
  short used;
} fshv_session;

char *fshv_session_to_s(fshv_session *s);

void fshv_session_free(fshv_session *s);

/* * pushing with all the parameters set and expiry time:
 *   start or refreshes a session
 *   returns the new session in case of success, NULL else
 * * pushing with only the sid set and now:
 *   queries and expires,
 *   returns a session in case of success, NULL else
 * * pushing with only the sid set and -1:
 *   stops the session and returns NULL
 * * pushing with all NULL and -1:
 *   resets the store and returns NULL
 */
typedef fshv_session *fshv_session_push(
  fshv_env *e,
  const char *sid,
  const char *user,
  const char *id,
  long long tus);

/* Used by login endpoints to start a session.
 */
void fshv_start_session(
  fshv_env *e, fshv_session_push *p, const char *cookie_name, const char *user);

/* Used by logout endpoints to leave a session.
 */
void fshv_stop_session(
  fshv_env *env, fshv_session_push *p, const char *sid);

int fshv_session_auth(
  fshv_env *e, fshv_session_push *p, const char *cookie_name);


//
// guards

int fshv_path_match(fshv_env *env, int sub, const char *path);
#define fshv_m(env, path) fshv_path_match(env, 0, path)
#define fshv_match(env, path) fshv_path_match(env, 0, path)
#define fshv_smatch(env, path) fshv_path_match(env, 1, path)


//
// serve

void fshv_serve(int port, fshv_handler *root_handler, flu_dict *conf);
  // interfaces?


#endif // FLON_SHERVIN_H

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
