
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


//
// *** WAIT

static char exe_wait(fdja_value *node, fdja_value *exe)
{
  fdja_value *atts = attributes(node, exe);

  char *f = fdja_lsd(atts, "_0", "");
  long long s = flu_parse_t(f);
  char *a = flu_sstamp(flu_gets('s') + s, 1, 's');

  log_d(node, exe, "sleep for %s (%llis) --> until %s", f, s, a);

  char *nid = fdja_ls(node, "nid", NULL);
  char *exid = fdja_ls(exe, "exid", NULL);

  fdja_value *msg = fdja_v("{}");
  fdja_set(msg, "point", fdja_s("receive"));
  fdja_set(msg, "nid", fdja_s(nid));
  fdja_set(msg, "exid", fdja_s(exid));
  fdja_set(msg, "payload", fdja_lc(exe, "payload"));

  fdja_value *t0 = fdja_clone(tree(node, exe));
  fdja_value *t1 = fdja_v("[]");
  fdja_push(t1, fdja_lc(node, "inst"));
  fdja_push(t1, atts);
  fdja_push(t1, fdja_v("[]"));
  fdja_psetv(t1, "2.]", "%zu", fdja_size(fdja_l(t0, "2")));
  fdja_psetv(t0, "2", "[ %zu ]", fdja_size(fdja_l(t0, "2")));

  flon_schedule_msg("at", a, nid, t0, t1, msg);

  if (fdja_l(node, "timers") == NULL) fdja_set(node, "timers", fdja_v("[]"));
  fdja_psetv(node, "timers.]", "{ at: \"%s\" }", a);

  free(f);
  free(a);
  free(nid);
  free(exid);

  return 'k'; // ok
}

