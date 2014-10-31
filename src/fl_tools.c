
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

//#include <time.h>
#include <stdlib.h>
//#include <string.h>
//#include <sys/time.h>

#include "flutil.h"
//#include "mnemo.h"
//#include "aabro.h"
//#include "djan.h"
//#include "fl_common.h"
#include "fl_ids.h"
#include "fl_tools.h"


void flon_prettyprint(const char *exid)
{
  char *fep = flon_exid_path(exid);

  char *path = flu_sprintf("var/run/%s", fep);
  if (flu_fstat(path) == '\0')
  {
    free(path);
    path = flu_sprintf("var/archive/%s", fep);
  }

  free(fep);

  printf("\n# %s/\n", path);

  puts("\n## execution log\n#");
  printf("[0;32m"); fflush(stdout);
  flu_system("cat %s/exe.log", path);
  printf("[0;0m");

  puts("\n## msgs log\n#");
  char *fpath = flu_sprintf("%s/msgs.log", path);
  FILE *f = fopen(fpath, "r");
  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath);
    perror("reason:");
  }
  else
  {
    char *line = NULL;
    size_t len = 0;
    fdja_value *v = NULL;

    while (getline(&line, &len, f) != -1)
    {
      v = fdja_parse(line);
      if (v)
      {
        v->sowner = 0;
        char *s = fdja_todc(v); puts(s); free(s);
        fdja_free(v); }
      else { puts(line); }
    }
    free(line);
    fclose(f);
  }
  free(fpath);

  puts("\n## run.json\n#");
  fdja_value *v = fdja_parse_f("%s/run.json", path);
  if (v) {
    char *s = fdja_todc(v); puts(s); free(s);
    fdja_free(v);
  }
  else
  {
    flu_system("cat %s/run.json", path);
  }

  puts("\n## processed\n#");
  printf("[0;32m"); fflush(stdout);
  flu_system("ls -lh %s/processed", path);
  printf("[0;0m");

  puts("\n## exe.pid\n#");
  flu_system("cat %s/exe.pid", path);

  // over

  printf("\n\n# %s/ .\n", path);
  puts("");

  free(path);
}
