
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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "flutil.h"
#include "fl_tools.h"


static void print_usage()
{
  puts("");
  puts("# flon-scope");
  puts("");
  puts("  flon-scope [-d {dir}] {exid-fragment}");
  puts("");
  puts("Pretty prints the first execution it finds.");
  puts("looks in {start-dir}/var/run/ first, then in {start-dir}/var/archive/");
  puts("");
}

static char *lookup_exid(char *dir, char *fragment, size_t depth)
{
  //printf("dir: %s, depth: %zu\n", dir, depth);

  char *r = NULL;

  DIR *d = opendir(dir);
  if (d == NULL) return NULL;

  struct dirent *ep;
  while ((ep = readdir(d)) != NULL)
  {
    if (*ep->d_name == '.') continue;

    char *path = flu_sprintf("%s/%s", dir, ep->d_name);
    char s = flu_fstat(path);

    //printf("  * %s %c\n", path, s);

    if (depth < 2 && s == 'd')
    {
      r = lookup_exid(path, fragment, depth + 1);
      if (r) goto _over;
    }
    else if (depth == 2 && s == 'd')
    {
      if (strstr(ep->d_name, fragment))
      {
        r = ep->d_name; goto _over;
      }
    }
  }

_over:

  closedir(d);

  return r;
}

static char *determine_exid(char *fragment)
{
  char *r = NULL;

  r = lookup_exid("var/run/", fragment, 0);
  if (r == NULL) r = lookup_exid("var/archive/", fragment, 0);

  return r;
}

int main(int argc, char *argv[])
{
  char *d = ".";
  short badarg = 0;

  int opt; while ((opt = getopt(argc, argv, "d:")) != -1)
  {
    if (opt == 'd') d = optarg;
    else badarg = 1;
  }

  if (optind >= argc) badarg = 1;
  if (badarg) { print_usage(); return 1; }

  if (chdir(d) != 0) { printf("couldn't chdir to %s", d); return 1; }

  char *fragment = argv[optind];

  char *exid = determine_exid(fragment);

  if (exid == NULL)
  {
    printf("couldn't find an execution matching >%s<\n", fragment);
    return 1;
  }

  flon_pp_execution(exid);

  return 0;
}

