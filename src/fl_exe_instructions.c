
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

#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "flutim.h"
#include "djan.h"
#include "gajeta.h"
#include "dollar.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_executor.h"


// TODO: include parsable documentation on top of each "instruction"


//
// declarations

typedef char flon_instruction(fdja_value *, fdja_value *);


//
// helpers

//static char *node_id(fdja_value *node, fdja_value *msg)
//{
//  //flu_putf(fdja_todc(node));
//  //flu_putf(fdja_todc(msg));
//
//  char *nid = fdja_lsd(node, "nid", "nid?");
//  char *exid = fdja_ls(node, "exid", NULL);
//  if (exid == NULL) exid = fdja_lsd(msg, "exid", "exid?");
//
//  char *r = flu_sprintf("%s-%s", exid, nid);
//
//  free(nid); free(exid);
//
//  return r;
//}

static void n_log(
  char level,
  fdja_value *node, fdja_value *msg,
  const char *file, int line, const char *func,
  const char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *m = flu_svprintf(format, ap);
  va_end(ap);

  char *nid = fdja_lsd(node, "nid", "nid?");

  fgaj_log(level, file, line, func, "-%s: %s", nid, m);

  free(nid);
  free(m);
}
#define log_t(node, msg, ...) \
  n_log('t', node, msg, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_d(node, msg, ...) \
  n_log('d', node, msg, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_i(node, msg, ...) \
  n_log('i', node, msg, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_w(node, msg, ...) \
  n_log('w', node, msg, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_e(node, msg, ...) \
  n_log('e', node, msg, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_r(node, msg, ...) \
  n_log('r', node, msg, __FILE__, __LINE__, __func__, __VA_ARGS__)

static fdja_value *tree(fdja_value *node, fdja_value *msg)
{
  fdja_value *r =  NULL;

  if (msg) r = fdja_l(msg, "tree");

  if (r) return r;

  char *nid = fdja_ls(node, "nid", NULL);
  if (nid == NULL) return NULL;

  r = flon_node_tree(nid);

  free(nid);

  return r;
}

static fdja_value *payload(fdja_value *msg, int clone)
{
  fdja_value *pl = fdja_l(msg, "payload");
  if (pl == NULL) return NULL;
  return clone ? fdja_clone(pl) : pl;
}

static ssize_t child_count(fdja_value *node, fdja_value *msg)
{
  fdja_value *t = tree(node, msg);
  if (t == NULL) return -1;

  fdja_value *cs = fdja_lookup(t, "2");
  if (cs == NULL) return -1;

  return fdja_size(cs);
}

static char extract_prefix(const char *path)
{
  if (strncmp(path, "f.", 2) == 0) return 'f';
  if (strncmp(path, "v.", 2) == 0) return 'v';
  if (strncmp(path, "fld.", 4) == 0) return 'f';
  if (strncmp(path, "var.", 4) == 0) return 'v';
  if (strncmp(path, "field.", 4) == 0) return 'f';
  if (strncmp(path, "variable.", 9) == 0) return 'v';
  //return *path; // no worky...
  return 'F'; // default
}

typedef struct { fdja_value *node; fdja_value *msg; } lup;

static char *lookup(void *data, const char *path)
{
  lup *lu = data;

  fdja_value *pl = payload(lu->msg, 0);
  fdja_value *v = fdja_l(pl, path);

  if (v == NULL) return strdup("");
  return fdja_to_string(v);
}

static void expand(
  fdja_value *v, fdja_value *node, fdja_value *msg)
{
  if (v->key && strstr(v->key, "$("))
  {
    char *k = v->key;
    v->key = fdol_expand(k, &(lup){ node, msg }, lookup);
    free(k);
  }

  if (v->type == 's' || v->type == 'q' || v->type == 'y')
  {
    char *s = fdja_to_string(v);
    if (strstr(s, "$("))
    {
      char *ss = fdol_expand(s, &(lup){ node, msg }, lookup);

      fdja_value *vv = fdja_v(ss);
      if (vv == NULL) vv = fdja_s(ss);

      fdja_replace(v, vv);

      free(ss);
    }
    free(s);
  }
  else if (v->type == 'o' || v->type == 'a')
  {
    for (fdja_value *c = v->child; c; c = c->sibling) expand(c, node, msg);
  }
  //else // do not expand
}

static fdja_value *attributes(fdja_value *node, fdja_value *msg)
{
  fdja_value *atts = fdja_l(tree(node, msg), "1");
  fdja_value *r = fdja_v("{}");

  if (atts == NULL || atts->type != 'o')
  {
    log_e(node, msg, "tree not found, no attributes");
    return r;
  }

  for (fdja_value *v = atts->child; v; v = v->sibling)
  {
    fdja_value *vv = fdja_clone(v); vv->key = strdup(v->key);
    expand(vv, node, msg);
    fdja_set(r, vv->key, vv);
  }

  return r;
}


//
// ... some defaults

static char rcv_(fdja_value *node, fdja_value *rcv)
{
  return 'v'; // over
}


//
// *** INVOKE

static char exe_invoke(fdja_value *node, fdja_value *exe)
{
  char r = 'k'; // for now, ok

  char *exid = execution_id;
  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *inv = fdja_v("{ exid: \"%s\", nid: \"%s\" }", exid, nid);
  fdja_psetv(inv, "point", "invoke");
  fdja_set(inv, "tree", fdja_lc(exe, "tree"));
  fdja_set(inv, "payload", payload(exe, 1));

  fdja_value *args = fdja_lc(exe, "tree.1");
  expand(args, node, exe);
  fdja_pset(inv, "payload.args", args);

  if (flon_lock_write(inv, "var/spool/dis/inv_%s-%s.json", exid, nid) != 1)
  {
    fgaj_r("failed writing to var/spool/dis/inv_%s-%s.json", exid, nid);
    r = 'r';
  }

  fdja_free(inv);
  free(nid);

  return r;
}

static char rcv_invoke(fdja_value *node, fdja_value *rcv)
{
  fdja_pset(rcv, "payload.args", NULL);

  return 'v'; // over
}


//
// *** SEQUENCE

static char rcv_sequence(fdja_value *node, fdja_value *rcv)
{
  char r = 'k'; // ok, for now

  char *nid = fdja_ls(node, "nid", NULL);
  char *from = fdja_ls(rcv, "from", NULL);

  char *next = from ? flon_nid_next(from) : flu_sprintf("%s_0", nid);

  fdja_value *t = next ? flon_node_tree(next) : NULL;

  if (t)
    flon_queue_msg("execute", next, nid, payload(rcv, 0));
  else
    r = 'v'; // over

  free(nid);
  free(next);
  if (from) free(from);

  return r;
}

static char exe_sequence(fdja_value *node, fdja_value *exe)
{
  if (child_count(node, exe) < 1) return 'v';
  return rcv_sequence(node, exe);
}


//
// *** TRACE

static char exe_trace(fdja_value *node, fdja_value *exe)
{
  fdja_value *pl = payload(exe, 0);

  if (fdja_l(pl, "trace", NULL) == NULL) fdja_set(pl, "trace", fdja_v("[]"));
  fdja_value *trace = fdja_l(pl, "trace");

  fdja_value *atts = attributes(node, exe);

  fdja_push(trace, fdja_lc(atts, "_0"));

  fdja_free(atts);

  return 'v'; // over
}


//
// *** SET

static char exe_set(fdja_value *node, fdja_value *exe)
{
  fdja_value *pl = payload(exe, 0);

  fdja_value *atts = attributes(node, exe);

  for (fdja_value *a = atts->child; a; )
  {
    char *key = a->key;
    fdja_value *sibling = a->sibling;

    fgaj_d("key: 0 >%s<", key);

    char k = extract_prefix(a->key);
    if (k == 'f' || k == 'v') key = strchr(key, '.') + 1;

    fgaj_d("key: 1 >%s<", key);

    if (k == 'f' || k == 'F') fdja_pset(pl, key, a);
    //else if (k == 'v') // TODO

    a = sibling;
  }

  atts->child = NULL; fdja_free(atts);

  return 'v'; // over
}


//
// *** WAIT

static char exe_wait(fdja_value *node, fdja_value *exe)
{
  fdja_value *atts = attributes(node, exe);

  char *f = fdja_ls(atts, "_0", "");
  long long s = flu_parse_t(f);
  char *a = flu_sstamp(flu_gets('s') + s, 1, 's');

  log_d(node, exe, "sleep for %s (%llis) --> until %s", f, s, a);

  char *nid = fdja_ls(node, "nid", NULL);
  char *exid = fdja_ls(exe, "exid", NULL);

  fdja_value *msg = fdja_v("{}");
  fdja_set(msg, "point", fdja_s("receive"));
  fdja_set(msg, "nid", fdja_s(nid));
  fdja_set(msg, "exid", fdja_s(exid));

  flon_schedule_msg("at", a, nid, msg);

  fdja_free(atts);
  free(f);
  free(a);
  free(nid);
  free(exid);

  return 'k'; // ok
}


//
// function table

typedef struct {
  char *name;
  flon_instruction *exe;
  flon_instruction *rcv;
} flon_ni;

static flon_ni *instructions[] = {
  &(flon_ni){ "invoke", exe_invoke, rcv_invoke },
  &(flon_ni){ "sequence", exe_sequence, rcv_sequence },
  &(flon_ni){ "trace", exe_trace, rcv_ },
  &(flon_ni){ "set", exe_set, rcv_ },
  &(flon_ni){ "wait", exe_wait, rcv_ },
  NULL
};


//
// call instruction

static char unknown_instruction(
  char dir, const char *name, fdja_value *node, fdja_value *msg)
{
  fdja_set(node, "status", fdja_s("failed"));
  fdja_set(node, "note", fdja_s("unknown instruction '%s'", name));

  return '?';
}

char flon_call_instruction(
  char dir, const char *name, fdja_value *node, fdja_value *msg)
{
  fgaj_d("dir: %c, inst: %s", dir, name);

  flon_instruction *i = NULL;

  for (size_t j = 0; ; ++j)
  {
    flon_ni *ni = instructions[j];

    if (ni == NULL) break;
    if (strcmp(ni->name, name) != 0) continue;

    i = (dir == 'r') ? ni->rcv : ni->exe;
    break;
  }

  if (i == NULL) return unknown_instruction(dir, name, node, msg);

  char r = i(node, msg);

  // TODO: handle other kinds of errors

  return r;
}

