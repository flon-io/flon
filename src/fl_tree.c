
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

#define _POSIX_C_SOURCE 200809L

#include "fl_tree.h"


int flon_is_tree(fdja_value *v)
{
  if (v == NULL) return 0;
  if (v->type != 'a') return 0;

  if (v->child == NULL) return 0; // head
  //if (strchr("sqya", v->child->type) == NULL) return 0;
  if (v->child->type == 'a' && ! flon_is_tree(v->child)) return 0;
    //
    // the head can be anything, but if it's an array, it has to be a tree
    //
    // TODO think and [maybe] fix in flar first then adapt flon here

  if (v->child->sibling == NULL) return 0; // attributes
  if (v->child->sibling->type != 'o') return 0;

  if (v->child->sibling->sibling == NULL) return 0; // line
  if (v->child->sibling->sibling->type != 'n') return 0;

  if (v->child->sibling->sibling->sibling == NULL) return 0; // children
  if (v->child->sibling->sibling->sibling->type != 'a') return 0;

  for (
    fdja_value *c = v->child->sibling->sibling->sibling->child;
    c;
    c = c->sibling
  ) if ( ! flon_is_tree(c)) return 0;

  return 1;
}

