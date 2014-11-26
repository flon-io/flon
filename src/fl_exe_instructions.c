
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
#include "djan.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_executor.h"


// TODO: include parsable documentation on top of each "instruction"

// helpers

static fdja_value *tree(fdja_value *node)
{
  char *nid = fdja_ls(node, "nid", NULL);
  if (nid == NULL) return NULL;

  fdja_value *r = flon_node_tree(nid);
  free(nid);

  return r;
}

//static fdja_value *payload(fdja_value *exe, int clone)
//{
//  fdja_value *pl = fdja_l(exe, "payload");
//  if (pl == NULL) return NULL;
//  return clone ? fdja_clone(pl) : pl;
//}

static ssize_t child_count(fdja_value *node)
{
  fdja_value *t = tree(node);
  if (t == NULL) return -1;

  fdja_value *cs = fdja_lookup(t, "2");
  if (cs == NULL) return -1;

  return fdja_size(cs);
}

// ... some defaults

static char rcv_(fdja_value *node, fdja_value *rcv)
{
  return 'v'; // over
}

// *** INVOKE

static char exe_invoke(fdja_value *node, fdja_value *exe)
{
  char r = 'k'; // for now, ok

  char *exid = execution_id;
  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *inv = fdja_v("{ exid: \"%s\", nid: \"%s\" }", exid, nid);
  fdja_psetv(inv, "point", "invoke");
  fdja_set(inv, "tree", fdja_lc(exe, "tree"));
  fdja_set(inv, "payload", fdja_lc(exe, "payload"));

  fdja_pset(inv, "payload.args", fdja_lc(exe, "tree.1"));

  if (flon_lock_write(inv, "var/spool/dis/inv_%s-%s.json", exid, nid) != 1)
  {
    fgaj_r("failed writing to var/spool/dis/inv_%s-%s.json", exid, nid);
    r = 'r';
  }

  fdja_free(inv);
  free(nid);

  return r;
}

//static char rcv_invoke(fdja_value *node, fdja_value *rcv)
//{
//  // TODO enventually remove payload.args?
//
//  return 'v'; // over
//}

// *** SEQUENCE

static char rcv_sequence(fdja_value *node, fdja_value *rcv)
{
  char r = 'k'; // ok, for now

  char *nid = fdja_ls(node, "nid", NULL);
  char *from = fdja_ls(rcv, "from", NULL);

  char *next = from ? flon_nid_next(from) : flu_sprintf("%s_0", nid);

  fdja_value *t = next ? flon_node_tree(next) : NULL;

  if (t)
    flon_queue_msg("execute", next, nid, fdja_l(rcv, "payload", NULL));
  else
    r = 'v'; // over

  free(nid);
  free(next);
  if (from) free(from);

  return r;
}

static char exe_sequence(fdja_value *node, fdja_value *exe)
{
  //fgaj_d("cc: %zu", child_count(node));
  if (child_count(node) < 1) return 'v';
  return rcv_sequence(node, exe);
}

// *** TRACE

static char exe_trace(fdja_value *node, fdja_value *exe)
{
  fdja_value *pl = fdja_l(exe, "payload");
  if (fdja_l(pl, "trace", NULL) == NULL) fdja_set(pl, "trace", fdja_v("[]"));
  fdja_value *trace = fdja_l(pl, "trace");
  fdja_push(trace, fdja_lc(exe, "tree.1._0"));

  return 'v'; // over
}

// *** SET

static char exe_set(fdja_value *node, fdja_value *exe)
{
  //if (fdja_l(pl, "trace", NULL) == NULL) fdja_set(pl, "trace", fdja_v("[]"));
  //fdja_value *trace = fdja_l(pl, "trace");
  //fdja_push(trace, fdja_lc(exe, "tree.1._0"));
  //fdja_value *pl = fdja_l(exe, "payload");

  return 'v'; // over
}

// function table

typedef struct {
  char *name;
  flon_instruction *exe;
  flon_instruction *rcv;
} flon_ni;

static flon_ni *instructions[] = {
  &(flon_ni){ "invoke", exe_invoke, rcv_ },
  &(flon_ni){ "sequence", exe_sequence, rcv_sequence },
  &(flon_ni){ "trace", exe_trace, rcv_ },
  &(flon_ni){ "set", exe_set, rcv_ },
  NULL
};

// call instruction

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

  //fgaj_d("%-*s%s %c %s", flon_nid_depth(nid) * 2, "", nid, a, instruction);
  //fgaj_i("%c_%s --> %c", a, instruction, r);

  return i ? i(node, msg) : '?';
}

