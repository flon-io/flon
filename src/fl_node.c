
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

#include <stdlib.h>

#include "flutil.h"
#include "djan.h"
#include "fl_node.h"


fdja_value *flon_node(fdja_value *exe, const char *nid)
{
  return fdja_l(exe, "nodes.%s", nid);
}

static fdja_value *node_tree(fdja_value *exe, const char *nid, int clone)
{
  fdja_value *t = fdja_l(exe, "nodes.0.tree");

  flu_list *l = flu_split(nid, "_");
  if (l->size > 1) for (flu_node *n = l->first->next; n != NULL; n = n->next)
  {
    t = fdja_l(t, "2.%lli", strtol((char *)n->item, NULL, 16));
  }
  flu_list_free_all(l);

  return clone ? fdja_clone(t) : t;
}

fdja_value *flon_node_tree(fdja_value *exe, const char *nid)
{
  return node_tree(exe, nid, 0);
}

fdja_value *flon_node_tree_c(fdja_value *exe, const char *nid)
{
  return node_tree(exe, nid, 1);
}

char *flon_node_parent_nid(fdja_value *exe, const char *nid)
{
  fdja_value *node = flon_node(exe, nid);

  if (node == NULL) return NULL;

  char *pnid = fdja_ls(node, "parent", NULL);

  if (pnid) return pnid;

  // TODO
  return NULL;
}

