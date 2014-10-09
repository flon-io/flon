
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

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
//#include <unistd.h>

#include "flutil.h"
#include "djan.h"
//#include "gajeta.h"
#include "fl_common.h"


static fdja_value *flon_configuration = NULL;

void flon_configure(char *root)
{
  fdja_value *v = fdja_parse_obj_f("%s/etc/flon.json", root);

  if (v) fdja_set(v, "_root", fdja_s(root));

  flon_configure_j(v);
}

void flon_configure_j(fdja_value *obj)
{
  if (flon_configuration) fdja_value_free(flon_configuration);

  flon_configuration = obj;
}

fdja_value *flon_conf(const char *key)
{
  return fdja_lookup(flon_configuration, key);
}

int flon_conf_boolean(const char *key, int def)
{
  return fdja_lookup_bool(flon_configuration, key, def);
}

long long flon_conf_int(const char *key, long long def)
{
  return fdja_lookup_int(flon_configuration, key, def);
}

char *flon_conf_string(const char *key, char *def)
{
  return fdja_lookup_string(flon_configuration, key, def);
}

char *flon_conf_path(const char *key, char *def)
{
  char *v = flon_conf_string(key, def);

  if (v == NULL) return NULL;

  return flu_canopath(v);
}

char *flon_generate_id()
{
  struct timeval tv;
  struct tm *tm;
  gettimeofday(&tv, NULL);
  tm = gmtime(&tv.tv_sec);
  char *i = calloc(29, sizeof(char));
  strftime(i, 29, "%Y%m%d.%H%M%S.000000", tm);
  snprintf(i + 16, 7, "%06ld", tv.tv_usec);

  char *u = flon_conf_string("unit.id", "u0");
  char *g = flon_conf_string("unit.group_id", "g0");
  char *r = flu_sprintf("%s_%s_%s", u, g, i);

  if (strcmp(u, "u0")) free(u);
  if (strcmp(g, "g0")) free(g);
  free(i);

  return r;
}
