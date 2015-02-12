
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


static char rcv_map(fdja_value *node, fdja_value *rcv)
{
//  fdja_putdc(node);
//  fdja_putdc(rcv);
//
//  char *nid = fdja_ls(node, "nid", NULL);
//  char *from = fdja_ls(rcv, "from", NULL);
//
//  //size_t mindex = 0;
//  //if (from && strcmp(from, nid) != 0)
//
//  // TODO use flon_nid_index(nid)...
//
//  fdja_value *rets = fdja_l(node, "rets");
//  fdja_push(rets, fdja_lc(rcv, "payload.ret"));
  return 'k';
}

static char exe_map(fdja_value *node, fdja_value *exe)
{
//  fdja_set(node, "values", fdja_array_malloc());
//
//  fdja_set(node, "rets", fdja_array_malloc());
//  //fdja_set(node, "mindex", fdja_v("-1"));
//
//  return rcv_map(node, exe);
  return 'k';
}

