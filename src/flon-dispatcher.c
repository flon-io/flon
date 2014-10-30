
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
#include <unistd.h>
#include <dirent.h>

#include <ev.h>

#include "gajeta.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


static void scan_dir()
{
  fgaj_i(".");

  DIR *dir = opendir("var/spool/dis/");
  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if (*de->d_name == '.') continue;
    flon_dispatch(de->d_name);
  }

  closedir(dir);

  //fgaj_i("> scanning over.");
}

static void spool_cb(struct ev_loop *loop, ev_stat *w, int revents)
{
  //fgaj_i(".");
  scan_dir();
  flu_do_msleep(1050);
  scan_dir();
}

int main(int argc, char *argv[])
{
  // change dir

  char *d = ".";
  if (argc > 1) d = argv[1];

  if (chdir(d) != 0) { fgaj_r("couldn't chdir to %s", d); return 1; }
  char *cp = flu_canopath(d); fgaj_i("changed dir to %s", cp); free(cp);

  // load configuration

  flon_configure(".");

  // set up logging

  flon_setup_logging("dispatcher");

  // scan once

  scan_dir();

  // then, ev...

  struct ev_loop *l = ev_default_loop(0);

  // watch var/spool/dis/

  ev_stat est;
  ev_stat_init(&est, spool_cb, "var/spool/dis/", 0.);
  ev_stat_start(l, &est);

  // check from time to time too

  //ev_periodic epe;
  //ev_periodic_init (&epe, spool_cb, 0., .5, 0);
  //ev_periodic_start (l, &epe);

  // loop

  //fgaj_i("about to ev_loop...");

  ev_loop(l, 0);

  fgaj_r("something went wrong");
}

