
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

#include <stdlib.h>
#include <unistd.h>

#include "gajeta.h"
#include "shervin.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_listener.h"


static void print_usage()
{
  fprintf(stderr, "" "\n");
  fprintf(stderr, "# flon-listener" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  flon-listener [-d {dir}] [-p {port}]" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "starts the flon-listener" "\n");
  fprintf(stderr, "" "\n");
}

int main(int argc, char *argv[])
{
  // read options

  int port = -1;
  char *dir = NULL;
  short badarg = 0;

  int opt; while ((opt = getopt(argc, argv, "d:p:")) != -1)
  {
    if (opt == 'd') dir = optarg;
    else if (opt == 'p') port = strtol(optarg, NULL, 10);
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

  // configure

  if (flon_configure(".") != 0)
  {
    fgaj_r("couldn't read %s/etc/flon.json, cannot start", dir);
    return 1;
  }

  flon_setup_logging("listener");

  free(dir);

  fshv_route *routes[] =
  {
    fshv_r(
      NULL,
      fshv_basic_auth_filter,
      "func", flon_auth_enticate, "realm", "flon", NULL),

    fshv_rp("GET /i", flon_i_handler, NULL),
    fshv_rp("POST /i/in", flon_in_handler, NULL),
    fshv_rp("GET /i/executions", flon_exes_handler, NULL),
    fshv_rp("GET /i/executions/:id", flon_exe_handler, NULL),
    fshv_rp("GET /i/executions/:id/:sub", flon_exe_sub_handler, NULL),
    fshv_rp("GET /i/msgs/:id", flon_msg_handler, NULL),
    fshv_rp("GET /i/metrics", flon_metrics_handler, NULL),
    fshv_rp("GET /**", fshv_dir_handler, "r", "var/www", NULL),

    NULL
  };

  // serve

  if (port < 0) port = 1980;

  fshv_serve(port, routes);
}

