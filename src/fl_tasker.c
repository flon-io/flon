
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

char *flon_lookup_tasker(const char *domain, const char *name)
{
  size_t l = strlen(domain);

  while (1)
  {
    char *path = flu_sprintf("usr/local/tsk/%.*s/%s", l, domain, name);
    if (flu_fstat(path) == 'd') return path;
    free(path);
    do --l; while (l > 0 && domain[l] != '.');
    if (l == 0) break;
  }

  char *r = flu_sprintf("usr/local/tsk/any/%s", name);
  if (flu_fstat(r) == 'd') return r;

  free(r);

  return NULL;
}

int flon_task(const char *path)
{
  fgaj_d("path: %s", path);

  fdja_value *tsk = fdja_parse_obj_f(path);

  if (tsk == NULL)
  {
    fgaj_r("couldn't read tsk msg at %s", path); return 1;
  }

  fdja_value *tree = fdja_lookup(tsk, "tree");

  if (tree == NULL)
  {
    fgaj_e("no 'tree' key in the message"); return 1;
  }

  fdja_value *payload = fdja_lookup(tsk, "payload");
  if (payload == NULL) payload = fdja_object_malloc();

  char *exid = fdja_ls(tsk, "exid", NULL);
  char *nid = fdja_ls(tsk, "nid", NULL);

  char *domain = flon_exid_domain(exid);

  char *tasker_name = fdja_ls(tree, "1._0", NULL);
  char *tasker_path = flon_lookup_tasker(domain, tasker_name);

  if (tasker_path == NULL)
  {
    fgaj_r("didn't find tasker %s (domain %s)", tasker_name, domain);
    return 1;
  }

  char *ret = flu_sprintf("var/spool/dis/ret_%s-%s.json", exid, nid);

  char cwd[1024 + 1]; getcwd(cwd, 1024);
  fgaj_i("cwd: %s", cwd);

  fgaj_i("exid: %s, nid: %s, domain: %s", exid, nid, domain);
  fgaj_i("tasker at %s", tasker_path);

  fdja_value *tasker_conf = fdja_parse_obj_f("%s/flon.json", tasker_path);

  if (tasker_conf == NULL)
  {
    fgaj_r("didn't find tasker conf at %s/flon.json", tasker_path);
    return 1;
  }

  char *cmd = fdja_ls(tasker_conf, "run", NULL); // was "invoke"

  char *out = fdja_ls(tasker_conf, "out", NULL);
  if (out && strcmp(out, "discard") == 0)
  {
    free(ret); ret = strdup("/dev/null");
  }
  free(out);

  if (cmd == NULL)
  {
    fgaj_e("no 'run' key in tasker conf at %s/flon.json", tasker_path);
    return 1;
  }

  if (strstr(cmd, "$("))
  {
    char *cmd1 = expand(cmd, exid, nid, payload);
    free(cmd);
    cmd = cmd1;
  }

  fgaj_i("tasker %s running >%s<", tasker_name, cmd);

  int pds[2];

  int r = pipe(pds);

  if (r != 0)
  {
    fgaj_r("failed to setup pipe between taskmaster and tasker");
    return 1;
  }

  pid_t i = fork();

  if (i < 0) // failure
  {
    fgaj_r("taskmaster failed to fork tasker");
  }
  else if (i == 0) // child
  {
    fgaj_i("child, pid %i", getpid());

    close(pds[1]);
    dup2(pds[0], STDIN_FILENO);
    close(pds[0]);

    if (setsid() == -1)
    {
      fgaj_r("setsid() failed");
      return 127;
    }

    if (freopen(ret, "w", stdout) == NULL)
    {
      fgaj_r("failed to reopen child stdout to %s", ret);
      return 127;
    }
    if (flock(STDOUT_FILENO, LOCK_NB | LOCK_EX) != 0)
    {
      fgaj_r("couldn't lock %s", ret);
      return 127;
    }

    if (chdir(tasker_path) != 0)
    {
      fgaj_r("failed to chdir to %s", tasker_path);
      return 127;
    }

    fflush(stderr);

    r = execl("/bin/sh", "", "-c", cmd, NULL);

    // excl has returned... fail zone...

    fgaj_r("execl failed (%i)", r);

    return 127;
  }
  else // parent
  {
    close(pds[0]);

    FILE *f = fdopen(pds[1], "w");
    fdja_to_j(f, payload, 0);
    fclose(f);

    // over, no wait

    fgaj_i("tasker %s ran >%s< pid %i", tasker_name, cmd, i);
  }

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
  free(tasker_name);
  free(ret);

  // exit

  return 0;
}

