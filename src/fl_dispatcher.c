
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
//#include <libgen.h>

#include "flutil.h"
#include "djan.h"
#include "gajeta.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


static int double_fork(char *ctx, char *log_path, char *argv[])
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

    if (freopen(log_path, "a", stderr) == NULL)
    {
      fgaj_r("failed to reopen stderr to %s", log_path);
      _exit(127);
    }
    fgaj_i("pointing invoker stderr to %s", log_path);

    fgaj_i("cmd is >%s %s<", argv[0], argv[1]);

    fflush(stderr);

    int r = execv(argv[0], argv);

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

static int invoke(const char *fname)
{
  char *bin = flon_conf_string("invoker.bin", "bin/flon-invoker");
  char *basename = flu_basename(fname, ".txt");

  char *invpath = flu_sprintf("var/spool/inv/%s", fname);
  char *logpath = flu_sprintf("var/log/inv/%s", basename);

  free(basename);

  if (flu_move("var/spool/dis/%s", fname, invpath) != 0)
  {
    fgaj_r("failed to move %s to %s", fname, invpath);
    return -1; // triggers rejection
  }

  int r = double_fork(
    "invoker", logpath, (char *[]){ bin, (char *)invpath, NULL });

  free(bin);
  free(invpath);
  free(logpath);

  return r;
}

static int execute(const char *fname, fdja_value *j)
{
  return 1; // failure
}

static int reject(const char *fname)
{
  int r = flu_move("var/spool/dis/%s", fname, "var/spool/rejected/");

  if (r == 0) fgaj_i(fname);
  else fgaj_r("failed to move %s to var/spool/rejected/", fname);

  return 1; // failure
}

static int dispatch(const char *fname, fdja_value *j)
{
  if (j == NULL) return -1;
  if (fdja_l(j, "invoke")) return invoke(fname);
  if (fdja_l(j, "execute")) return execute(fname, j);
  return -1;
}

int flon_dispatch(const char *fname)
{
  fgaj_i(fname);

  int r = 0;
  fdja_value *j = NULL;

  if ( ! flu_strends(fname, ".json")) r = -1;

  if (
    r == 0 &&
    strncmp(fname, "exe_", 4) != 0 &&
    strncmp(fname, "inv_", 4) != 0 &&
    strncmp(fname, "ret_", 4) != 0 &&
    strncmp(fname, "rcv_", 4) != 0
  ) r = -1;

  if (r == 0) j = fdja_parse_obj_f("var/spool/dis/%s", fname);

  // TODO reroute?

  if (r == 0) r = dispatch(fname, j);
  if (r == -1) r = reject(fname);

  if (j) fdja_value_free(j);

  return r;
}

