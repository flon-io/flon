
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

#include "djan.h"
#include "gajeta.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


static int invoke(const char *path, fdja_value *j, fdja_value *inv)
{
  //printf("invoke() >>>\n%s\n<<<\n", fdja_to_json(j));

  // as quickly as possible discard useless invocations

  pid_t i = fork();

  if (i < 0) { fgaj_r("fork 1 for invoker failed"); return 0; }

  if (i == 0) // child
  {
    pid_t j = fork();

    if (j < 0) { fgaj_r("fork 2 for invoker failed"); _exit(127); }
    if (j != 0) { _exit(0); } // intermediate parent exits immediately

    if (setsid() == -1)
    {
      fgaj_r("setsid() failed");
      _exit(127);
    }

    char *dir = flon_conf_path("_root", ".");
    fgaj_i("dir is >%s<", dir);

    if (chdir(dir) != 0)
    {
      fgaj_r("failed to chdir()");
      _exit(127);
    }

    char *fn = flu_sprintf("var/log/inv/%s", flu_basename(path, ".txt"));

    fflush(stderr);

    if (freopen(fn, "a", stderr) == NULL)
    {
      fgaj_r("failed to reopen stderr to %s", fn);
      _exit(127);
    }
    fgaj_i("pointing invoker stderr to %s", fn);

    char *invoker_bin = flon_conf_string("invoker.bin", "bin/flon-invoker");

    fgaj_i("cmd is >%s %s<", invoker_bin, path);

    fflush(stderr);

    int r = execl(invoker_bin, invoker_bin, path, NULL);

    // fail zone...

    fgaj_r("execl failed (%i)", r);

    _exit(127);
  }
  else { // parent
    fgaj_i("invoker forked");
  }

  return 0; // success
}

static int reject(const char *path, fdja_value *j)
{
  int r = flu_move(path, "var/spool/rejected/");

  if (r == 0) fgaj_i("rejected %s", path);
  else fgaj_r("failed to move %s to var/spool/rejected", path);

  return r;
}

int flon_dispatch(const char *path)
{
  fdja_value *j = fdja_parse_obj_f(path);

  if (j == NULL) return reject(path, j);

  fdja_value *inv = fdja_lookup(j, "invocation");

  // TODO reroute?

  if (inv) return invoke(path, j, inv);

  return reject(path, j);
}

