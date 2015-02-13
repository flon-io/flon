
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


// sequence
//
//   map [ 1, 2, 3 ]
//     1 + $(ret)
//   # --> [ 2, 3, 4 ]
//
//   [ 1, 2, 3 ]
//   map
//     2 + $(ret)
//
//   define add-three ret
//     3 + $(ret)
//
//   map [ 1, 2, 3 ] add-three
//
//   [ 1, 2, 3 ]
//   map add-three


static char rcv_map(fdja_value *node, fdja_value *rcv)
{
  printf("~~~~~\n");
  fdja_putdc(node);
  fdja_putdc(rcv);

  char r = 'k'; // ok for now

  ssize_t index = fdja_li(node, "map.index", (ssize_t)-1);

  if (index > -1) fdja_pset(node, "map.rets.]", fdja_lc(payload(rcv), "ret"));

  fdja_value *values = fdja_l(node, "map.values");

  ++index;

  if (index >= fdja_size(values))
  {
    fdja_pset(rcv, "payload.ret", fdja_lc(node, "map.rets"));
    r = 'v'; goto _over;
  }

  // TODO: continue me

_over:

  return r;
}

static char exe_map(fdja_value *node, fdja_value *exe)
{
  char r = 'k'; // ok for now

  fdja_value *t = tree(node, exe);
  fdja_value *atts = fdja_at(t, 1);
  fdja_value *a0 = atts->child;
  fdja_value *a1 = a0 ? a0->sibling : NULL;
  fdja_value *cn = fdja_at(t, 3); // children

  // marshall input

  fdja_value *values = NULL;
  fdja_value *callname = NULL; char *cname = NULL;
  //fdja_value *callable = NULL;
  fdja_value *block = NULL;

  if (a0 && a1) // map [ 1, 2, 3 ] add-three
  {
    values = a0;
    callname = a1;
  }
  else if (a0 && cn->child) // map [ 1, 2, 3 ] \n 1 + $(ret)
  {
    values = a0;
    block = cn;
  }
  else if (a0) // map add-three
  {
    values = payload(exe);
    callname = a0;
  }
  else if (cn->child) // map \n 1 + $(ret)
  {
    values = payload(exe);
    block = cn;
  }
  //else {} // error

  // raise on bad input

  if (values == NULL)
  {
    push_error(node, "no values to map from", t);
    r = 'r'; goto _over;
  }
  if (callname == NULL && block == NULL)
  {
    push_error(node, "no function/block to map with", t);
    r = 'r'; goto _over;
  }

  //if (callname) cname = fdja_to_string(callname);
  //if (cname) callable = lookup_var(node, 'l', cname); // 'l' for "local"
  //
  //if (callname && callable == NULL)
  //{
  //  push_error(node, "cannot map with non-callable", callname);
  //  r = 'r'; goto _over;
  //}

  // ok, start mapping

  fdja_set(node, "map", fdja_object_malloc());
  fdja_pset(node, "map.values", fdja_clone(values));
  fdja_pset(node, "map.callname", fdja_clone(callname));
  //fdja_pset(node, "map.index", fdja_v("-1"));
  fdja_pset(node, "map.rets", fdja_array_malloc());

  r = rcv_map(node, exe);

_over:

  free(cname);

  return r;
}

