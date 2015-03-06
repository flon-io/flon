
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
#include "djan.h"
#include "fl_ids.h"
#include "fl_flon.h"


char *flon_launch(const char *path)
{
  char *domain = NULL;
  char *stree = NULL;
  char *spayload = NULL;
  char *svars = NULL;

  char current = 0;
  flu_sbuffer *sb = flu_sbuffer_malloc();

  FILE *f = fopen(path, "r");
  if (f == NULL) return NULL;

  char *line = NULL;
  size_t len = 0;

  while (1)
  {
    int r = getline(&line, &len, f);

    char prev = current;

    if (r == -1) current = 'V';
    else if (strcmp(line, "# domain\n") == 0) current = 'd';
    else if (strcmp(line, "# dom\n") == 0) current = 'd';
    else if (strcmp(line, "# tree\n") == 0) current = 't';
    else if (strcmp(line, "# payload\n") == 0) current = 'p';
    else if (strcmp(line, "# fields\n") == 0) current = 'p';
    else if (strcmp(line, "# vars\n") == 0) current = 'v';
    else if (strcmp(line, "# variables\n") == 0) current = 'v';

    if (current == prev)
    {
      flu_sbputs(sb, line);
    }
    else
    {
      if (prev == 'd') domain = flu_sbuffer_to_string(sb);
      else if (prev == 't') stree = flu_sbuffer_to_string(sb);
      else if (prev == 'p') spayload = flu_sbuffer_to_string(sb);
      else /*if (prev == 'v')*/ svars = flu_sbuffer_to_string(sb);

      sb = flu_sbuffer_malloc();
    }

    if (r == -1) break;
  }

  flu_sbuffer_free(sb);

  free(line);
  fclose(f);

  char *s = flu_strtrim(domain); free(domain); domain = s;

  fdja_value *tree = fdja_parse_radial(stree, path);
  fdja_value *payload = fdja_parse_obj(spayload);
  fdja_value *vars = fdja_parse_obj(svars);

  if (payload == NULL) payload = fdja_object_malloc();

  char *exid = flon_generate_exid(domain);

  fdja_value *msg = fdja_object_malloc();
  fdja_set(msg, "point", fdja_s("execute"));
  fdja_set(msg, "exid", fdja_s(exid));
  fdja_set(msg, "tree", tree);
  fdja_set(msg, "payload", payload);
  if (vars) fdja_set(msg, "vars", vars);

  fdja_to_json_f(msg, "var/spool/dis/exe_%s.json", exid);

  free(domain);

  fdja_free(msg);

  return exid;
}

