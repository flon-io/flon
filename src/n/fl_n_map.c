
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
  if (is_msg_to_self(rcv)) return 'v'; // over

  ssize_t index = fdja_li(node, "map.index", (ssize_t)-1);

  if (index > -1) fdja_pset(node, "map.rets.]", fdja_lc(payload(rcv), "ret"));

  fdja_value *values = fdja_l(node, "map.values");

  fdja_pset(node, "map.index", fdja_v("%zu", ++index));

  if (index >= fdja_size(values)) // iteration over
  {
    fdja_pset(rcv, "payload.ret", fdja_lc(node, "map.rets"));
    return 'v';
  }

  fdja_value *callname = fdja_l(node, "map.callname");

  fdja_value *callee = NULL;

  if (callname)
  {
    callee =
      fdja_clone(callname);
  }
  else
  {
    callee =
      fdja_o(
        "nid", fdja_lc(node, "nid"),
        "args", fdja_v("[ ret, v.key, v.index ]"),
        NULL);
  }

  fdja_value *cargs = fdja_object_malloc();

  //fdja_set(
  //  cargs, "_0",
  //  fdja_o("nid", nid, "args", fdja_v("[ ret, v.key, v.index ]"), NULL));
  fdja_set(cargs, "_0", callee);
  fdja_set(cargs, "_1", fdja_atc(values, index));
  fdja_set(cargs, "_2", fdja_v("%zu", index));
  fdja_set(cargs, "_3", fdja_v("%zu", index));

  return do_call(node, rcv, cargs);
}

static char exe_map(fdja_value *node, fdja_value *exe)
{
  fdja_value *t = tree(node, exe);
  fdja_value *atts = fdja_at(t, 1);
  fdja_value *a0 = atts->child;
  fdja_value *a1 = a0 ? a0->sibling : NULL;
  fdja_value *cn = fdja_at(t, 3); // children

  // marshall input

  fdja_value *values = NULL;
  fdja_value *callname = NULL;
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
    values = fdja_l(exe, "payload.ret");
    callname = a0;
  }
  else if (cn->child) // map \n 1 + $(ret)
  {
    values = fdja_l(exe, "payload.ret");
    block = cn;
  }
  //else {} // error

  // raise on bad input

  if (values == NULL)
  {
    push_error(node, "no values to map from", t);
    return 'r';
  }
  if (callname == NULL && block == NULL)
  {
    push_error(node, "no function/block to map with", t);
    return 'r';
  }

  // ok, start mapping

  fdja_set(node, "map", fdja_object_malloc());
  fdja_pset(node, "map.values", fdja_clone(values));
  fdja_pset(node, "map.callname", fdja_clone(callname));
  fdja_pset(node, "map.rets", fdja_array_malloc());

  return rcv_map(node, exe);
}

