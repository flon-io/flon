
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


static char rcv_cand(fdja_value *node, fdja_value *rcv)
{
//  if (is_msg_to_self(rcv)) return seq_rcv(node, rcv);
//
//  fdja_value *vret = fdja_l(rcv, "payload.ret");
//  int ret = ret_to_boolean(vret);
//  char *op = fdja_ls(node, "inst", NULL);
//
//  char r = 'k';
//
//  if (strcmp(op, "or") == 0)
//  {
//    if (ret) r = 'v';
//  }
//  else // "and"
//  {
//    if ( ! ret) r = 'v';
//  }
//
//  free(op);
//
//  fdja_psetv(rcv, "payload.ret", ret ? "true" : "false");
//
//  if (r != 'k') return r;
//  return seq_rcv(node, rcv);

  return 'k';
}

static char exe_cand(fdja_value *node, fdja_value *exe)
{
  return con_exe(node, exe);
}

