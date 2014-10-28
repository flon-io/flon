
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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "flutil.h"
#include "mnemo.h"
#include "aabro.h"
#include "djan.h"
#include "fl_common.h"
#include "fl_ids.h"


static short counter = 0;


char *flon_generate_exid(const char *domain)
{
  char *uid = flon_conf_uid();
  short local = flon_conf_is("unit.time", "local");

  struct timeval tv;
  struct tm *tm;
  char t[20];

  gettimeofday(&tv, NULL);
  tm = local ? localtime(&tv.tv_sec) : gmtime(&tv.tv_sec);
  strftime(t, 20, "%Y%m%d.%H%M", tm);

  char *sus =
    fmne_to_s((tv.tv_sec % 60) * 100000000 + tv.tv_usec * 100 + counter);

  char *r =
    flu_sprintf("%s-%s-%s.%s", domain, uid, t, sus);

  free(sus);
  free(uid);

  counter++; if (counter > 99) counter = 0;

  return r;
}


static fabr_parser *flon_nid_parser = NULL;

static void flon_nid_parser_init()
{
  fabr_parser *hex = fabr_rex("(0|[1-9a-f][0-9a-f]*)");
  fabr_parser *symb = fabr_rex("[a-z0-9_]+");
  fabr_parser *dash = fabr_string("-");
  fabr_parser *dot = fabr_string(".");

  fabr_parser *node =
    fabr_n_seq("node", hex, fabr_seq(fabr_string("_"), hex, fabr_r("*")), NULL);
  fabr_parser *counter =
    fabr_name("counter", hex);

  fabr_parser *domain =
    fabr_n_seq("domain", symb, fabr_seq(dot, symb, fabr_r("*")), NULL);
  fabr_parser *feu =
    fabr_n_seq(
      "feu",
      fabr_n_seq("group", symb, dot, fabr_r("?")),
      fabr_name("unit", symb), NULL);
  fabr_parser *tid =
    fabr_n_seq(
      "tid",
      fabr_n_rex("date", "[0-9]{8,9}"), dot,
      fabr_n_rex("hour", "[0-9]{4}"), dot,
      fabr_n_rex("sec", "[a-z]+"),
      NULL);

  fabr_parser *exid =
    fabr_n_seq("exid", domain, dash, feu, dash, tid, NULL);
  fabr_parser *nid =
    fabr_n_seq("nid", node, fabr_seq(dash, counter, fabr_r("?")), NULL);

  fabr_parser *msg =
    fabr_n_rex("msg", "(exe_|inv_|rcv_|ret_)");
  fabr_parser *ftype =
    fabr_n_rex("ftype", "\\.json");

  flon_nid_parser =
    fabr_alt(
      fabr_seq(msg, exid, fabr_seq(dash, nid, fabr_r("?")), ftype, NULL),
      fabr_seq(exid, dash, nid, NULL),
      exid,
      nid,
      NULL);
}

fdja_value *flon_parse_nid(const char *s)
{
  if (flon_nid_parser == NULL) flon_nid_parser_init();

  //fabr_tree *dt = fabr_parse_f(s, 0, flon_nid_parser, 0);
  ////fabr_tree *dt = fabr_parse_all(s, 0, flon_nid_parser);
  //printf("s >%s<\n", s);
  //puts(fabr_tree_to_string(dt, s, 1));

  fabr_tree *t = fabr_parse_all(s, 0, flon_nid_parser);
  if (t->result != 1) { fabr_tree_free(t); return NULL; }

  fdja_value *r = fdja_v("{}");

  char *keys[] = {
    "msg",
    "exid", "domain", "feu", "tid", "nid", "node", "counter",
    "ftype", NULL
  };
  size_t i = 0; for (char *k = keys[i]; k != NULL; k = keys[++i])
  {
    fabr_tree *tt = fabr_tree_lookup(t, k);
    if (tt)
    {
      char *v  = fabr_tree_string(s, tt);
      fdja_set(r, k, fdja_s(v));
      free(v);
    }
  }

  fabr_tree_free(t);

  return r;
}

char *flon_exid_path(const char *s)
{
  char *r = NULL;
  fdja_value *i = NULL;
  char *exid = NULL;
  char *domain = NULL;

  i = flon_parse_nid(s);
  if (i == NULL) goto _over;

  exid = fdja_ls(i, "exid", NULL);
  domain = fdja_ls(i, "domain", NULL);

  if (exid == NULL) goto _over;

  size_t l = strlen(exid);
  char *dot = strrchr(exid, '.') + 1;

  r = flu_sprintf("%s/%.2s/%s", domain, dot, exid);

_over:

  if (i) fdja_free(i);
  if (exid) free(exid);
  if (domain) free(domain);

  return r;
}

char *flon_nid_next(const char *nid)
{
  fdja_value *i = flon_parse_nid(nid);

  char *n = fdja_ls(i, "node");
  char *u = strrchr(n, '_');
  *u = 0;
  long l = strtol(u + 1, NULL, 16);
  char *r = flu_sprintf("%s_%x", n, l + 1);

  free(n);
  fdja_free(i);

  return r;
}

