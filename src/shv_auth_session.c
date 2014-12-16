
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
#include <unistd.h>
#include <sys/types.h>

#include "flutil.h"
#include "flutim.h"
#include "flu64.h"
#include "shervin.h"
#include "shv_protected.h"


//
// session (cookie) authentication

#define SHV_SA_RANDSIZE 48
#define SHV_SA_EXPIRY (long)24 * 3600 * 1000 * 1000

flu_list *session_store;
char *spid;

static fshv_session *fshv_session_malloc(
  char *user, char *id, char *sid, long long mtimeus)
{
  fshv_session *r = calloc(1, sizeof(fshv_session));
  if (r == NULL) return NULL;

  r->sid = sid;
  r->user = user;
  r->id = id;
  r->mtimeus = mtimeus;
  r->used = 0;

  return r;
}

char *fshv_session_to_s(fshv_session *s)
{
  if (s == NULL) return strdup("(fshv_session null)");

  char *ts = flu_sstamp(s->mtimeus / 1000000, 1, 's');
  char *r = flu_sprintf(
    "(fshv_session '%s', '%s', '%s', %lli (%s), u%i)",
    s->user, s->id, s->sid, s->mtimeus, ts, s->used);
  free(ts);

  return r;
}

static void fshv_session_free(fshv_session *s)
{
  if (s == NULL) return;

  free(s->sid);
  free(s->user);
  free(s->id);
  free(s);
}

flu_list *fshv_session_store()
{
  return session_store;
}

char *fshv_session_store_to_s()
{
  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbputc(b, '{');
  for (flu_node *fn = session_store->first; fn; fn = fn->next)
  {
    flu_sbputs(b, "\n  * ");
    char *s = fshv_session_to_s(fn->item); flu_sbputs(b, s); free(s);
  }
  flu_sbputs(b, "\n}");

  return flu_sbuffer_to_string(b);
}

fshv_session *fshv_session_add(
  const char *user, const char *id, const char *sid, long long nowus)
{
  if (session_store == NULL) session_store = flu_list_malloc();

  fshv_session *ses =
    fshv_session_malloc(strdup(user), strdup(id), strdup(sid), nowus);

  flu_list_unshift(session_store, ses);

  return ses;
}

void fshv_session_store_reset()
{
  flu_list_and_items_free(session_store, (void (*)(void *))fshv_session_free);
  session_store = NULL;
}

static char *generate_sid(fshv_request *req, flu_dict *params)
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
    SHV_SA_RANDSIZE - (req->startus / 1000000) % 10);
}

static fshv_session *lookup_session(
  fshv_request *req, flu_dict *params, const char *sid, long expus)
{
  fshv_session *r = NULL;

  if (session_store == NULL) session_store = flu_list_malloc();

  size_t count = 0;
  flu_node *last = NULL;

  for (flu_node *fn = session_store->first; fn; fn = fn->next)
  {
    fshv_session *s = fn->item;

    if (expus > 0 && req->startus > s->mtimeus + expus) break;
    if (s->used) continue;

    if (strcmp(s->sid, sid) == 0) { r = s; break; }

    last = fn; ++count;
  }

  if (expus == 0)
  {
    if (r) r->used = 1;

    return NULL;
  }

  if (r)
  {
    char *sid = generate_sid(req, params);
    if (sid == NULL) sid = strdup(r->sid);

    if (session_store->first->item == r)
    {
      free(r->sid); r->sid = sid;
      r->mtimeus = req->startus;
    }
    else
    {
      fshv_session *s =
        fshv_session_malloc(strdup(r->id), strdup(r->user), sid, req->startus);

      flu_list_unshift(session_store, s);

      r->used = 1;

      r = s;
    }

    return r;
  }

  // TODO enventually let clean up before returning found session

  session_store->size = count;
  session_store->last = last;
  if (last == NULL) session_store->first = NULL;

  for (flu_node *fn = last, *next = NULL; fn; fn = next)
  {
    next = fn->next;
    fshv_session_free(fn->item);
    flu_node_free(fn);
  }

  return NULL;
}

static char *get_cookie_name(flu_dict *params)
{
  char *r = flu_list_get(params, "name");
  if (r == NULL) r = flu_list_get(params, "n");
  if (r == NULL) r = "flon.io.shervin";

  return r;
}

static void set_session_cookie(
  fshv_request *req, fshv_response *res, flu_dict *params,
  fshv_session *ses, long expiry)
{
  char *cn = get_cookie_name(params);
  char *ts = flu_sstamp((ses->mtimeus + expiry) / 1000000 , 1, 'g');

  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbputs(b, cn); flu_sbputc(b, '='); flu_sbputs(b, ses->sid);
  flu_sbputs(b, ";Expires="); flu_sbputs(b, ts);
  flu_sbputs(b, ";HttpOnly");
  if (fshv_request_is_https(req)) flu_sbputs(b, ";Secure");

  flu_list_set(res->headers, "set-cookie", flu_sbuffer_to_string(b));

  free(ts);
}

void fshv_start_session(
  fshv_request *req, fshv_response *res, flu_dict *params, const char *user)
{
  if (spid == NULL) spid = flu_sprintf("%lli_%lli", getppid(), getpid());

  flu_sbuffer *b = flu_sbuffer_malloc();
  flu_sbputs(b, user); flu_sbputc(b, ':');
  flu_sbputs(b, spid); flu_sbputc(b, ':');
  flu_sbputs(b, flu_list_getd(req->uri_d, "_port", "80"));
  char *id = flu_sbuffer_to_string(b);

  char *sid = generate_sid(req, params);

  fshv_session *ses = fshv_session_add(user, id, sid, req->startus);

  set_session_cookie(req, res, params, ses, SHV_SA_EXPIRY);
}

void fshv_stop_session(
  fshv_request *req, fshv_response *res, flu_dict *params, const char *sid)
{
  lookup_session(req, params, sid, 0);
    // 0 forces to forget the session
}

int fshv_session_auth_filter(
  fshv_request *req, fshv_response *res, int mode, flu_dict *params)
{
  int authentified = 0;

  char *cookies = flu_list_get(req->headers, "cookie");
  if (cookies == NULL) goto _over;

  char *cname = get_cookie_name(params);

  char *sid = NULL;
  for (char *cs = cookies; cs; cs = strchr(cs, ';'))
  {
    while (*cs == ';' || *cs == ' ') ++cs;

    char *eq = strchr(cs, '=');
    if (eq == NULL) break;

    if (strncmp(cs, cname, eq - cs) != 0) continue;

    char *eoc = strchr(eq + 1, ';');
    sid = eoc ? strndup(eq + 1, eoc - eq - 1) : strdup(eq + 1);
    break;
  }

  fshv_session *s = lookup_session(req, params, sid, SHV_SA_EXPIRY);

  free(sid);

  if (s == NULL) goto _over;

  authentified = 1;

  fshv_set_user(req, "session", s->user);

  set_session_cookie(req, res, params, s, SHV_SA_EXPIRY);

_over:

  if ( ! authentified) res->status_code = 401;

  return 0;
}

