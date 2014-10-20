
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

//#include <time.h>

#include "djan.h"


fdja_value *flon_node(fdja_value *execution, char *nid)
{
  return fdja_l(execution, "nodes.%s", nid);
}

static fdja_value *node_tree(fdja_value *exe, char *nid, int clone)
{
  //fdja_value *n = flon_node(exe, nid);
  fdja_value *n = flon_node(exe, "0");

  //puts(fdja_to_json(n));

  return clone ? fdja_lc(n, "tree") : fdja_l(n, "tree");
}

fdja_value *flon_node_tree(fdja_value *execution, char *nid)
{
  return node_tree(execution, nid, 0);
}

fdja_value *flon_node_tree_c(fdja_value *execution, char *nid)
{
  return node_tree(execution, nid, 1);
}

