
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


static char rcv_ife(fdja_value *node, fdja_value *rcv)
{
  char r = 'k';

  char *nid = fdja_ls(node, "nid", NULL);
  char *from = fdja_ls(rcv, "from", NULL);

  char *conditional = flon_nid_child(nid, 0);

  if (strcmp(from, conditional) != 0) { r = 'v'; goto _over; }

  int ret = ret_to_boolean(fdja_l(rcv, "payload.ret"));

  if (fdja_strcmp(fdja_l(node, "inst"), "ife") != 0) ret = ! ret;
  int branch = ret ? 1 : 2;

  fdja_psetv(rcv, "payload.ret", ret ? "true" : "false");

  char *next = flon_nid_next(from, branch);

  if (flon_node_tree(next))
    queue_child_execute(next, node, rcv, NULL);
  else
    r = 'v';

  free(next);

_over:

  free(nid);
  free(from);
  free(conditional);

  return r;
}

static char exe_ife(fdja_value *node, fdja_value *exe)
{
  fdja_putdc(node);
  fdja_putdc(exe);

  if (fdja_size(fdja_l(exe, "tree.3")) < 1) return 'v';
    // no children ? "ife" over.

  char *nid = fdja_ls(node, "nid", NULL);
  char *next = flon_nid_child(nid, 0);

  queue_child_execute(next, node, exe, NULL);

  free(nid);
  free(next);

  return 'k';
}

