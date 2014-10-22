
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
#include <unistd.h>
#include <dirent.h>

#include <ev.h>

#include "gajeta.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


static void scan_dir()
{
  fgaj_i("scanning var/spool/dis/");

  DIR *dir = opendir("var/spool/dis/");
  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if (*de->d_name == '.') continue;
    flon_dispatch(de->d_name);
  }

  closedir(dir);
}

static void spool_cb(struct ev_loop *loop, ev_stat *w, int revents)
{
  if (w->attr.st_nlink) scan_dir();
}

int main(int argc, char *argv[])
{
  // set up logging

  fgaj_conf_get()->out = stderr;

  // change dir

  char *d = ".";
  if (argc > 1) d = argv[1];

  if (chdir(d) != 0) { fgaj_r("couldn't chdir to %s", d); return 1; }

  // load configuration

  flon_configure(".");

  // scan once

  scan_dir();

  // then watch

  struct ev_loop *l = ev_default_loop(0);
  ev_stat est;

  ev_stat_init(&est, spool_cb, "var/spool/dis/", 0.);
  ev_stat_start(l, &est);

  ev_loop(l, 0);
}

