
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

static int is_index(char *key)
{
  if (key == NULL || *key != '_') return 0;
  for (key = key + 1; *key; ++key) if (*key < '0' || *key > '9') return 0;
  return 1;
}

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

//static fdja_value *tree_clone(fdja_value *node, fdja_value *msg)
//{
//  return fdja_clone(tree(node, msg));
//}

static fdja_value *payload(fdja_value *msg)
{
  return fdja_l(msg, "payload");
}

static fdja_value *payload_clone(fdja_value *msg)
{
  return fdja_clone(fdja_l(msg, "payload"));
}

static fdja_value *parent(fdja_value *node)
{
  char *nid = fdja_ls(node, "nid"); if (nid == NULL) return NULL;
  char *pnid = flon_node_parent_nid(nid);
  fdja_value *r = flon_node(pnid);

  free(nid);
  free(pnid);

  return r;
}

static fdja_value *lookup_var(fdja_value *node, const char *key)
{
  if (node == NULL) return NULL;

  fdja_value *vars = fdja_l(node, "vars");

  if (vars)
  {
    fdja_value *val = fdja_l(vars, key);
    if (val) return val;
  }

  return lookup_var(parent(node), key);
}

static fdja_value *lookup_vars(fdja_value *node)
{
  fdja_value *vars = fdja_l(node, "vars");

  if (vars) return vars;

  fdja_value *par = parent(node);
  if (par) return lookup_vars(par);

  return NULL;
}

static void set_var(fdja_value *node, char *key, fdja_value *val)
{
  fdja_value *vars = lookup_vars(node);

  if (vars) fdja_pset(vars, key, val);

  // TODO: update the "mtime" of the node holding the "vars"
}

static void set_error_note(fdja_value *node, char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *t = flu_svprintf(format, ap);
  fdja_value *v = va_arg(ap, fdja_value *);
  va_end(ap);

  if (v == NULL)
  {
    fdja_set(node, "note", fdja_s(t));
  }
  else
  {
    //char *sv = fdja_to_djan(v, FDJA_F_ONELINE | FDJA_F_COMPACT | FDJA_F_NULL);
    char *sv = fdja_to_djan(v, FDJA_F_ONELINE | FDJA_F_NULL);
    fdja_set(node, "note", fdja_s("%s: %s", t, sv));
    free(sv);
  }

  free(t);
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
  return 0; // none

  // TODO: "vf" vs "fv" ? or $(a||v.a) vs $(v.a||a)
}

typedef struct { fdja_value *node; fdja_value *msg; } lup;

static char *dol_lookup(void *data, const char *path)
{
  lup *lu = data;

  // some shortcuts

  if (strcmp(path, "nid") == 0) return fdja_ls(lu->node, "nid");
  if (strcmp(path, "exid") == 0) return strdup(execution_id);

  if (strcmp(path, "exnid") == 0 || strcmp(path, "enid") == 0)
  {
    char *nid = fdja_ls(lu->node, "nid");
    char *r = flu_sprintf("%s-%s", execution_id, nid);
    free(nid);
    return r;
  }

  // regular case, var or fld

  char k = extract_prefix(path);

  fdja_value *v = NULL;

  if (k != 0) path = strchr(path, '.') + 1;

  if (k == 'v')
    v = lookup_var(lu->node, path);
  else
    v = fdja_l(payload(lu->msg), path);

  if (v == NULL)
    return strdup("");

  if (v->type == 's' || v->type == 'q' || v->type == 'y')
    return fdja_to_string(v);

  return fdja_tod(v);
}

static int is_blank(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static fdja_value *expand(fdja_value *v, fdja_value *node, fdja_value *msg)
{
  if (v->key && strstr(v->key, "$("))
  {
    char *k = v->key;
    v->key = fdol_expand(k, &(lup){ node, msg }, dol_lookup);
    free(k);
  }

  if (v->type == 's' || v->type == 'q' || v->type == 'y')
  {
    char *s = fdja_to_string(v);
    char *dol = strstr(s, "$(");

    if (dol)
    {
      char *ss = fdol_expand(s, &(lup){ node, msg }, dol_lookup);

      fdja_value *vv = NULL;
      if (is_blank(*ss) || is_blank(*(ss + strlen(ss) - 1))) vv = fdja_s(ss);
      if (vv == NULL) vv = fdja_v(ss);
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

  return v;
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

static void unschedule_timer(
  const char *type, char *ts, fdja_value *node, size_t i)
{
  fgaj_d("%zu %s: %s\n", i, type, ts);

  fdja_psetv(node, "timers.%zu.cancelled", i, "true");

  char *nid = fdja_ls(node, "nid", NULL);

  flon_unschedule_msg(type, ts, nid);

  free(nid);
  free(ts);
}

static void unschedule_timers(fdja_value *node, fdja_value *msg)
{
  //log_d(node, msg, "self: %s", fdja_tod(node));

  fdja_value *timers = fdja_l(node, "timers");
  if (timers == NULL) return;

  for (size_t i = 0; ; ++i)
  {
    if (fdja_l(node, "timers.%zu.cancelled", i)) continue;

    char *at = fdja_ls(node, "timers.%zu.at", i, NULL);
    if (at) { unschedule_timer("at", at, node, i); continue; }

    char *cron = fdja_ls(node, "timers.%zu.cron", i, NULL);
    if (cron) { unschedule_timer("cron", cron, node, i); continue; }

    break;
  }
}

static char rcv_(fdja_value *node, fdja_value *rcv)
{
  // TODO: remove timers
  // TODO: cancel bastards

  return 'v'; // over
}

static size_t cancel_dependents(fdja_value *node, char *type)
{
  fdja_value *array = fdja_l(node, type);
  if (array == NULL) return 0;

  char *nid = fdja_ls(node, "nid", NULL);

  for (fdja_value *v = array->child; v; v = v->sibling)
  {
    char *cnid = fdja_to_string(v);
    flon_queue_msg("cancel", cnid, nid, NULL, NULL);
    free(cnid);
  }

  free(nid);

  return fdja_size(array);
}

static char can_(fdja_value *node, fdja_value *can)
{
  fdja_set(node, "status", fdja_s("cancelling"));
  //fdja_set(node, "flavor", fdja_s("normal")); // normal / kill ?

  size_t count = 0;
  count += cancel_dependents(node, "children");
  count += cancel_dependents(node, "bastards");

  // if no children, return 'v';

  unschedule_timers(node, can);

  return count > 0 ? 'k' : 'v'; // over
}

#include "fl_n_call.c"
#include "fl_n_cmp.c"
#include "fl_n_concurrence.c"
#include "fl_n_define.c"
#include "fl_n_invoke.c"
#include "fl_n_sequence.c"
#include "fl_n_set.c"
#include "fl_n_stall.c"
#include "fl_n_trace.c"
#include "fl_n_val.c"
#include "fl_n_wait.c"


//
// function table

typedef struct {
  char *name;
  flon_instruction *exe;
  flon_instruction *rcv;
  flon_instruction *can;
} flon_ni;

static flon_ni *instructions[] = {
  &(flon_ni){ "sequence", exe_sequence, rcv_sequence, can_ },
  &(flon_ni){ "define", exe_define, rcv_, can_ },
  &(flon_ni){ "set", exe_set, rcv_, can_ },
  &(flon_ni){ "call", exe_call, rcv_, can_ },
  &(flon_ni){ "invoke", exe_invoke, rcv_invoke, can_ },
  &(flon_ni){ "cmp", exe_cmp, rcv_, can_ },
  &(flon_ni){ "concurrence", exe_concurrence, rcv_concurrence, can_ },
  &(flon_ni){ "trace", exe_trace, rcv_, can_ },
  &(flon_ni){ "stall", exe_stall, rcv_, can_ },
  &(flon_ni){ "val", exe_val, rcv_, can_ },
  &(flon_ni){ "wait", exe_wait, rcv_, can_ },
  NULL
};


//
// call instruction

flon_instruction *flon_lookup_instruction(char dir, const char *name)
{
  for (size_t i = 0; ; ++i)
  {
    flon_ni *ni = instructions[i];

    if (ni == NULL) break;
    if (strcmp(ni->name, name) != 0) continue;

    flon_instruction *inst = ni->exe;
    if (dir == 'r') inst = ni->rcv;
    else if (dir == 'c') inst = ni->can;

    return inst;
  }

  return NULL;
}

char flon_call_instruction(char dir, fdja_value *node, fdja_value *msg)
{
  char *inst = fdja_ls(node, "inst");

  fgaj_d("dir: %c, inst: %s", dir, inst);

  flon_instruction *i = flon_lookup_instruction(dir, inst);

  //if (i == NULL)
  //{
  //  fdja_value *v = lookup_var(node, inst);
  //
  //  if (is_callable(v))
  //  {
  //    fdja_psetv(node, "inst", "call");
  //
  //    return flon_call_instruction(dir, "call", node, msg);
  //  }
  //}

  char r = '?'; // 'unknown' for now

  if (i == NULL)
  {
    fdja_set(node, "status", fdja_s("failed"));
    fdja_set(node, "note", fdja_s("unknown instruction '%s'", inst));

    goto _over;
  }

  r = i(node, msg);

  if (r == 'r') // error
  {
    fdja_set(node, "status", fdja_s("failed"));
    //fdja_set(node, "note", fdja_s("xxx")); // set by the instruction itself
  }

_over:

  free(inst);

  return r;
}

