
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


static char rcv_else(fdja_value *node, fdja_value *rcv)
{
  char *nid = fdja_ls(node, "nid", NULL);
  char *from = fdja_ls(rcv, "from", NULL);

  char r = (strcmp(nid, from) == 0) ? 'v' : 'r';

  free(nid);
  free(from);

  return r == 'v' ? r : seq_rcv(node, rcv);
}

static char exe_else(fdja_value *node, fdja_value *exe)
{
  //printf("=== EXE\n");
  //fdja_putdc(node);
  //fdja_putdc(exe);

  if (fdja_size(fdja_l(exe, "tree.3")) < 1) return 'v';
    // no children ? "else" over.

  fdja_value *ret = fdja_l(exe, "payload.ret");
  if (ret == NULL || ret->type != 'f') return 'v';

  return seq_exe(node, exe, 0);
}

