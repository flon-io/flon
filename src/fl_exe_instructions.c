
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
#include "fl_executor.h"


// TODO: include parsable documentation on top of each "instruction"

// *** INVOKE

static char exe_invoke(fdja_value *node, fdja_value *exe)
{
  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *inv = fdja_v("{ exid: \"%s\", nid: \"%s\" }", execution_id, nid);
  fdja_set(inv, "invoke", fdja_lc(exe, "tree"));
  fdja_set(inv, "payload", fdja_lc(exe, "payload"));

  fdja_pset(inv, "payload.args", fdja_lc(exe, "tree.1"));

  fdja_to_json_f(inv, "var/spool/inv/inv_%s-%s.json", execution_id, nid);

  fdja_free(inv);
  free(nid);

  return 'k'; // ok
}

static char rcv_invoke(fdja_value *node, fdja_value *rcv)
{
  // TODO enventually remove payload.args?

  return 'v'; // over
}

// *** SEQUENCE

static char rcv_sequence(fdja_value *node, fdja_value *rcv)
{
  char r = 'k'; // ok
  char *nid = fdja_ls(node, "nid", NULL);
  char *from = fdja_ls(rcv, "from", NULL);

  char *next = from ? flon_nid_next(from) : flu_sprintf("%s_0", nid);

  fdja_value *tree = flon_node_tree(next);

  if (tree)
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

// function table

typedef struct {
  char *name;
  flon_instruction *exe;
  flon_instruction *rcv;
} flon_ni;

static flon_ni *instructions[] = {
  &(flon_ni){ "invoke", exe_invoke, rcv_invoke },
  &(flon_ni){ "sequence", exe_sequence, rcv_sequence },
  &(flon_ni){ "trace", exe_trace, NULL },
  NULL
};

// function lookup

flon_instruction *flon_instruction_lookup(char dir, const char *name)
{
  for (size_t i = 0; instructions[i] != NULL; ++i)
  {
    flon_ni *ni = instructions[i];
    if (strcmp(ni->name, name) == 0) return dir == 'r' ? ni->rcv : ni->exe;
  }

  fgaj_e(
    "don't know how to %s \"%s\"",
    dir == 'r' ? "receive" : "execute", name);

  return NULL;
}

