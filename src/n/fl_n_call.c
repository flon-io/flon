
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


static char exe_call(fdja_value *node, fdja_value *exe)
{
  char *pnid = fdja_ls(node, "nid");
  fdja_value *atts = attributes(node, exe);

  char *name = fdja_to_string(atts->child);
  fdja_value *val = lookup_var(node, name);

  //fdja_putdc(val);

  char *nid = fdja_ls(val, "nid");
  size_t counter = fdja_li(val, "counter") + 1;
  char *cnid = flu_sprintf("%s-%x", nid, counter);
  fdja_psetv(val, "counter", "%d", counter);

  fdja_psetv(node, "vars", "{}");

  flon_queue_msg("execute", cnid, pnid, payload(exe));

  free(pnid);
  free(nid);
  free(cnid);
  free(name);
  fdja_free(atts);

  return 'k'; // ok, not over
}

