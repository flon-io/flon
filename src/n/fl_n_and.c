
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


static int to_boolean(fdja_value *v)
{
  if (v == NULL) return 0;
  if (v->type == 'f') return 0;
  if (v->type == '0') return 0;
  //if (v->type == 't') return 1;
  return 1;
}

static char rcv_and(fdja_value *node, fdja_value *rcv)
{
  char r = seq_rcv(node, rcv);

  if (r != 'v' && r != 'k') return r;

  fdja_value *vret = fdja_l(rcv, "payload.ret");
  char *op = fdja_ls(node, "inst", NULL);

  int ret = to_boolean(vret);

  if (strcmp(op, "or") == 0)
  {
    if (ret) r = 'v';
  }
  else // "and"
  {
    if ( ! ret) r = 'v';
  }

  free(op);

  fdja_psetv(rcv, "payload.ret", ret ? "true" : "false");

  return r;
}

static char exe_and(fdja_value *node, fdja_value *exe)
{
  return seq_exe(node, exe, 0); // no need to collect the "ret"s...
}
