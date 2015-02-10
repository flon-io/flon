
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


static char rcv_set(fdja_value *node, fdja_value *rcv)
{
  fdja_value *att = fdja_at(fdja_at(tree(node, rcv), 1), 0);
  expand(att, node, rcv);

  char *satt = fdja_to_string(att);
  char *key = satt;

  char k = extract_prefix(key);
  if (k != 0) key = strchr(key, '.') + 1;

  fdja_value *val = fdja_lc(rcv, "payload.ret");

  if (k == 'f' || k == 0)
    fdja_pset(fdja_l(rcv, "payload"), key, val);
  else if (k == 'v')
    set_var(node, *satt, key, val);
  else if (k == 'w')
    set_war(node, key, val);

  free(satt);

  return 'v';
}

static char exe_set(fdja_value *node, fdja_value *exe)
{
  if (child_count(node, exe) < 1) return 'v';

  char *nid = fdja_ls(node, "nid", NULL);
  char *next = flon_nid_child(nid, 0);

  queue_child_execute(next, node, exe, NULL);

  free(next);
  free(nid);

  return 'k';
}

