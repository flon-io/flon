
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


// at-20141130.105800-dtest.trig-u0-20141207.0156.kagemusha-0_0.json

static fabr_tree *_dash(fabr_input *i) { return fabr_str(NULL, i, "-"); }
static fabr_tree *_dot(fabr_input *i) { return fabr_str(NULL, i, "."); }
static fabr_tree *_us(fabr_input *i) { return fabr_str(NULL, i, "_"); }

static fabr_tree *_hex(fabr_input *i)
{ return fabr_rex(NULL, i, "(0|[1-9a-f][0-9a-f]*)"); }

static fabr_tree *_symb(fabr_input *i)
{ return fabr_rex(NULL, i, "[a-z0-9_]+"); }

static fabr_tree *_group(fabr_input *i)
{ return fabr_seq("group", i, _symb, _dot, NULL); }
static fabr_tree *_unit(fabr_input *i)
{ return fabr_rename("unit", i, _symb); }
static fabr_tree *_feu(fabr_input *i)
{ return fabr_seq("feu", i, _group, fabr_qmark, _unit, NULL); }

static fabr_tree *_date(fabr_input *i)
{ return fabr_rex("date", i, "[0-9]{8,9}"); }
static fabr_tree *_hour(fabr_input *i)
{ return fabr_rex("hour", i, "[0-9]{4}"); }
static fabr_tree *_sec(fabr_input *i)
{ return fabr_rex("sec", i, "[a-z]+"); }
static fabr_tree *_tid(fabr_input *i)
{ return fabr_seq("tid", i, _date, _dot, _hour, _dot, _sec, NULL); }

static fabr_tree *_domain(fabr_input *i)
{ return fabr_jseq("domain", i, _symb, _dot); }

static fabr_tree *_exid(fabr_input *i)
{ return fabr_seq("exid", i, _domain, _dash, _feu, _dash, _tid, NULL); }

static fabr_tree *_counter(fabr_input *i)
{ return fabr_rename("counter", i, _hex); }

static fabr_tree *_dash_counter(fabr_input *i)
{ return fabr_seq(NULL, i, _dash, _counter, NULL); }

static fabr_tree *_node(fabr_input *i)
{ return fabr_jseq("node", i, _hex, _us); }

static fabr_tree *_msg(fabr_input *i)
{ return fabr_rex("msg", i, "((exe|tsk|rcv|ret|sch|can)_|at-[^-]+-)"); }

static fabr_tree *_ftype(fabr_input *i)
{ return fabr_rex("ftype", i, "\\.[^\\.]+"); }

static fabr_tree *_nid(fabr_input *i)
{ return fabr_seq("nid", i, _node, _dash_counter, fabr_qmark, NULL); }

static fabr_tree *_dash_nid(fabr_input *i)
{ return fabr_seq(NULL, i, _dash, _nid, NULL); }

static fabr_tree *_exid_nid(fabr_input *i)
{ return fabr_seq(NULL, i, _exid, _dash_nid, fabr_qmark, NULL); }

static fabr_tree *_fname(fabr_input *i)
{ return fabr_seq("FNAME", i, _msg, _exid_nid, _ftype, NULL); }

//  flon_nid_parser =
//    fabr_alt(
//      fabr_seq(msg, exid, fabr_seq(dash, nid, fabr_r("?")), ftype, NULL),
//      fabr_seq(exid, dash, nid, NULL),
//      exid,
//      nid,
//      NULL);
static fabr_tree *_root(fabr_input *i)
{ return fabr_alt(NULL, i, _fname, _exid_nid, _nid, NULL); }

fdja_value *flon_parse_nid(const char *s)
{
  //printf("flon_parse_nid() s   >[1;33m%s[0;0m<\n", s);

  char *ss = (char *)s;
  char *slash = strrchr(ss, '/');
  if (slash) ss = slash + 1;

  //printf("flon_parse_nid() ss  >[1;33m%s[0;0m<\n", ss);

  //fabr_tree *tt = fabr_parse_f(ss, _root, FABR_F_ALL);
  //printf("flon_parse_nid():\n"); fabr_puts_tree(tt, ss, 1);
  //fabr_tree_free(tt);

  fabr_tree *t = fabr_parse_all(ss, _root);
  //printf("flon_parse_nid() (pruned):\n"); fabr_puts(t, ss, 3);
  if (t->result != 1) { fabr_tree_free(t); return NULL; }

  fdja_value *r = fdja_object_malloc();

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
      char *v = fabr_tree_string(ss, tt);
      fdja_set(r, k, fdja_s(v));
      free(v);
    }
  }

  if (slash) fdja_set(r, "uri", fdja_s(s));

  fabr_tree_free(t);

  //flu_putf(fdja_value_to_s(r));

  return r;
}

char *flon_get_exid(const char *s)
{
  fdja_value *v = flon_parse_nid(s);
  if (v == NULL) return NULL;

  char *r = fdja_ls(v, "exid", NULL);
  fdja_free(v);

  return r;
}

char *flon_get_nid(const char *s)
{
  fdja_value *v = flon_parse_nid(s);
  if (v == NULL) return NULL;

  char *r = fdja_ls(v, "nid", NULL);
  fdja_free(v);

  return r;
}

int flon_is_domain(const char *s)
{
  //fabr_tree *tt = fabr_parse_f(s, _domain, FABR_F_ALL);
  //printf("flon_is_domain():\n"); fabr_puts_tree(tt, s, 1);
  //fabr_tree_free(tt);

  fabr_tree *t = fabr_parse_all(s, _domain);
  //printf("flon_is_domain() (pruned):\n"); fabr_puts(t, s, 3);

  int r = t->result == 1;

  fabr_tree_free(t);

  return r;
}

char *flon_exid_domain(const char *exid)
{
  char *dash = strchr(exid, '-');

  return dash ? strndup(exid, dash - exid) : NULL;
}

char *flon_nid_next(const char *nid, int increment)
{
  char *r = NULL;
  char *node = NULL;
  char *counter = NULL;

  fdja_value *i = flon_parse_nid(nid);
  if (i == NULL) goto _over;

  node = fdja_ls(i, "node", NULL);
  if (node == NULL) goto _over;

  char *u = strrchr(node, '_');
  if (u == NULL) goto _over;

  counter = fdja_ls(i, "counter", NULL);

  *u = '\0';
  long long l = strtoll(u + 1, NULL, 16);

  if (counter && strcmp(counter, "0") != 0)
    r = flu_sprintf("%s_%x-%s", node, l + increment, counter);
  else
    r = flu_sprintf("%s_%x", node, l + increment);

_over:

  free(node);
  free(counter);
  fdja_free(i);

  return r;
}

char *flon_nid_parent(const char *nid, int chop)
{
  char *u = strrchr(nid, '_');
  if (u == NULL) return NULL;

  char *i = strdup(nid);

  char *c = i + (u - nid);
  char *d = strchr(nid, '-');

  if (d && chop != 1)
    while (1) { *c = *d; if (*d == 0) break; ++c; ++d; }
  else
    *c = 0;

  return i;
}

char *flon_nid_child(const char *nid, int n)
{
  char *dash = strchr(nid, '-');

  if (dash) return flu_sprintf("%.*s_%x-%s", dash - nid, nid, n, dash + 1);
  return flu_sprintf("%s_%x", nid, n);
}

size_t flon_nid_depth(const char *nid)
{
  char *s = (char *)nid;
  short f = 0;
  //
  if (strchr(nid, '-'))
  {
    fdja_value *i = flon_parse_nid(nid);
    s = fdja_ls(i, "node", "");
    fdja_free(i);
    f = 1;
  }

  char *ss = s;

  size_t r = 0;
  while (1)
  {
    ss = strchr(ss, '_');
    if (ss == NULL) break;
    ++r;
    ++ss;
  }

  if (f) free(s);

  return r;
}

size_t flon_nid_index(const char *nid)
{
  char *un = strrchr(nid, '_'); if (un) un++; else un = (char *)nid;
  char *dash = strchr(nid, '-'); if (dash == NULL) dash = strchr(nid, 0);
  char *id = strndup(un, dash - un);

  size_t r = strtoll(id, NULL, 16);

  free(id);

  return r;
}

char *flon_point_to_prefix(const char *point)
{
  if (point == NULL) return "UNK_";
  if (*point == 'e') return "exe_";
  if (*point == 'i') return "inv_";
  if (*point == 's') return "sch_";
  if (*point == 'r') return "rcv_";
  if (*point == 'c') return "can_";
  if (*point == 't') return "tsk_";
  return "UNK_";
}

int flon_is_plain_receive(fdja_value *msg)
{
  int r = 0;

  char *emitter = NULL;
  char *receiver = NULL;
  fdja_value *em = NULL;
  fdja_value *re = NULL;
  char *emc = NULL;
  char *rec = NULL;
  char *emn = NULL;
  char *ren = NULL;

  emitter = fdja_ls(msg, "from", NULL);
  if (emitter == NULL) goto _over;

  receiver = fdja_ls(msg, "nid", NULL);
  if (receiver == NULL) goto _over;

  em = flon_parse_nid(emitter);
  re = flon_parse_nid(receiver);
  if (em == NULL || re == NULL) goto _over;

  emc = fdja_ls(em, "counter", NULL);
  rec = fdja_ls(re, "counter", NULL);

  if (emc == NULL) emc = strdup("");
  if (rec == NULL) rec = strdup("");
  if (strcmp(emc, rec) != 0) goto _over;

  emn = fdja_ls(em, "node", "");
  ren = fdja_ls(re, "node", "");

  if (strncmp(emn, ren, strlen(ren)) != 0) goto _over;

  r = 1;

_over:

  free(emitter);
  free(receiver);
  fdja_free(em);
  fdja_free(re);
  free(emc);
  free(rec);
  free(emn);
  free(ren);

  return r;
}

