
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

#include <dollar.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/file.h>

#include "flutil.h"
#include "gajeta.h"
#include "djan.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_tasker.h"


typedef struct { char *exid; char *nid; fdja_value *pl; } lup;

static char *lookup(void *data, const char *path)
{
  lup *lu = (lup *)data;

  if (strcmp(path, "exid") == 0) return strdup(lu->exid);
  if (strcmp(path, "nid") == 0) return strdup(lu->nid);

  if (strncmp(path, "pl.", 3) == 0)
    return fdja_ls(lu->pl, path + 3, NULL);
  if (strncmp(path, "payload.", 8) == 0)
    return fdja_ls(lu->pl, path + 8, NULL);

  return NULL;
}

static char *expand(char *cmd, char *exid, char *nid, fdja_value *payload)
{
  return fdol_quote_expand(cmd, &(lup){ exid, nid, payload }, lookup);
}

char *flon_lookup_tasker_path(
  const char *domain, const char *taskee, short created)
{
  size_t l = strlen(domain);

  while (1)
  {
    if (created)
    {
      char *path = flu_sprintf("usr/local/tsk/%.*s/_", l, domain);
      if (flu_fstat(path) == 'd')
      {
        if (flu_fstat("%s/flon.json", path) != 'f')
          created = 0; // stop looking for an _ offerer
        else
          return path;
      }
      free(path);
    }

    char *path = flu_sprintf("usr/local/tsk/%.*s/%s", l, domain, taskee);
    if (flu_fstat(path) == 'd') return path;
    free(path);

    do --l; while (l > 0 && domain[l] != '.');
    if (l == 0) break;
  }

  char *r = flu_sprintf("usr/local/tsk/any/%s", taskee);
  if (flu_fstat(r) == 'd') return r;

  free(r);

  return NULL;
}

static void fail(
  const char *path, fdja_value *tsk, short r, const char *msg)
{
  if (r) fgaj_r(msg); else fgaj_e(msg);

  fdja_set(tsk, "tstate", fdja_v("failed"));
  fdja_set(tsk, "ton", fdja_v("offer"));
  fdja_set(tsk, "msg", fdja_s(msg));

  if (path)
  {
    char *fname = strrchr(path, '/');

    if (flon_lock_write(tsk, "var/spool/dis/%s", fname) != 1)
    {
      fgaj_r("failed to write 'failed' tsk_ msg to var/spool/dis/%s", fname);
    }
  }
  else
  {
    fdja_putj(tsk);
  }
}

static void failf(
  const char *path, fdja_value *tsk, short r, const char *format, ...)
{
  va_list ap; va_start(ap, format); char *msg = flu_sv(format, ap); va_end(ap);

  fail(path, tsk, r, msg);

  free(msg);
}

static void failo(
  fdja_value *tsk, short r, const char *format, ...)
{
  va_list ap; va_start(ap, format); char *msg = flu_sv(format, ap); va_end(ap);

  fail(NULL, tsk, r, msg);

  free(msg);
}

static void reject(const char *path, short r, const char *format, ...)
{
  va_list ap; va_start(ap, format); char *msg = flu_sv(format, ap); va_end(ap);

  if (r) fgaj_r(msg); else fgaj_e(msg);

  flon_move_to_rejected(path, msg);

  free(msg);
}

int flon_task(const char *path)
{
  int r = 0;

  fgaj_d("path: %s", path);

  char *fname = strrchr(path, '/');

  fdja_value *id = flon_parse_nid(fname);

  if (id == NULL)
  {
    reject(path, 0, "couldn't identify tsk at %s", fname);
    return 1;
  }

  char *exid = fdja_ls(id, "exid", NULL);
  char *nid = fdja_ls(id, "nid", NULL);
  char *domain = flon_exid_domain(exid);

  fdja_free(id);

  fdja_value *tsk = NULL;
  fdja_value *tasker_conf = NULL;

  char *taskee = NULL;
  char *tasker_path = NULL;
  char *ret = NULL;
  char *cmd = NULL;

  tsk = fdja_parse_obj_f(path);

  if (tsk == NULL)
  {
    reject(path, 1, "couldn't parse tsk at %s", fname);
    r = 1; goto _over;
  }

// HERE

  fdja_value *tstate = fdja_l(tsk, "tstate", NULL);
  short created = tstate != NULL && fdja_strcmp(tstate, "created") == 0;

  taskee = fdja_ls(tsk, "taskee", NULL);
  tasker_path = flon_lookup_tasker_path(domain, taskee, created);

  fgaj_d("tasker_path: %s", tasker_path);

  if (tasker_path == NULL)
  {
    failf(path, tsk, 0, "didn't find tasker '%s' (domain %s)", taskee, domain);
    r = 1; goto _over;
  }

  tasker_conf = fdja_parse_obj_f("%s/flon.json", tasker_path);

  if (tasker_conf == NULL)
  {
    failf(path, tsk, 1, "didn't find tasker conf at %s/flon.json", tasker_path);
    r = 1; goto _over;
  }

  if (fdja_lk(tasker_conf, "out") == 'd') // discard
    ret = strdup("/dev/null");
  else
    ret = flu_sprintf("var/spool/dis/tsk_%s-%s.json", exid, nid);

  char cwd[1024 + 1]; getcwd(cwd, 1024);
  fgaj_i("cwd: %s", cwd);

  fgaj_i("exid: %s, nid: %s, domain: %s", exid, nid, domain);
  fgaj_i("tasker at %s", tasker_path);

  cmd = fdja_ls(tasker_conf, "run", NULL); // was "invoke"

  if (cmd == NULL)
  {
    failf(
      path, tsk, 0, "no 'run' key in tasker conf at %s/flon.json", tasker_path);
    r = 1; goto _over;
  }

// if cmd is xxx.rad, then run "inline" (re-dispatch self?)

  fdja_value *payload = fdja_lookup(tsk, "payload");
  if (payload == NULL) payload = fdja_object_malloc();

  if (strstr(cmd, "$("))
  {
    char *cmd1 = expand(cmd, exid, nid, payload);
    free(cmd);
    cmd = cmd1;
  }

  fgaj_i("about to run >%s< for taskee '%s'", cmd, taskee);

  int pds[2];

  if (pipe(pds) != 0)
  {
    failf(path, tsk, 1, "failed to setup pipe between taskmaster and tasker");
    r = 1; goto _over;
  }

  //
  // let's fork

  pid_t i = fork();

  if (i < 0) // failure
  {
    failf(path, tsk, 1, "taskmaster failed to fork tasker");
    r = 1; goto _over;
  }
  else if (i == 0) // child
  {
    fgaj_i("child, pid %i", getpid());

    close(pds[1]);
    dup2(pds[0], STDIN_FILENO);
    close(pds[0]);

    if (setsid() == -1)
    {
      failf(path, tsk, 1, "setsid() failed");
      r = 127; goto _over;
    }

    if (freopen(ret, "w", stdout) == NULL)
    {
      failf(path, tsk, 1, "failed to reopen child stdout to %s", ret);
      r = 127; goto _over;
    }
    if (flock(STDOUT_FILENO, LOCK_NB | LOCK_EX) != 0)
    {
      failf(path, tsk, 1, "couldn't lock %s", ret);
      r = 127; goto _over;
    }

    if (chdir(tasker_path) != 0)
    {
      failo(tsk, 1, "failed to chdir to %s", tasker_path);
      r = 127; goto _over;
    }

    fflush(stderr);

    int er = execl("/bin/sh", "", "-c", cmd, NULL);

    // excl has returned... fail zone...

    fgaj_r("execl failed (%i)", er);

    r = 127; //goto _over;
  }
  else // parent
  {
    close(pds[0]);

    FILE *f = fdopen(pds[1], "w");

    char in = fdja_lk(tasker_conf, "in");

    if (in == 'a') // "all"
      fdja_to_j(f, tsk, 0);
    else
      fdja_to_j(f, payload, 0);

    fclose(f);

    // over, no wait

    fgaj_i("tasker %s ran >%s< pid %i", taskee, cmd, i);
  }

_over:

  // resource cleanup
  //
  // not really necessary, but it helps when debugging / checking...
  // 0 lost, like all the others (hopefully).

  fdja_free(tsk);
  fdja_free(tasker_conf);

  free(exid);
  free(nid);
  free(domain);
  free(tasker_path);
  free(cmd);
  free(taskee);
  free(ret);

  return r;
}

