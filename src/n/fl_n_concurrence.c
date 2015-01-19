
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


// TODO: move the core of it to src/n/fl_seq_con.c

static char rcv_concurrence(fdja_value *node, fdja_value *rcv)
{
  // TODO: focus immediately on "merge participants (invokers)"

  char *from = fdja_ls(rcv, "from", NULL);
  fdja_value *children = fdja_l(node, "children");

  //flu_putf(fdja_todc(children));
  //flu_putf(fdja_todc(rcv));

  int found = fdja_unpush(children, from);

  if (found) // merge
  {
    // TODO merge
  }

  free(from);

  //if (found == 0) // not found...
  if (fdja_size(children) == 0) return 'v'; // over
  return 'k';
}

static char exe_concurrence(fdja_value *node, fdja_value *exe)
{
  //flu_putf(fdja_todc(node));
  //flu_putf(fdja_todc(exe));

  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *children = fdja_set(node, "children", fdja_v("[]"));

  char *cnid = NULL;

  for (size_t i = 0; ; ++i)
  {
    free(cnid); cnid = flu_sprintf("%s_%zu", nid, i);
    fdja_value *t = flon_node_tree(cnid); if (t == NULL) break;
    flon_queue_msg("execute", cnid, nid, payload(exe), NULL, NULL);
    fdja_push(children, fdja_s(cnid));
  }

  free(cnid);
  free(nid);

  if (fdja_size(children) == 0) return 'v'; // already over
  return 'k'; // ok
}

