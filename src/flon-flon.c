
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
#include <string.h>

#include "flutil.h"
#include "fl_ids.h"
#include "fl_common.h"


static void print_usage(short explain_dir)
{
  fprintf(stderr, "" "\n");
  fprintf(stderr, "# flon-flon" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "## exid generation" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "  flon-flon [-d {dir}] -i {domain}" "\n");
  fprintf(stderr, "" "\n");
  fprintf(stderr, "Generates an exid." "\n");
  fprintf(stderr, "" "\n");
}

int main(int argc, char *argv[])
{
  char *dir = NULL;
  char *domain = NULL;
  short badarg = 0;

  int opt; while ((opt = getopt(argc, argv, "d:i:")) != -1)
  {
    if (opt == 'd') dir = optarg;
    else if (opt == 'i') domain = optarg;
    else badarg = 1;
  }

  if (badarg || domain == NULL) { print_usage(0); return 1; }

  if (dir == NULL)
  {
    dir = flu_canopath(argv[0]);
    *strrchr(dir, '/') = '\0';
    if (strcmp(strrchr(dir, '/') + 1, "bin") != 0) { print_usage(1); return 1; }
    *strrchr(dir, '/') = '\0';
  }

  if (chdir(dir) != 0)
  {
    fprintf(stderr, "couldn't chdir to %s", dir); return 1;
  }
  if (flon_configure(".") != 0)
  {
    fprintf(stderr, "couldn't read %s/etc/flon.json", dir); return 1;
  }

  puts(flon_generate_exid(domain));

  return 0;
}

