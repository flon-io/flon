
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

int flon_auth_enticate(char *user, char *pass, flu_dict *params)
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

int flon_is_subdomain(const char *root, const char *dom)
{
  size_t l = strlen(root);
  if (strncmp(root, dom, l) != 0) return 0;
  return dom[l] == '\0' || dom[l] == '.';
}

int flon_may(char right, const char *user, const char *dom)
{
  if (user == NULL) return 0;
  if (dom == NULL) return 0;

  if ( ! load_domain()) return 0;

  fdja_value *doms = fdja_lookup(domain, user);
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

int flon_may_r(char right, fshv_env *env, const char *dom)
{
  //return flon_may(right, flu_list_get(env->bag, "_user"), dom);
  return flon_may(right, fshv_get_user(env, NULL), dom);
}

