
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
#include "fl_executor.h"


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

typedef struct {
  fdja_value *tsk;
  fdja_value *in;
  fdja_value *tasker_conf;
  char *path;
  char *fname;
  char *exid;
  char *nid;
  char *domain;
  char *taskee;
  char *tasker_path;
  char *cmd;
  char *out;
  int offerer;
} tasking_data;

static void tasking_data_free(tasking_data *td)
{
  fdja_free(td->tsk);
  fdja_free(td->tasker_conf);
  //fdja_free(td->in); // no, since in points to tsk or tsk.payload
  //free(td->path); // no.
  //free(td->fname); // no.
  free(td->exid);
  free(td->nid);
  free(td->domain);
  free(td->taskee);
  free(td->tasker_path);
  free(td->cmd);
  free(td->out);
}

static char *lookup(void *data, const char *path)
{
  tasking_data *td = (tasking_data *)data;

  if (strcmp(path, "exid") == 0) return strdup(td->exid);
  if (strcmp(path, "nid") == 0) return strdup(td->nid);

  fdja_value *payload = fdja_l(td->tsk, "payload");

  if (strncmp(path, "pl.", 3) == 0)
    return fdja_ls(payload, path + 3, NULL);
  if (strncmp(path, "payload.", 8) == 0)
    return fdja_ls(payload, path + 8, NULL);

  return NULL;
}

static void fail(char o_or_f, tasking_data *td, short r, const char *msg)
{
  if (r) fgaj_r(msg); else fgaj_e(msg);

  fdja_psetv(td->tsk, "task.state", "failed");
  fdja_psetv(td->tsk, "task.event", "offering");
  fdja_psetv(td->tsk, "task.from", "tasker");
  fdja_pset(td->tsk, "task.msg", fdja_s(msg));

  if (o_or_f == 'f')
  {
    if (flon_lock_write(td->tsk, "var/spool/dis/%s", td->fname) != 1)
    {
      fgaj_r(
        "failed to write 'failed' tsk_ msg to var/spool/dis/%s", td->fname);
    }
  }
  else
  {
    fdja_putj(td->tsk);
  }
}

static void failf(tasking_data *td, short r, const char *format, ...)
{
  va_list ap; va_start(ap, format); char *msg = flu_sv(format, ap); va_end(ap);

  fail('f', td, r, msg);

  free(msg);
}

static void failo(tasking_data *td, short r, const char *format, ...)
{
  va_list ap; va_start(ap, format); char *msg = flu_sv(format, ap); va_end(ap);

  fail('o', td, r, msg);

  free(msg);
}

static void reject(const char *path, short r, const char *format, ...)
{
  va_list ap; va_start(ap, format); char *msg = flu_sv(format, ap); va_end(ap);

  if (r) fgaj_r(msg); else fgaj_e(msg);

  flon_move_to_rejected(path, msg);

  free(msg);
}

static int run_rad(tasking_data *td)
{
  fgaj_d("cmd >%s< taskee >%s< out >%s<", td->cmd, td->taskee, td->out);

  fdja_value *rad =
    fdja_parse_radial_f("%s/%s", td->tasker_path, td->cmd);

  if (rad == NULL)
  {
    failf(td, 1, "failed to parse %s", td->cmd);
    return 1;
  }

  //fgaj_d("rad: %s", fdja_tod(rad));

  fdja_value *pl = fdja_lc(td->tsk, "payload");

  if (td->offerer)
  {
    fdja_set(pl, "taskee", fdja_s(td->taskee));
    // TODO load task history if required...
    // TODO load taskbox state if required...
  }

  fdja_value *vars = fdja_object_malloc();

  fdja_value *msg = flon_execut(td->domain, rad, pl, vars);

  //fgaj_d("over: %s", fdja_tod(msg));

  if (td->offerer)
  {
    //fdja_psetv(td->tsk, "task.state", "offered");
    //fdja_psetv(td->tsk, "task.event", "offering");
    //fdja_psetv(td->tsk, "task.from", "%s/%s", td->tasker_path, td->cmd);
    fdja_pset(td->tsk, "task.for", fdja_lc(msg, "payload.taskee"));
  }
  else
  {
    fdja_set(td->tsk, "payload", fdja_lc(msg, "payload"));
  }

  fdja_free(msg);

  if (flon_lock_write(td->tsk, "var/spool/dis/%s", td->fname) != 1)
  {
    fgaj_r("failed to write back to var/spool/dis/%s", td->fname);
    return 1;
  }

  return 0;
}

static int run_cmd(tasking_data *td)
{
  fgaj_i(">%s< for taskee '%s'", td->cmd, td->taskee);

  int pds[2];

  if (pipe(pds) != 0)
  {
    failf(td, 1, "failed to setup pipe between taskmaster and tasker");
    return 1;
  }

  //
  // let's fork

  pid_t pid = fork();

  if (pid < 0) // failure
  {
    failf(td, 1, "taskmaster failed to fork tasker");
    return 1;
  }

  if (pid == 0) // child
  {
    fgaj_i("child, pid %i", getpid());

    close(pds[1]);
    dup2(pds[0], STDIN_FILENO);
    close(pds[0]);

    if (setsid() == -1)
    {
      failf(td, 1, "setsid() failed");
      return 127;
    }

    if (freopen(td->out, "w", stdout) == NULL)
    {
      failf(td, 1, "failed to reopen child stdout to %s", td->out);
      return 127;
    }
    if (flock(STDOUT_FILENO, LOCK_NB | LOCK_EX) != 0)
    {
      failf(td, 1, "couldn't lock %s", td->out);
      return 127;
    }

    if (chdir(td->tasker_path) != 0)
    {
      failo(td, 1, "failed to chdir to %s", td->tasker_path);
      return 127;
    }

    fflush(stderr);

    int er = execl("/bin/sh", "", "-c", td->cmd, NULL);

    // excl has returned... fail zone...

    fgaj_r("execl failed (%i)", er);

    return 127;
  }

  // else we're in the parent

  close(pds[0]);

  FILE *f = fdopen(pds[1], "w");

  fdja_to_j(f, td->in, 0);

  fclose(f);

  // over, no wait

  fgaj_i("tasker %s ran >%s< pid %i", td->taskee, td->cmd, pid);

  return 0;
}

static void prepare_tasker_cmd(tasking_data *td)
{
  char cwd[1024 + 1]; getcwd(cwd, 1024);
  //fgaj_i("cwd: %s", cwd);

  fgaj_i("exid: %s, nid: %s, domain: %s", td->exid, td->nid, td->domain);
  fgaj_i("tasker at %s", td->tasker_path);

  td->cmd = fdja_ls(td->tasker_conf, "run", NULL); // was "invoke"

  if (td->cmd == NULL) return;

  if (strstr(td->cmd, "$("))
  {
    char *cmd1 = fdol_quote_expand(td->cmd, td, lookup);
    free(td->cmd);
    td->cmd = cmd1;
  }
}

static void prepare_tasker_output(tasking_data *td)
{
  if (td->offerer == 0 && fdja_lk(td->tasker_conf, "out") == 'd') // discard
    td->out = strdup("/dev/null");
  else
    td->out = flu_sprintf("var/spool/dis/tsk_%s-%s.json", td->exid, td->nid);
}

static void prepare_tasker_input(tasking_data *td)
{
  if (fdja_l(td->tsk, "payload") == NULL)
  {
    fdja_set(td->tsk, "payload", fdja_object_malloc());
  }

  char in = fdja_lk(td->tasker_conf, "in");

  if (in == 'a' || td->offerer)
    td->in = td->tsk;
  else
    td->in = fdja_l(td->tsk, "payload");

  if (td->offerer)
  {
    fdja_psetv(td->tsk, "task.state", "offered");
    fdja_psetv(td->tsk, "task.event", "offering");
    fdja_pset(td->tsk, "task.from", fdja_s("%s/%s", td->tasker_path, td->cmd));
    fdja_psetv(td->tsk, "task.for", td->taskee);
  }
  else //if (in == 'a')
  {
    fdja_psetv(td->tsk, "task.state", "completed");
    fdja_psetv(td->tsk, "task.event", "completion");
    fdja_pset(td->tsk, "task.from", fdja_s("%s/%s", td->tasker_path, td->cmd));
    fdja_psetv(td->tsk, "task.for", td->taskee);
  }
}

int flon_task(const char *path)
{
  int r = 0;

  fgaj_d("path: %s", path);

  tasking_data td; memset(&td, 0, sizeof(tasking_data));

  td.path = (char *)path;
  td.fname = strrchr(path, '/');

  fdja_value *id = flon_parse_nid(td.fname);

  if (id == NULL)
  {
    reject(td.path, 0, "couldn't identify tsk at %s", td.fname);
    return 1;
  }

  td.exid = fdja_ls(id, "exid", NULL);
  td.nid = fdja_ls(id, "nid", NULL);
  td.domain = flon_exid_domain(td.exid);

  fdja_free(id);

  td.tsk = fdja_parse_obj_f(path);

  if (td.tsk == NULL)
  {
    reject(td.path, 1, "couldn't parse tsk at %s", td.fname);
    r = 1; goto _over;
  }

  fdja_value *tstate = fdja_l(td.tsk, "task.state", NULL);
  short created = tstate != NULL && fdja_strcmp(tstate, "created") == 0;

  td.taskee = fdja_ls(td.tsk, "task.for", NULL);
  td.tasker_path = flon_lookup_tasker_path(td.domain, td.taskee, created);

  fgaj_d("tasker_path: %s", td.tasker_path);

  if (td.tasker_path == NULL)
  {
    failf(&td, 0, "didn't find tasker '%s' (domain %s)", td.taskee, td.domain);
    r = 1; goto _over;
  }

  td.tasker_conf = fdja_parse_obj_f("%s/flon.json", td.tasker_path);

  if (td.tasker_conf == NULL)
  {
    failf(&td, 1, "didn't find tasker conf at %s/flon.json", td.tasker_path);
    r = 1; goto _over;
  }

  td.offerer = flu_strends(td.tasker_path, "/_");

  prepare_tasker_cmd(&td);

  if (td.cmd == NULL)
  {
    failf(
      &td, 0, "no 'run' key in tasker conf at %s/flon.json", td.tasker_path);
    r = 1; goto _over;
  }

  prepare_tasker_output(&td);
  prepare_tasker_input(&td);

  if (flu_strends(td.cmd, ".rad"))
    r = run_rad(&td);
  else
    r = run_cmd(&td);

_over:

  // resource cleanup
  //
  // not really necessary, but it helps when debugging / checking...
  // 0 lost, like all the others (hopefully).

  tasking_data_free(&td);

  return r;
}

