
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
#include <string.h>
#include <unistd.h>
//#include <sys/types.h>

#include "flutil.h"
#include "flutim.h"
#include "flu64.h"
//#include "shervin.h"
#include "shv_protected.h"
#include "shv_auth_session_memstore.h"


//
// session (cookie) authentication

#define SHV_SA_RANDSIZE 48
#define SHV_SA_EXPIRY (long)24 * 3600 * 1000 * 1000

char *spid;

char *fshv_session_to_s(fshv_session *s)
{
  if (s == NULL) return strdup("(fshv_session null)");

  char *ts = flu_sstamp(s->expus / 1000000, 1, 's');
  char *r = flu_sprintf(
    "(fshv_session '%s', '%s', '%s', %lli (%s), u%i)",
    s->user, s->id, s->sid, s->expus, ts, s->used);
  free(ts);

  return r;
}

void fshv_session_free(fshv_session *s)
{
  if (s == NULL) return;

  free(s->sid);
  free(s->user);
  free(s->id);
  free(s);
}

static char *generate_sid(fshv_env *env)
{
  // bringing the params in,
  // eventually grab a pointer to another generate sid method

  char rand[SHV_SA_RANDSIZE];

  FILE *f = fopen("/dev/urandom", "r");
  if (f == NULL) return NULL;

  size_t r = fread(rand, sizeof(char), SHV_SA_RANDSIZE, f);
  if (r < SHV_SA_RANDSIZE) return NULL;

  if (fclose(f) != 0) return NULL;

  return flu64_encode_for_url(
    rand,
    SHV_SA_RANDSIZE - (env->req->startus / 1000000) % 10);
}

static void set_session_cookie(
  fshv_env *env, const char *cookie_name, fshv_session *ses)
{
  char *ts = flu_sstamp((ses->expus) / 1000000 , 1, 'g');

  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbputs(b, cookie_name); flu_sbputc(b, '='); flu_sbputs(b, ses->sid);
  flu_sbputs(b, ";Expires="); flu_sbputs(b, ts);
  flu_sbputs(b, ";HttpOnly");
  if (fshv_request_is_https(env->req)) flu_sbputs(b, ";Secure");

  flu_list_set(env->res->headers, "set-cookie", flu_sbuffer_to_string(b));

  free(ts);
}

void fshv_start_session(
  fshv_env *env,
  fshv_session_push *push_func,
  const char *cookie_name,
  const char *user)
{
  if (spid == NULL) spid = flu_sprintf("%lli_%lli", getppid(), getpid());

  flu_sbuffer *b = flu_sbuffer_malloc();
  flu_sbputs(b, user); flu_sbputc(b, ':');
  flu_sbputs(b, spid); flu_sbputc(b, ':');
  flu_sbprintf(b, "%d", env->req->uri->port);
  char *id = flu_sbuffer_to_string(b);

  char *sid = generate_sid(env);

  long long expus =
    (env->req ? env->req->startus : flu_gets('u')) + SHV_SA_EXPIRY;

  fshv_session *ses = push_func(env, sid, user, id, expus);

  set_session_cookie(env, cookie_name, ses);

  free(id);
  free(sid);
}

void fshv_stop_session(
  fshv_env *env, fshv_session_push *push_func, const char *sid)
{
  push_func(env, sid, NULL, NULL, -1);
}

int fshv_session_auth(
  fshv_env *env, fshv_session_push *push_func, const char *cookie_name)
{
  fshv_session *s = NULL;

  char *cookies = flu_list_get(env->req->headers, "cookie");
  if (cookies == NULL) goto _over;

  char *sid = NULL;
  for (char *cs = cookies; cs; cs = strchr(cs, ';'))
  {
    while (*cs == ';' || *cs == ' ') ++cs;

    char *eq = strchr(cs, '=');
    if (eq == NULL) break;

    if (strncmp(cs, cookie_name, eq - cs) != 0) continue;

    char *eoc = strchr(eq + 1, ';');
    sid = eoc ? strndup(eq + 1, eoc - eq - 1) : strdup(eq + 1);
    break;
  }
  if (sid == NULL) goto _over;

  s = push_func(env, sid, NULL, NULL, env->req->startus);
    // query

  free(sid);

  if (s == NULL) goto _over;

  sid = generate_sid(env);

  fshv_session *s1 =
    push_func(env, sid, s->user, s->id, env->req->startus + SHV_SA_EXPIRY);
      // refresh

  free(sid);

  if (s1 == NULL) goto _over;

  fshv_set_user(env, cookie_name, s1->user);

  set_session_cookie(env, cookie_name, s1);

_over:

  if (s == NULL) { env->res->status_code = 401; return 0; }

  return 1;
}

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
