
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

#include <string.h>
#include <stdlib.h>

#include "shv_protected.h"


flu_list *store = NULL;


static fshv_session *stop_session(const char *sid)
{
  for (flu_node *fn = store->first; fn; fn = fn->next)
  {
    fshv_session *s = fn->item;
    if (strcmp(s->sid, sid) == 0) { s->used = 1; break; }
  }

  return NULL;
}

static fshv_session *reset_store()
{
  flu_list_and_items_free(store, (void (*)(void *))fshv_session_free);
  store = NULL;

  return NULL;
}

static fshv_session *query_session(const char *sid, long long nowus)
{
  size_t count = 0;
  flu_node *last = NULL;

  for (flu_node *fn = store->first; fn; fn = fn->next)
  {
    fshv_session *s = fn->item;

    if (s->expus <= nowus) break;
    ++count; last = fn;

    if (strcmp(s->sid, sid) != 0) continue;

    if (s->used == 1) return NULL;
    return s;
  }

  if (last == store->last) return NULL;

  // hit the expiry limit... clean

  flu_node *next = last ? last->next : store->first;

  // truncate store

  store->size = count;
  if (last) last->next = NULL;
  store->last = last;
  if (count == 0) store->first = NULL;

  // free expired sessions

  flu_node *nxt = NULL;

  for (flu_node *fn = next; fn; fn = nxt)
  {
    nxt = fn->next;
    fshv_session_free(fn->item);
    flu_node_free(fn);
  }

  return NULL;
}

static fshv_session *query_by_id(const char *id)
{
  for (flu_node *fn = store->first; fn; fn = fn->next)
  {
    fshv_session *s = fn->item;
    if (strcmp(s->id, id) == 0) return s;
  }
  return NULL;
}

static fshv_session *start_session(
  const char *sid, const char *user, const char *id, long long expus)
{
  fshv_session *old = query_by_id(id);
  if (old) old->used = 1;

  fshv_session *new = calloc(1, sizeof(fshv_session));
  new->user = strdup(user);
  new->id = strdup(id);
  new->sid = strdup(sid);
  new->expus = expus;

  flu_list_unshift(store, new);

  return new;
}

fshv_session *fshv_session_memstore_push(
  const char *sid, const char *user, const char *id, long long tus)
{
  if (store == NULL) store = flu_list_malloc();

  if (tus == -1 && sid != NULL) return stop_session(sid);
  if (tus == -1) return reset_store();
  if (user == NULL) return query_session(sid, tus);
  if (id) return start_session(sid, user, id, tus);

  return NULL;
}

flu_list *fshv_session_memstore()
{
  return store;
}

//commit 6da902f0b1b923f6e0da7c4881ef323c9ce03011
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Mon Jan 5 07:04:24 2015 +0900
//
//    adapt no_auth() to new fshv_autenticate() sig
