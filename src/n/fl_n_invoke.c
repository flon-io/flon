
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


static char exe_invoke(fdja_value *node, fdja_value *exe)
{
  char r = 'k'; // for now, ok

  char *exid = execution_id;
  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *inv = fdja_v("{ exid: \"%s\", nid: \"%s\" }", exid, nid);
  fdja_psetv(inv, "point", "invoke");
  fdja_set(inv, "tree", fdja_lc(exe, "tree"));
  fdja_set(inv, "payload", payload_clone(exe));

  //fdja_value *args = fdja_lc(exe, "tree.1");
  //expand(args, node, exe);
  fdja_value *args = expand(fdja_lc(exe, "tree.1"), node, exe);
  fdja_pset(inv, "payload.args", args);

  if (flon_lock_write(inv, "var/spool/dis/inv_%s-%s.json", exid, nid) != 1)
  {
    fgaj_r("failed writing to var/spool/dis/inv_%s-%s.json", exid, nid);
    set_error_note(node, "failed to write invocation file", NULL);
    r = 'r';
  }

  fdja_free(inv);
  free(nid);

  return r;
}

static char rcv_invoke(fdja_value *node, fdja_value *rcv)
{
  fdja_pset(rcv, "payload.args", NULL);

  return 'v'; // over
}

