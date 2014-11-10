
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

#include "flutil.h"
#include "gajeta.h"
#include "shervin.h"
#include "shv_protected.h"
#include "fl_common.h"


//static int root_handler(shv_request *req, shv_response *res, flu_dict *params)
//{
//  res->status_code = 200;
//  flu_list_add(res->body, strdup("hello world"));
//  return 1;
//}

static void print_usage()
{
  fprintf(stderr, "" "\n");
  fprintf(stderr, "# flon-listener" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  flon-listener [-d {dir}]" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "starts the flon-listener" "\n");
  fprintf(stderr, "" "\n");
}

int main(int argc, char *argv[])
{
  // read options

  int port = 1980;
  char *dir = NULL;

  short badarg = 0;

  int opt; while ((opt = getopt(argc, argv, "d:")) != -1)
  {
    if (opt == 'd') dir = optarg;
    else badarg = 1;
  }

  if (badarg) { print_usage(); return 1; }

  // configure

  if (dir == NULL) dir = ".";

  if (chdir(dir) != 0)
  {
    fprintf(stderr, "couldn't chdir to %s", dir); return 1;
  }
  if (flon_configure(".") != 0)
  {
    perror(
      flu_sprintf(
        "couldn't read %s/etc/flon.json, cannot start", flu_canopath(dir)));
    return 1;
  }

  flon_setup_logging("listener");

  char *cp = flu_canopath(dir); fgaj_i("-d %s", cp); free(cp);

  shv_route *routes[] =
  {
    shv_rp("/**", shv_dir_handler, "r", "var/www", NULL),
    NULL
  };

  // serve

  shv_serve(port, routes);
}

