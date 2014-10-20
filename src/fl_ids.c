
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

#include "flutil.h"
#include "mnemo.h"
#include "fl_common.h"


static short counter = 0;


char *flon_generate_exid(const char *domain)
{
  char *uid = flon_conf_uid();
  short local = flon_conf_is("unit.time", "local");

  struct timeval tv;
  struct tm *tm;
  char t[20];

  gettimeofday(&tv, NULL);
  tm = local ? localtime(&tv.tv_sec) : gmtime(&tv.tv_sec);
  strftime(t, 20, "%Y%m%d.%H%M", tm);

  char *sus =
    fmne_to_s((tv.tv_sec % 60) * 100000000 + tv.tv_usec * 100 + counter);

  char *r =
    flu_sprintf("%s-%s-%s.%s", domain, uid, t, sus);

  free(sus);
  free(uid);

  counter++; if (counter > 99) counter = 0;

  return r;
}

