
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
//#include <stdlib.h>
#include <string.h>
//#include <sys/time.h>

//#include "flutil.h"
//#include "mnemo.h"
//#include "aabro.h"
#include "djan.h"
//#include "fl_common.h"
//#include "fl_ids.h"


char *flon_nid_path(fdja_value *nid)
{
  if (nid == NULL) return NULL;

  char *r = NULL;

  char *exid = fdja_ls(nid, "exid", NULL);
  char *domain = fdja_ls(nid, "domain", NULL);

  if (exid == NULL || domain == NULL) goto _over;

  char *dot = strrchr(exid, '.') + 1;

  r = flu_sprintf("%s/%.2s/%s", domain, dot, exid);

_over:

  free(exid);
  free(domain);

  return r;
}

char *flon_exid_path(const char *s)
{
  fdja_value *n = flon_parse_nid(s);
  if (n == NULL) return NULL;

  char *r = flon_nid_path(n);
  fdja_free(n);

  return r;
}

