
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

#define _POSIX_C_SOURCE 200809L

#include <string.h>
#include <stdlib.h>

#include "flu64.h"
#include "gajeta.h"
#include "djan.h"
#include "tsifro.h"
#include "fl_listener.h"


// TODO: when receiving SIGHUP, reload passwd.json (and domain.json?)
//       OR keep track of mtime...

static fdja_value *passwd = NULL;
static fdja_value *domain = NULL;

static char *dummy_hash =
  "$2a$08$3xtQyM06K85SCaL2u8EadeO8FwLd5M1xEb4/mtxRaArfx1MFB9/QK";


static int load_passwd()
{
  if (passwd) return 1;

  passwd = fdja_parse_obj_f("etc/passwd.json");

  if (passwd) return 1;

  fgaj_r("couldn't load etc/passwd.json");
  return 0;
}

static int load_domain()
{
  if (domain) return 1;

  domain = fdja_parse_obj_f("etc/domain.json");

  if (domain) return 1;

  fgaj_r("couldn't load etc/domain.json");
  return 0;
}

int flon_auth_enticate(char *user, char *pass)
{
  if ( ! load_passwd()) return 0;

  fdja_value *u = fdja_l(passwd, user);

  char *hash = u ? fdja_ls(u, "pass", NULL) : NULL;
  if (hash == NULL) { u = NULL; hash = dummy_hash; }

  // TODO: shouldn't the workfactor be set in the conf?

  int r = ftsi_bc_verify(pass, hash);

  if (hash != dummy_hash) free(hash);

  return u ? r : 0;
}

int flon_auth_filter(shv_request *req, shv_response *res, flu_dict *params)
{
  int r = 1;
  char *user = NULL;

  if (flu_list_get(req->uri_d, "logout")) goto _over;
    // /?logout logs out...

  char *auth = flu_list_get(req->headers, "authorization");
  if (auth == NULL) goto _over;

  if (strncmp(auth, "Basic ", 6) != 0) goto _over;

  user = flu64_decode(auth + 6, -1);
  char *pass = strchr(user, ':');
  if (pass == NULL) goto _over;

  *pass = 0; pass = pass + 1;
  if (flon_auth_enticate(user, pass) == 0) goto _over;

  r = 0; // success
  flu_list_set(req->routing_d, "_user", strdup(user));

_over:

  if (r == 1)
  {
    flu_list_set(
      res->headers, "WWW-Authenticate", strdup("Basic realm=\"flon\""));
    res->status_code = 401;
  }

  free(user);

  return r;
}

int flon_dom_matches(const char *dom, const char *pat)
{
  int r = 0;

  flu_list *d = flu_split(dom, ".");
  flu_list *p = flu_split(pat, ".");

  flu_node *dn = d->first;

  //printf("--- d>%s< vs p>%s<\n", dom, pat);

  for (flu_node *pn = p->first; pn; pn = pn->next)
  {
    if (dn == NULL)
    {
      if (strcmp(pn->item, "**") == 0 && pn->next == NULL) r = 1;
      break;
    }

    char *ds = dn->item; char *ps = pn->item;

    //printf("d>%s< vs p>%s<\n", ds, ps);

    if (strcmp(ps, "*") == 0)
    {
      dn = dn->next;
    }
    else if (strcmp(ps, "**") == 0)
    {
      if (pn->next == NULL) { r = 1; break; }
      char *stop = pn->next->item;
      while (dn->next && strcmp(dn->item, stop) != 0) dn = dn->next;
    }
    else if (strcmp(ds, ps) != 0)
    {
      break;
    }
    else
    {
      dn = dn->next;
    }

    if (pn->next == NULL && dn == NULL) r = 1;
  }

  flu_list_free_all(d);
  flu_list_free_all(p);

  return r;
}

static int flon_may(shv_request *req, char *dom, char right)
{
  if (dom == NULL) return 0;

  if ( ! load_domain()) return 0;

  char *u = flu_list_get(req->routing_d, "_user");
  if (u == NULL) return 0;

  fdja_value *doms = fdja_lookup(domain, u);
  if (doms == NULL) return 0;

  int result = 0;

  char *d = strdup(dom); char *dash = strchr(d, '-'); if (dash) *dash = 0;

  for (fdja_value *c = doms->child; c; c = c->sibling)
  {
    if ( ! flon_dom_matches(d, c->key)) continue;
    char *s = fdja_to_string(c);
    char *r = strchr(s, right);
    free(s);
    if (r) { result = 1; goto _over; }
  }

_over:

  free(d);

  return result;
}

int flon_may_read(shv_request *req, char *dom)
{
  return flon_may(req, dom, 'r');
}

int flon_may_launch(shv_request *req, char *dom)
{
  return flon_may(req, dom, 'l');
}

