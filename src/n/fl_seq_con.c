
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


static char seq_rcv(fdja_value *node, fdja_value *rcv)
{
  char *nid = fdja_ls(node, "nid", NULL);
  char *from = fdja_ls(rcv, "from", NULL);

  fdja_value *children = fdja_l(node, "children");
  fdja_splice(children, 0, 1, NULL); // empty children array

  fdja_value *rets = fdja_l(node, "rets");
  if (rets) fdja_push(rets, fdja_lc(rcv, "payload.ret"));

//printf("<---->\n");
//fdja_putdc(node);
//printf("</--->\n");

  char *next = from ? flon_nid_next(from) : flon_nid_child(nid, 0);
  fdja_value *t = next ? flon_node_tree(next) : NULL;
  char r = 'v'; // over, for now

  if (t)
  {
    flon_queue_msg("execute", next, nid, payload(rcv), NULL, NULL);
    fdja_push(children, fdja_s(next));
    r = 'k'; // ok, not yet over
  }

  free(nid);
  free(next);
  free(from);

  return r;
}

static char seq_exe(fdja_value *node, fdja_value *exe, int track_ret)
{
  fdja_set(node, "children", fdja_array_malloc());
  if (track_ret) fdja_set(node, "rets", fdja_array_malloc());

  if (child_count(node, exe) < 1) return 'v';

  return seq_rcv(node, exe);
}

static char con_exe(fdja_value *node, fdja_value *exe)
{
  return '?'; // TODO
}

static char con_rcv(fdja_value *node, fdja_value *rcv)
{
  return '?'; // TODO
}

