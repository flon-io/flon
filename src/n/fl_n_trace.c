
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


static fdja_value *shrink_atts(fdja_value *atts)
{
  for (fdja_value *v = atts->child; v; v = v->sibling)
  {
    if ( ! is_index(v->key)) return fdja_clone(atts);
  }

  fdja_value *r = fdja_array_malloc();

  for (fdja_value *v = atts->child; v; v = v->sibling)
  {
    fdja_push(r, fdja_clone(v));
  }

  return r;
}

static char exe_trace(fdja_value *node, fdja_value *exe)
{
  fdja_value *pl = payload(exe);

  fdja_value *trace = fdja_l(pl, "trace");
  if (trace == NULL) trace = fdja_set(pl, "trace", fdja_array_malloc());

  fdja_value *atts = attributes(node, exe);

  if (atts->child == NULL)
  {
    fdja_value *ret = fdja_l(exe, "payload.ret");
    fdja_push(trace, ret ? fdja_clone(ret) : fdja_v("null"));
  }
  else if (atts->child && atts->child->sibling == NULL)
  {
    fdja_push(trace, fdja_lc(atts, "_0"));
  }
  else
  {
    fdja_push(trace, shrink_atts(atts));
  }

  fdja_free(atts);

  return 'v'; // over
}

