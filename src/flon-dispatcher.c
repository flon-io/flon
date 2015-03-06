
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <ev.h>

#include "flutim.h"
#include "gajeta.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


int scan_dir_count = 0;


static void print_usage()
{
  fprintf(stderr, "" "\n");
  fprintf(stderr, "# flon-dispatcher" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  flon-dispatcher [-d {dir}]" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "starts the flon-dispatcher" "\n");
  fprintf(stderr, "" "\n");
}

static size_t scan_dir()
{
  //fgaj_d(". sdc: %i", scan_dir_count);

  size_t seen = 0;
  size_t dispatched = 0;

  DIR *dir = opendir("var/spool/dis/");
  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if (*de->d_name == '.') continue;

    ++seen;

    short r = flon_dispatch(de->d_name);
    if (r > 0) ++dispatched;

    //fgaj_d("flon_dispatch: %i", r);
  }

  closedir(dir);

  //fgaj_i("> scanning over.");
  if (seen > 0) fgaj_d("dispatched: %zu/%zu", dispatched, seen);

  return dispatched;
}

static void spool_cb(struct ev_loop *loop, ev_stat *w, int revents)
{
  if (EV_ERROR & revents) { fgaj_r("invalid event"); return; }
    // TODO: shutdown flon-dispatcher

  scan_dir_count = 3;
}

static void trigger_cb(struct ev_loop *loop, ev_periodic *ep, int revents)
{
  if (EV_ERROR & revents) { fgaj_r("invalid event"); return; }
    // TODO: shutdown flon-dispatcher

  flon_trigger(ev_now(loop));

  if (scan_dir_count < 1) return;

  scan_dir_count += scan_dir() < 1 ? -1 : 1;
}

static ev_tstamp trigger_reschedule_cb(ev_periodic *ep, ev_tstamp now)
{
  return now + (scan_dir_count > 0 ? .07 : .32);
}

static void sighup_cb(struct ev_loop *loop, ev_signal *es, int revents)
{
  if (EV_ERROR & revents) { fgaj_r("invalid event"); return; }
    // TODO: shutdown flon-dispatcher

  fgaj_i("received SIGHUP");

  flon_load_timers();

  // TODO reload configuration and reset logging as well
}

int main(int argc, char *argv[])
{
  // read options

  char *dir = NULL;
  short badarg = 0;

  int opt; while ((opt = getopt(argc, argv, "d:")) != -1)
  {
    if (opt == 'd') dir = optarg;
    else badarg = 1;
  }

  if (badarg) { print_usage(); return 1; }

  // change dir

  dir = flon_path(argv[0], dir);

  if (chdir(dir) != 0)
  {
    fgaj_r("couldn't chdir to %s", dir);
    return 1;
  }

  fgaj_i("-d %s", dir);

  // load configuration

  if (flon_configure(".") != 0)
  {
    fgaj_r("couldn't read %s/etc/flon.json, cannot start", dir);
    return 1;
  }

  free(dir);

  // set up logging

  flon_setup_logging("dispatcher");

  // scan once

  scan_dir();

  // load timers

  flon_load_timers();

  // then, ev...

  struct ev_loop *l = ev_default_loop(0);

  // watch var/spool/dis/

  ev_stat est;
  ev_stat_init(&est, spool_cb, "var/spool/dis/", 0.);
  ev_stat_start(l, &est);

  // check from time to time too

  ev_periodic epe;
  ev_periodic_init(&epe, trigger_cb, 0., .35, trigger_reschedule_cb);
  ev_periodic_start(l, &epe);

  //ev_timer eti;
  //ev_timer_init(&eti, do_something_when_loop_ready_cb, 0., 0.);
  //ev_timer_start(l, &eti);

  ev_signal esi;
  ev_signal_init(&esi, sighup_cb, SIGHUP);
  ev_signal_start(l, &esi);

  // loop

  //fgaj_i("about to ev_loop...");

  ev_loop(l, 0);

  fgaj_r("something went wrong");
}

