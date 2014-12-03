
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

void flon_empty_timers()
{
  flu_list_and_items_free(at_timers, (void (*)(void *))flon_timer_free);
  flu_list_and_items_free(cron_timers, (void (*)(void *))flon_timer_free);

  at_timers = flu_list_malloc();
  cron_timers = flu_list_malloc();
}

static int at_cmp(const void *ta, const void *tb)
{
  return strcmp(((flon_timer *)ta)->ts, ((flon_timer *)tb)->ts);
}
//static int cron_cmp(const void *ta, const void *tb)
//{
//  // high frequency first... // MAYBE
//}

static void add_at_timer(const char *ts, ssize_t l, const char *fn)
{
  if (at_timers == NULL) flon_load_timers();

  flon_timer *t = calloc(1, sizeof(flon_timer));
  t->ts = l < 1 ? strdup(ts) : strndup(ts, l);
  t->fn = strdup(fn);

  flu_list_oinsert(at_timers, t, at_cmp);

  fgaj_d("ts: %s, fn: %s", t->ts, t->fn);
}

static void add_cron_timer(const char *ts, const char *fn)
{
  if (at_timers == NULL) flon_load_timers();

  flon_timer *t = calloc(1, sizeof(flon_timer));
  t->ts = strdup(ts);
  t->fn = strdup(fn);

  flu_list_add(cron_timers, t);
    // simply add at the end

  //flu_list_oinsert(cron_timers, t, cron_cmp);
    // at some point, one could think of adding high frequency first,
    // low frequency last, to help determine the scheduling frequency...
}

static void remove_timer(flu_list *l, const char *ts, const char *fn)
{
  for (flu_node **link = &l->first; *link; link = &(*link)->next)
  {
    flu_node *n = *link;
    flon_timer *t = n->item;

    if (strcmp(t->ts, ts) != 0 || strcmp(t->fn, fn) != 0) continue;

    *link = (*link)->next;
    flon_timer_free(t);
    flu_node_free(n);
    l->size--;

    break;
  }
}

void flon_load_timers()
{
  flon_empty_timers();

  flu_list *l = flon_find_json("var/spool/tdis");

  for (flu_node *n = l->first; n; n = n->next)
  {
    char *fn = strrchr((char *)n->item, '/'); if (fn == NULL) continue;

    char *a = strchr(fn, '-'); if (a == NULL) continue;
    char *b = strchr(a + 1, '-'); if (b == NULL) continue;

    if (fn[1] == 'a')
    {
      add_at_timer(a + 1, b - a - 1, ((char *)n->item) + 15);
    }
    else
    {
      char *ts = flu64_decode(a + 1, b - a - 1);
      add_cron_timer(ts, ((char *)n->item) + 15);
      free(ts);
    }
  }

  flu_list_free_all(l);

  fgaj_i("at: %zu, cron: %zu", at_timers->size, cron_timers->size);
  if (at_timers->first)
  {
    fgaj_i("next at: %s", ((flon_timer *)at_timers->first->item)->ts);
  }
}

static void move_to_rejected(const char *path, ...)
{
  va_list ap; va_start(ap, path);
  char *pa = flu_svprintf(path, ap);
  char *re = flu_svprintf(va_arg(ap, char *), ap);
  va_end(ap);

  if (strncmp(pa, "var/", 4) != 0)
  {
    char *opa = pa; pa = flu_path("var/spool/dis/%s", pa); free(opa);
  }

  FILE *f = fopen(pa, "a");
  if (f)
  {
    char *ts = flu_tstamp(NULL, 1, 'u');
    fprintf(f, "\n# reason: %s (%s Z)\n", re, ts);
    fclose(f);
    free(ts);
  }

  int mr = flu_move(pa, "var/spool/rejected/");

  if (mr == 0)
    fgaj_i("%s, reason is: %s", pa, re);
  else
    fgaj_r("failed moving %s to var/spool/rejected, reason was: %s", pa, re);

  free(pa);
  free(re);
}

static void move_to_processed(char *fep, const char *dformat, const char *fn)
{
  //printf("%s, %s, %s\n", fep, dformat, fn);

  char *d = flu_fstat("var/run/%s/processed", fep) != 'd' ? "archive" : "run";

  int r = flu_move(dformat, fn, "var/%s/%s/processed", d, fep);
  if (r == 0) return;

  fgaj_r(
    "failed to move %s to var/%s/%s/processed/", fn, d, fep);

  move_to_rejected(
    dformat, fn, "couldn't move to var/%s/%s/processed/", d, fep);
}

static short schedule(const char *fname, fdja_value *msg)
{
  int r = 1; // seen, failed, for now

  int unschedule = (*fdja_srk(fdja_l(msg, "point")) == 'u');

  fgaj_d("%sschedule order: %s", unschedule ? "un" : "", fname);

  char *exid = flon_parse_exid(fname);
  char *fep = flon_exid_path(exid);

  // write to var/spool/tdis/

  char *type = "at";
  char *ts = fdja_ls(msg, "at", NULL);
  if (ts == NULL) { type = "cron"; ts = fdja_ls(msg, "cron", NULL); }

  if (ts == NULL)
  {
    r = -1; move_to_rejected(fname, "no 'at' or 'cron'"); goto _over;
  }

  char *ots = ts;
  if (*type == 'c') ts = flu64_encode(ts, -1);

  char *fn = flu_sprintf("%s/%s-%s-%s", fep, type, ts, fname + 4);

  if (unschedule) // remove schedule
  {
    if (flu_unlink("var/spool/tdis/%s", fn) != 0)
    {
      fgaj_r("failed to unlink %s", fn); goto _over;
    }

    flu_prune_empty_dirs("var/spool/tdis");

    // unlist from timer index

    remove_timer(*type == 'c' ? cron_timers : at_timers, ots, fn);
  }
  else // add schedule
  {
    if (flu_mkdir_p("var/spool/tdis/%s", fep, 0755) != 0)
    {
      fgaj_r("failed to mkdir var/spool/tdis/%s/", fep); goto _over;
    }

      // directly write the source of the msg to file
    if (flu_writeall("var/spool/tdis/%s", fn, msg->source) != 1)
    {
      fgaj_r("failed to write %s", fn); goto _over;
    }

    // list in timer index

    if (*type == 'c') add_cron_timer(ots, fn);
    else add_at_timer(ots, -1, fn);
  }

  // move to processed/

  move_to_processed(fep, "var/spool/dis/%s", fname); // ignore result

  r = 2; // success

_over:

  if (ots != ts) free(ots);
  free(ts);

  free(fn);
  free(fep);
  free(exid);

  return r;
}

static int do_trigger(const char *ns)
{
  //fgaj_d("ns: %s", ns);

  int r = 0;

  char *fep = NULL;
  fdja_value *sch = NULL;
  char *point = NULL;
  char *prefix = NULL;
  char *exid = NULL;
  char *nid = NULL;
  char *fn = NULL;
  flon_timer *t = NULL;

  if (at_timers->first == NULL) goto _over;

  t = at_timers->first->item;

  if (strcmp(t->ts, ns) > 0) goto _over;

  r = 1;
  t = flu_list_shift(at_timers); // shift

  fgaj_d(t->fn);
  sch = fdja_parse_obj_f("var/spool/tdis/%s", t->fn);
  //fgaj_d("sch: %p", sch);

  if (sch == NULL)
  {
    move_to_rejected(t->fn, "couldn't parse"); goto _over;
  }

  fep = strdup(t->fn); *(strrchr(fep, '/')) = 0;
  move_to_processed(fep, "var/spool/tdis/%s", t->fn);

  fdja_value *msg = fdja_l(sch, "msg");

  if (msg == NULL)
  {
    move_to_rejected(t->fn, "doesn't contain 'msg' key"); goto _over;
  }

  point = fdja_ls(msg, "point", NULL);
  prefix = flon_point_to_prefix(point);
  exid = fdja_ls(msg, "exid", NULL);
  nid = fdja_ls(msg, "nid", NULL);

  fn = flu_sprintf("var/spool/dis/%s%s-%s.json", prefix, exid, nid);

  fdja_set(msg, "trigger", fdja_v("{}"));
  fdja_pset(msg, "trigger.now", fdja_s(ns));
  fdja_pset(msg, "trigger.ts", fdja_s(t->ts));
  fdja_pset(msg, "trigger.fn", fdja_s(t->fn));

  if (fdja_to_json_f(msg, fn) != 1)
  {
    fgaj_r("failed to place msg from %s to %s", t->fn, fn);
  }

_over:

  free(fep); free(fn); free(nid); free(exid); free(point);
  fdja_free(sch);
  if (r) flon_timer_free(t);

  return r;
}

void flon_trigger(long long now_s)
{
  //fgaj_d("now_s: %lli", now_s);

  if (at_timers->size < 1) return;

  char *ns = flu_sstamp(now_s, 1, 's');
  short moved = 0;

  while (1)
  {
    int tr = do_trigger(ns);
    if (tr > 0) moved = 1; else break;
  }

  if (moved) flu_prune_empty_dirs("var/spool/tdis");

  free(ns);
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

  if (fdja_l(j, "point") == NULL)
  {
    move_to_rejected(fname, "no 'point'"); return -1;
  }

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
    move_to_rejected(fname, "failed to move to var/spool/%s/%s", ct, fname);
    r = -1; // rejected
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
  if (fdja_to_json_f(i, "var/spool/dis/rcv_%s", fname + 4) != 1)
  {
    move_to_rejected(
      fname,
      "failed to move to var/spool/dis/rcv_%s", fname + 4);
    r = -1;
    goto _over;
  }

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
//    0 error
//    1 dispatched
//
short flon_dispatch(const char *fname)
{
  //fgaj_i(fname);

  int r = 1;
  fdja_value *msg = NULL;

  if ( ! flu_strends(fname, ".json"))
  {
    r = -1; move_to_rejected(fname, "not a .json file"); goto _over;
  }

  if (strncmp(fname, "ret_", 4) == 0) { r = receive_ret(fname); goto _over; }

  if (
    strncmp(fname, "exe_", 4) != 0 &&
    strncmp(fname, "inv_", 4) != 0 &&
    strncmp(fname, "rcv_", 4) != 0 &&
    strncmp(fname, "sch_", 4) != 0
  ) {
    r = -1; move_to_rejected(fname, "unknown file prefix"); goto _over;
  }

  msg = flon_try_parse('o', "var/spool/dis/%s", fname);

  if (msg == NULL)
  {
    r = (errno == 0) ? -1 : 1;
    if (r == -1) move_to_rejected(fname, "couldn't parse json");
    goto _over;
  }

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

  fdja_free(msg);

  //fgaj_d("r: %i", r);

  return r;
}

