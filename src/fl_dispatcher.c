
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "flutil.h"
#include "djan.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


static int double_fork(char *ctx, char *logpath, char *arg)
{
  pid_t i = fork();

  if (i < 0) { fgaj_r("fork 1 for %s failed", ctx); return 0; }

  if (i == 0) // intermediate parent
  {
    pid_t j = fork();

    if (j < 0) { fgaj_r("fork 2 for %s failed", ctx); _exit(127); }
    if (j != 0) { _exit(0); } // intermediate parent exits immediately

    // double forked child

    if (setsid() == -1) { fgaj_r("setsid() failed"); _exit(127); }

    char *dir = flon_conf_path("_root", ".");
    fgaj_i("dir is >%s<", dir);
    if (chdir(dir) != 0) { fgaj_r("failed to chdir()"); _exit(127); }

    fflush(stderr);

    if (freopen(logpath, "a", stderr) == NULL)
    {
      fgaj_r("failed to reopen stderr to %s", logpath);
      _exit(127);
    }
    fgaj_i("pointing invoker stderr to %s", logpath);

    fflush(stderr);

    // TODO: if ctx is "executor", write var/run/{exid}.pid

    char *bin = NULL;
    //
    if (*ctx == 'i')
      bin = flon_conf_string("invoker.bin", "bin/flon-invoker");
    else
      bin = flon_conf_string("executor.bin", "bin/flon-executor");

    fgaj_i("cmd is >%s %s<", bin, arg);

    int r = execv(bin, (char *[]){ bin, arg, NULL });

    // fail zone...

    fgaj_r("execv failed (%i)", r);

    fflush(stderr);

    _exit(127);
  }
  else { // parent

    fgaj_i("%s forked", ctx);
  }

  return 0; // success
}

static int dispatch(const char *fname, fdja_value *j)
{
  if (j == NULL) return -1;

  //puts(fdja_to_pretty_djan(j));

  if (
    fdja_l(j, "execute") == NULL &&
    fdja_l(j, "receive") == NULL &&
    fdja_l(j, "invoke") == NULL
  ) return -1;

  // TODO:
  // return success if executor and executor already running for that exid

  int r = 1;

  char *tname = flu_basename(fname, ".txt");
  char *exid = fdja_ls(j, "exid", NULL);
  char *nid = fdja_ls(j, "nid", NULL);

  char *ct = "exe";
  char *ctx = "executor";
  char *arg = exid;
  char *logpath = NULL;
  //
  if (*fname == 'i') // invoke
  {
    ct = "inv";
    ctx = "invoker";
    arg = flu_sprintf("var/spool/inv/%s", fname);
    logpath = flu_sprintf("var/log/inv/%s-%s.txt", exid, nid);
  }
  else // execute, receive
  {
    logpath = flu_sprintf("var/log/exe/%s.txt", exid);
  }

  if (flu_move("var/spool/dis/%s", fname, "var/spool/%s/%s", ct, fname) != 0)
  {
    fgaj_r("failed to move %s to var/spool/%s/%s", fname, ct, fname);
    r = -1; // triggers rejection
  }

  if (r == 1) r = double_fork(ctx, logpath, arg);

  free(exid);
  free(nid);
  free(logpath);

  return r;
}

static int reject(const char *fname)
{
  int r = flu_move("var/spool/dis/%s", fname, "var/spool/rejected/");

  if (r == 0) fgaj_i(fname);
  else fgaj_r("failed to move %s to var/spool/rejected/", fname);

  return 1; // failure
}

static int receive_ret(const char *fname)
{
  int r = 0;

  fdja_value *i = NULL;
  fdja_value *j = NULL;

  j = fdja_parse_obj_f("var/spool/dis/%s", fname);
  if (j == NULL) { r = -1; goto _over; }

  i = flon_parse_nid(fname);
  if (i == NULL) { r = -1; goto _over; }

  fdja_set(i, "receive", fdja_v("1"));
  fdja_set(i, "payload", j);

  int rr = fdja_to_json_f(i, "var/spool/dis/rcv_%s", fname + 4);
  if (rr != 1) { r = -1; goto _over; }

  // unlink inv_

  if (flu_unlink("var/spool/inv/inv_%s", fname + 4) == 0)
    fgaj_i("unlinked var/spool/inv/inv_%s", fname + 4);
  else
    fgaj_i("failed to unlink var/spool/inv/inv_%s", fname + 4);

  // unlink ret_

  if (flu_unlink("var/spool/dis/%s", fname) == 0)
    fgaj_i("unlinked var/spool/dis/%s", fname);
  else
    fgaj_i("failed to unlink var/spool/dis/%s", fname);

_over:

  if (i && j) fdja_free(i);
  else if (j) fdja_free(j);

  return r;
}

int flon_dispatch(const char *fname)
{
  fgaj_i(fname);

  int r = 0;
  fdja_value *j = NULL;

  if ( ! flu_strends(fname, ".json")) { r = -1; goto _over; }

  if (strncmp(fname, "ret_", 4) == 0) { r = receive_ret(fname); goto _over; }

  if (
    strncmp(fname, "exe_", 4) != 0 &&
    strncmp(fname, "inv_", 4) != 0 &&
    strncmp(fname, "rcv_", 4) != 0
  ) { r = -1; goto _over; }

  j = fdja_parse_obj_f("var/spool/dis/%s", fname);

  // TODO reroute?

  r = dispatch(fname, j);

_over:

  if (r == -1) r = reject(fname);
  if (j) fdja_value_free(j);

  return r;
}

