
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


static char exe_define(fdja_value *node, fdja_value *exe)
{
  char *nid = fdja_ls(node, "nid");
  char *dash = strchr(nid, '-'); if (dash) *dash = 0;
    // truncate nid -counter part...

  fdja_value *atts = attributes(node, exe);

  char *name = fdja_to_string(atts->child);

  fdja_value *val = fdja_object_malloc();
  fdja_value *args = fdja_set(val, "args", fdja_array_malloc());
  fdja_psetv(val, "nid", nid);

  for (fdja_value *v = atts->child->sibling; v; v = v->sibling)
  {
    fdja_push(args, fdja_clone(v));
  }

  fdja_psetv(val, "tree.0", "sequence");
  fdja_psetv(val, "tree.1", "{}");

  set_var(node, 'l', name, val); // 'l' for "local"

  free(nid);
  free(name);
  fdja_free(atts);

  return 'v'; // over
}

