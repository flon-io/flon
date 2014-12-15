
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

#include "flutil.h"
#include "flutim.h"
#include "flu64.h"
#include "shervin.h"
#include "shv_protected.h"


//
// session (cookie) authentication

static fshv_session *fshv_session_malloc(
  char *user, char *id, char *sid, long long mtimeus)
{
  fshv_session *r = calloc(1, sizeof(fshv_session));
  if (r == NULL) return NULL;

  r->sid = sid;
  r->user = user;
  r->id = id;
  r->mtimeus = mtimeus;

  return r;
}

char *fshv_session_to_s(fshv_session *s)
{
  if (s == NULL) return strdup("(fshv_session null)");

  char *ts = flu_sstamp(s->mtimeus / 1000000, 1, 's');
  char *r = flu_sprintf(
    "(fshv_session '%s', '%s', '%s', %lli (%s))",
    s->user, s->id, s->sid, s->mtimeus, ts);
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

flu_list *session_store;

flu_list *fshv_session_store()
{
  return session_store;
}

void fshv_session_add(
  const char *user, const char *id, const char *sid, long long nowus)
{
  if (session_store == NULL) session_store = flu_list_malloc();

  flu_list_unshift(
    session_store,
    fshv_session_malloc(strdup(user), strdup(id), strdup(sid), nowus));
}

void fshv_session_store_reset()
{
  flu_list_and_items_free(session_store, (void (*)(void *))fshv_session_free);
  session_store = NULL;
}

#define SHV_SA_RANDSIZE 48

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

  return flu64_encode(
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

    if (req->startus > s->mtimeus + expus) break;

    if (strcmp(s->sid, sid) == 0) { r = s; break; }

    last = fn; ++count;
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

static void set_session_cookie(
  fshv_request *req, fshv_response *res, fshv_session *ses, long expiry)
{
  //flu_putf(flu_sstamp(ses->mtimeus / 1000000 , 1, 'g'));
  char *ts = flu_sstamp((ses->mtimeus + expiry) / 1000000 , 1, 'g');

  flu_sbuffer *b = flu_sbuffer_malloc();

  flu_sbputs(b, ses->sid);
  flu_sbputs(b, ";Expires="); flu_sbputs(b, ts);
  flu_sbputs(b, ";HttpOnly");
  if (fshv_request_is_https(req)) flu_sbputs(b, ";Secure");

  flu_list_set(res->headers, "set-cookie", flu_sbuffer_to_string(b));

  free(ts);
}

#define SHV_SA_EXPIRY (long)24 * 3600 * 1000 * 1000

int fshv_session_auth_filter(
  fshv_request *req, fshv_response *res, flu_dict *params)
{
  int r = 1; // handled (do not got to the next route)

  char *cname = flu_list_get(params, "name");
  if (cname == NULL) cname = flu_list_get(params, "n");
  if (cname == NULL) cname = "flon.io.shervin";

  char *cookies = flu_list_get(req->headers, "cookie");
  if (cookies == NULL) return r;

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

  if (s == NULL) return r;

  //flu_putf(fshv_session_to_s(s));

  r = 0; // success

  flu_list_set(req->routing_d, "_user", strdup(s->user));

  set_session_cookie(req, res, s, SHV_SA_EXPIRY);

  return r;
}

