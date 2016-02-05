
//
// Copyright (c) 2013-2016, John Mettraux, jmettraux+flon@gmail.com
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

#include "gajeta.h"
#include "fl_common.h"
#include "fl_executor.h"


static void unlink_exe_pid(const char *exid)
{
  if (flu_unlink("var/run/%s.pid", exid) == 0)
    fgaj_d("unlinked var/run/%s.pid", exid);
  else
    fgaj_r("failed to unlink var/run/%s.pid", exid);
}

int main(int argc, char *argv[])
{
  int r = 0;

  flon_configure(".");
  flon_setup_logging("executor");

  if (argc < 2) { fgaj_e("missing exid as arg"); r = 1; goto _over; }

  flon_execute(argv[1]);

_over:

  unlink_exe_pid(argv[1]);

  return r;
}

