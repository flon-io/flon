
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


#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "flutil.h"
#include "gajeta.h"
#include "djan.h"
#include "fl_ids.h"
#include "fl_executor.h"


fdja_value *flon_node(const char *nid)
{
  return fdja_l(execution, "nodes.%s", nid);
}

//static fdja_value *node_tree(const char *nid, int clone)
//{
//  fdja_value *t = fdja_l(execution, "nodes.0.tree");
//
//  if (t == NULL) return NULL;
//
//  flu_list *l = flu_split(nid, "_");
//
//  char *cnid = calloc(strlen(nid) + 1, sizeof(char));
//  char *ccnid = cnid + sprintf(cnid, l->first->item);
//
//  if (l->size > 1) for (flu_node *n = l->first->next; n; n = n->next)
//  {
//    ccnid += sprintf(ccnid, "_%s", (char *)n->item);
//
//    fdja_value *nt = fdja_l(execution, "nodes.%s.tree", cnid);
//
//    if (nt)
//      t = nt;
//    else
//      t = fdja_l(t, "3.%lli", strtoll((char *)n->item, NULL, 16));
//  }
//  flu_list_free_all(l);
//
//  free(cnid);
//
//  return clone ? fdja_clone(t) : t;
//    // fdja_clone(NULL) returns NULL
//}

static fdja_value *node_tree(const char *nid)
{
  fdja_value *t = fdja_l(execution, "nodes.%s.tree", nid);
  if (t) return t;

  char *pnid = NULL;

  pnid = fdja_ls(execution, "nodes.%s.parent", nid, NULL);
  if (pnid) t = node_tree(pnid);
  free(pnid);

  if (t) return fdja_at(fdja_at(t, 3), flon_nid_index(nid));

  fdja_value *t0 = NULL;
  pnid = flon_nid_parent(nid, 0);
  if (pnid) t0 = node_tree(pnid);
  free(pnid);

  size_t index = flon_nid_index(nid);

  if (t0) t0 = fdja_at(fdja_at(t0, 3), index);
  if (t0) return t0;

  fdja_value *t1 = NULL;
  pnid = flon_nid_parent(nid, 1);
  if (pnid) t1 = node_tree(pnid);
  free(pnid);

  return t1 ? fdja_at(fdja_at(t1, 3), index) : NULL;
}

fdja_value *flon_node_tree(const char *nid)
{
  return node_tree(nid);
}

fdja_value *flon_node_tree_clone(const char *nid)
{
  return fdja_clone(node_tree(nid));
}

  // TODO: rename to flon_parent_nid()
  // TODO: leverage flon_nid_parent()
  //
char *flon_node_parent_nid(const char *nid)
{
  fdja_value *node = flon_node(nid);
  //if (node) fgaj_d("for: %s", fdja_to_djan(node, 0));
  if (node == NULL) return NULL;

  fdja_value *pnid = fdja_l(node, "parent");
  if (pnid && pnid->type == '0') return NULL;
  if (pnid) return fdja_to_string(pnid);

  fdja_value *v = flon_parse_nid(nid);
  if (v == NULL) return NULL;

  char *ni = fdja_ls(v, "node", NULL);
  fdja_free(v);
  if (ni == NULL) return NULL;

  char *u = strrchr(ni, '_');
  if (u == NULL) { free(ni); return NULL; }

  *u = '\0';

  return ni;
}

