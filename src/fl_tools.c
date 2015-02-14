
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
#include <string.h>

#include "flutil.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"


void flon_pp_execution(const char *exid)
{
// Black       0;30     Dark Gray     1;30
// Blue        0;34     Light Blue    1;34
// Green       0;32     Light Green   1;32
// Cyan        0;36     Light Cyan    1;36
// Red         0;31     Light Red     1;31
// Purple      0;35     Light Purple  1;35
// Brown       0;33     Yellow        1;33
// Light Gray  0;37     White         1;37
  char *cclear = "[0;0m";
  char *cgreen = "[0;32m";
  char *cdgrey = "[1;30m";
  char *cbrown = "[0;33m";
  char *cyellow = "[1;33m";

  char *fep = flon_exid_path(exid);

  char *path = flu_sprintf("var/archive/%s", fep);
  if (flu_fstat(path) == '\0')
  {
    free(path);
    path = flu_sprintf("var/run/%s", fep);
  }

  printf("\n# %s/\n", path);

  puts("\n## trees\n#");
  flu_system("tree -h %s", path);
  flu_system("tree -h var/spool/ -P *%s*", exid);
  flu_system("tree -h var/log/%s", fep);

  puts("\n## dispatcher log\n#");
  printf(cgreen); fflush(stdout);
  flu_system(
    "cat var/log/dispatcher.log | grep --colour=never \"%s\"", exid);
  printf(cclear);

  puts("\n## execution log\n#");
  printf(cgreen); fflush(stdout);
  flu_system(
    "cat %s/exe.log", path);
  printf(cclear);

  puts("\n## invocation log\n#");
  printf(cgreen); fflush(stdout);
  flu_system(
    "find var/log/%s -name \"inv_%s-*.log\" | xargs tail -n +1", fep, exid);
  printf(cclear);

  puts("\n## msgs log (timeline view)\n#");
  char *fpath = flu_sprintf("%s/msgs.log", path);
  FILE *f = fopen(fpath, "r");
  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath); perror("reason:");
  }
  else
  {
    char *line = NULL;
    size_t len = 0;
    fdja_value *v = NULL;

    while (getline(&line, &len, f) != -1)
    {
      //printf("%.32s ", line);
      printf("%.26s ", line);
      char *br = strchr(line, '{');
      v = fdja_parse(br);
      char p = v ? fdja_lk(v, "point") : '?';
      if (p == 'e' || p == 'f') // execute or failed
      {
        v->sowner = 0;
        flu_putf(fdja_todc(v));
        fdja_free(v);
      }
      else
      {
        printf(br);
      }
    }
    free(line);
    fclose(f);
  }

  puts("\n## msgs log (xmas view)\n#");
  f = fopen(fpath, "r");
  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath); perror("reason:");
  }
  else
  {
    char *line = NULL;
    size_t len = 0;
    fdja_value *v = NULL;

    while (getline(&line, &len, f) != -1)
    {
      printf("%s%.26s%s ", cdgrey, line, cclear);
      char *br = strchr(line, '{');
      v = fdja_parse(br);
      char *nid = v ? fdja_ls(v, "nid", NULL) : NULL;
      if (v == NULL)
      {
        printf(br);
      }
      else
      {
        fdja_value *t = fdja_l(v, "tree");

        if (t)
        {
          long long line = fdja_li(t, "2", (long long)0);
          printf("%s%3lli%s ", cbrown, line, cclear);
        }
        else
        {
          printf("    ", line);
        }

        int depth = nid ? flon_nid_depth(nid) : 0;
        printf("%*s", 2 * depth, "");

        char *point = fdja_ls(v, "point", NULL);
        printf("%.2s ", point);
        free(point);

        if (t)
        {
          char *inst = fdja_ls(t, "0", NULL);
          printf("%s ", inst);
          free(inst);
        }

        if (fdja_l(v, "payload"))
        {
          char *payload = fdja_lj(v, "payload");
          printf("pl:%s ", payload);
          free(payload);
        }

        printf("\n");
      }
      //char p = v ? fdja_lk(v, "point") : '?';
      //if (p == 'e' || p == 'f') // execute or failed
      //{
      //  v->sowner = 0;
      //  flu_putf(fdja_todc(v));
      //  fdja_free(v);
      //}
      //else
      //{
      //  printf(br);
      //}
    }
    free(line);
    fclose(f);
  }
  free(fpath);

  puts("\n## run.json\n#");
  fdja_value *v = fdja_parse_f("%s/run.json", path);
  if (v) {
    flu_putf(fdja_todc(v));
    fdja_free(v);
  }
  else
  {
    flu_system("cat %s/run.json", path);
  }

  puts("\n## timers\n#");
  //flu_system("ls -lh var/spool/tdis/%s", fep);
  flu_list *l = flon_list_json("var/spool/tdis/%s", fep);
  if (l) for (flu_node *n = l->first; n; n = n->next)
  {
    char *fn = n->item;
    puts(strrchr(fn, '/') + 1);
    fdja_value *v = fdja_parse_obj_f(fn);
    if (v) flu_putf(fdja_todc(v)); else puts("(null)");
    fdja_free(v);
  }
  flu_list_free_all(l);

  puts("\n## processed\n#");
  printf(cgreen); fflush(stdout);
  flu_system("ls -lh %s/processed", path);
  printf(cclear);

  puts("\n## exe.pid\n#");
  flu_system("cat %s/exe.pid", path);

  // over

  printf("\n\n# %s/ .\n", path);
  puts("");

  free(fep);
  free(path);
}

