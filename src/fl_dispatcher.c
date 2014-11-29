
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>

#include "flutil.h"
#include "flutim.h"
#include "flu64.h"
#include "djan.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


flu_list *at_timers = NULL;
flu_list *cron_timers = NULL;

flu_list *flon__timer(char a_or_c)
{
  return a_or_c == 'a' ? at_timers : cron_timers;
}

static void flon_timer_free(flon_timer *t)
{
  if (t == NULL) return;

  free(t->ts); free(t->fn); free(t);
}

void flon__zero_timers()
{
  flu_list_and_items_free(at_timers, (void (*)(void *))flon_timer_free);
  flu_list_and_items_free(cron_timers, (void (*)(void *))flon_timer_free);

  at_timers = flu_list_malloc();
  cron_timers = flu_list_malloc();
}

static void init_timers()
{
  at_timers = flu_list_malloc();
  cron_timers = flu_list_malloc();

  // TODO: read from files...
}

static int at_cmp(const void *ta, const void *tb)
{
  return strcmp(((flon_timer *)ta)->ts, ((flon_timer *)tb)->ts);
}
//static int cron_cmp(const void *ta, const void *tb)
//{
//  // high frequency first... // MAYBE
//}

static void add_at_timer(const char *ts, const char *fn)
{
  if (at_timers == NULL) init_timers();

  flon_timer *t = calloc(1, sizeof(flon_timer));
  t->ts = strdup(ts);
  t->fn = strdup(fn);

  flu_list_oinsert(at_timers, t, at_cmp);
}

static void add_cron_timer(const char *ts, const char *fn)
{
  if (at_timers == NULL) init_timers();

  flon_timer *t = calloc(1, sizeof(flon_timer));
  t->ts = strdup(ts);
  t->fn = strdup(fn);

  flu_list_add(cron_timers, t);
    // simply add at the end

  //flu_list_oinsert(cron_timers, t, cron_cmp);
    // at some point, one could think of adding high frequency first,
    // low frequency last, to help determine the scheduling frequency...
}

static short schedule(
  const char *fname, fdja_value *msg)
{
  int r = 1; // seen, failed, for now

  char *exid = flon_parse_exid(fname);
  char *fep = flon_exid_path(exid);

  // write to var/spool/tdis/

  char *type = "at";
  char *ts = fdja_ls(msg, "at", NULL);
  if (ts == NULL) { type = "cron"; ts = fdja_ls(msg, "cron", NULL); }

  if (ts == NULL) { r = -1; goto _over; }

  char *ots = ts;
  if (*type == 'c') ts = flu64_encode(ts, -1);

  if (flu_mkdir_p("var/spool/tdis/%s", fep, 0755) != 0)
  {
    fgaj_r("failed to mkdir var/spool/tdis/%s/", fep);
    goto _over;
  }

  char *fn = flu_sprintf(
    "var/spool/tdis/%s/%s-%s-%s", fep, type, ts, fname + 4);

    // directly write the source of the msg to file
  if (flu_writeall(fn, msg->source) != 1)
  {
    fgaj_r("failed to write %s", fn);
    goto _over;
  }

  // list in timer index

  if (*type == 'c')add_cron_timer(ots, fn);
  else add_at_timer(ots, fn);

  // move to processed/

  char *d = flu_fstat("var/run/%s/processed", fep) != 'd' ? "archived" : "run";

  if (flu_move("var/spool/dis/%s", fname, "var/%s/%s/processed/", d, fep) != 0)
  {
    fgaj_r(
      "failed to move var/spool/dis/%s to /var/%s/%sprocessed/", fname, d, fep);
  }

  r = 2; // success

_over:

  if (ots != ts) free(ots);
  free(ts);

  free(fn);
  free(fep);
  free(exid);

  return r;
}

static short double_fork(char *ctx, char *logpath, char *arg)
{
  pid_t i = fork();

  if (i < 0) { fgaj_r("fork 1 for %s failed", ctx); return 1; /* 'seen' */ }

  if (i == 0) // intermediate parent
  {
    pid_t j = fork();

    if (j < 0) { fgaj_r("fork 2 for %s failed", ctx); _exit(127); }
    if (j != 0) { _exit(0); } // intermediate parent exits immediately

    // double forked child

    if (setsid() == -1) { fgaj_r("setsid() failed"); _exit(127); }

    char *dir = flon_conf_path("_root", ".");
    //fgaj_i("dir is >%s<", dir);
    if (chdir(dir) != 0) { fgaj_r("failed to chdir() to %s", dir); _exit(127); }

    fflush(stderr);

    char *basepath = strdup(logpath);
    *(strrchr(basepath, '/')) = '\0';
    //
    if (flu_mkdir_p(basepath, 0755) != 0)
    {
      fgaj_r("failed to mkdir -p %s", basepath);
      _exit(127);
    }
    //free(basepath);

    if (freopen(logpath, "a", stderr) == NULL)
    {
      fgaj_r("failed to reopen stderr to %s", logpath);
      _exit(127);
    }
    fgaj_i("pointed %s stderr to %s", ctx, logpath);

    if (*ctx == 'e')
    {
      freopen(logpath, "a", stdout);
      fflush(stdout);
      fgaj_i("pointed %s stdout to %s", ctx, logpath);

      char *exid = strrchr(basepath, '/') + 1;

      flu_writeall("var/run/%s.pid", exid, "%i", getpid());
      fgaj_d("wrote var/run/%s.pid", exid);
    }
      //
      // the invoker stdout is kept for payload output.

    fflush(stderr);

    char *bin = NULL;
    //
    if (*ctx == 'i')
      bin = flon_conf_string("invoker.bin", "bin/flon-invoker");
    else
      bin = flon_conf_string("executor.bin", "bin/flon-executor");

    char *val = "/usr/bin/valgrind";
    void *args = NULL;

    char *v = getenv("FLONVAL");
    if (
      v &&
      (
        strstr(v, "all") ||
        (*ctx == 'i' && strstr(v, "inv")) ||
        (*ctx == 'e' && strstr(v, "exe"))
      )
    )
    {
      args = &(char *[]){ val, "--leak-check=full", "-v", bin, arg, NULL };
      bin = val;
    }
    else
    {
      args = &(char *[]){ bin, arg, NULL };
    }
    //fgaj_d("FLONVAL: \"%s\", e: '%c'", v, *ctx);

    fgaj_i("cmd is >%s<", bin);
    fflush(stderr);

    int r = execv(bin, args);

    // fail zone...

    fgaj_r("execv failed (%i)", r);

    fflush(stderr);

    _exit(127);
  }
  else // parent
  {
    fgaj_i("%s forked", ctx);
  }

  return 2; // success, 'dispatched'
}

static int executor_not_running(const char *exid)
{
  if (flu_fstat("var/run/%s.pid", exid) == '\0') return 1;

  char *spid = flu_readall("var/run/%s.pid", exid);
  if (spid == NULL) return 1;

  pid_t pid = strtoll(spid, NULL, 10); free(spid);
  if (pid == 0) return 1;

  if (kill(pid, 0) != 0) return 1;
    // it might be a zombie though...

  return 0;
}

static short dispatch(const char *fname, fdja_value *j)
{
  //flu_putf(fdja_todc(j));

  if (fdja_l(j, "point") == NULL) return -1;

  int r = 2; // 'dispatched' for now

  char *exid = fdja_ls(j, "exid", NULL);
  char *fep = flon_exid_path(exid);
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
    logpath = flu_sprintf("var/log/%s/inv_%s-%s.log", fep, exid, nid);
  }
  else // execute, receive
  {
    logpath = flu_sprintf("var/run/%s/exe.log", fep);
  }

  if (flu_move("var/spool/dis/%s", fname, "var/spool/%s/%s", ct, fname) != 0)
  {
    fgaj_r("failed to move %s to var/spool/%s/%s", fname, ct, fname);
    r = -1; // triggers rejection
  }

  //fgaj_d("2f: %s, %s, %s", ctx, logpath, arg);

  if (r == 2 && executor_not_running(exid))
  {
    r = double_fork(ctx, logpath, arg);
  }

  // over

  if (*fname == 'i') free(arg); // else arg == exid
  free(exid);
  free(fep);
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

static short receive_ret(const char *fname)
{
  short r = 2;

  fdja_value *i = NULL;
  fdja_value *j = NULL;

  i = flon_parse_nid(fname);
  if (i == NULL) { r = -1; goto _over; }
    // TODO: move that check upstream

  j = flon_try_parse('o', "var/spool/dis/%s", fname);

  //flu_putf(fdja_todc(j));
  //if (j == NULL) fgaj_i("NULL: %s", fname);

  if (j == NULL) { r = 1; goto _over; }
    // the file's mtime will get examined

  fdja_psetv(i, "point", "receive");
  fdja_set(i, "payload", j);

  //flu_putf(fdja_todc(i));

    // no need to lock file when writing, since we're in the reader...
    //
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

// returns
//
//   -1 rejected
//    0 not seen
//    1 seen
//    2 dispatched
//
short flon_dispatch(const char *fname)
{
  //fgaj_i(fname);

  int r = 1;
  fdja_value *msg = NULL;

  if ( ! flu_strends(fname, ".json")) { r = -1; goto _over; }

  if (strncmp(fname, "ret_", 4) == 0) { r = receive_ret(fname); goto _over; }

  if (
    strncmp(fname, "exe_", 4) != 0 &&
    strncmp(fname, "inv_", 4) != 0 &&
    strncmp(fname, "rcv_", 4) != 0 &&
    strncmp(fname, "sch_", 4) != 0
  ) { r = -1; goto _over; }

  msg = flon_try_parse('o', "var/spool/dis/%s", fname);

  if (msg == NULL) { r = (errno == 0) ? -1 : 1; goto _over; }

  // TODO reroute?

  if (*fname == 's')
  {
    r = schedule(fname, msg);
  }
  else
  {
    r = dispatch(fname, msg);
    //r = route_or_dispatch(fname, msg);
  }

_over:

  if (r == 1)
  {
    char *path = flu_sprintf("var/spool/dis/%s", fname);
    struct stat sta;

    if (stat(path, &sta) != 0)
    {
      fgaj_r("failed to stat var/spool/dis/%s, rejecting...", fname);
      r = -1;
    }
    else
    {
      long long age = flu_gets('s') - sta.st_mtime;
      //fgaj_d("age: %lli", age);

      if (age > 1 * 60 * 60) r = -1; // reject
      else if (age > 2) r = 0; // not seen
      // else r = 1; // seen

      // TODO: make those 2 thresholds configurable (flu_parse_t())
    }

    free(path);
  }

  if (r == -1) r = reject(fname);

  fdja_free(msg);

  //fgaj_d("r: %i", r);

  return r;
}

