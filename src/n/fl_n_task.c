
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


static char exe_task(fdja_value *node, fdja_value *exe)
{
  char r = 'k'; // for now, ok

  char *exid = execution_id;
  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *tsk = fdja_v("{ exid: \"%s\", nid: \"%s\" }", exid, nid);
  fdja_psetv(tsk, "point", "task");
  fdja_psetv(tsk, "state", "created");
  fdja_set(tsk, "taskee", fdja_lc(exe, "tree.1._0"));
  fdja_set(tsk, "tree", fdja_lc(exe, "tree"));
  fdja_set(tsk, "payload", payload_clone(exe));
  fdja_pset(tsk, "payload.args", fdja_lc(exe, "tree.1"));

  if (flon_lock_write(tsk, "var/spool/dis/tsk_%s-%s.json", exid, nid) != 1)
  {
    fgaj_r("failed writing to var/spool/dis/tsk_%s-%s.json", exid, nid);
    push_error(node, "failed to write tsk_ file", NULL);
    r = 'r';
  }

  fdja_free(tsk);
  free(nid);

  return r;
}

static char rcv_task(fdja_value *node, fdja_value *rcv)
{
  fdja_pset(rcv, "payload.args", NULL);

  //printf("=================== rcv_task()\n");
  //fdja_putdc(node);
  //fdja_putdc(rcv);

  fdja_value *state = fdja_l(rcv, "state");
  //fdja_value *on = fdja_l(rcv, "on");

  // TODO there is "completed", "failed" and some others !!!!

  if (state == NULL || fdja_strcmp(state, "failed") != 0) return 'v'; // over

  push_error(node, fdja_ls(rcv, "msg", NULL), NULL);

  return 'r';
}

