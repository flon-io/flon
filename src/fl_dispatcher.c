
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
#include <libgen.h>

#include "djan.h"
#include "gajeta.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


/*
static void invoke(fdja_value *j, fdja_value *inv)
{
  // TODO double fork an invoker
}

static void discard(fdja_value *j)
{
  char *path = fdja_lookup_string(j, "_path", NULL);

  // TODO case where path is NULL

  fgaj_i("'ing %s", path);

  // TODO move to var/spool/discarded/
}
*/

static int reject(const char *path, fdja_value *j)
{
  char *dp = strdup(path);
  char *db = strdup(path);

  char *d = dirname(dp);
  char *b = basename(db);
  char *np = flu_sprintf("%s/../rejected/%s", d, b);

  int r = rename(path, np);
  if (r != 0) fgaj_r("couldn't rename to %s", np);
  else fgaj_i("to %s", np);

  free(dp);
  free(db);
  free(np);

  if (j) fdja_value_free(j);

  return 0; // indicates error
}

int flon_dispatch(const char *path)
{
  fdja_value *j = fdja_parse_obj_f(path);

  if (j == NULL) return reject(path, j);

  fdja_value *inv = fdja_lookup(j, "invocation");

  //if (inv) return invoke(path, j, inv);

  char *db = strdup(path);
  char *b = basename(db);
  fgaj_i("don't know how to deal with %s", b);
  free(db);
  return reject(path, j);
}

