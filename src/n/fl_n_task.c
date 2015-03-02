
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


static void log_task(fdja_value *msg)
{
  char *exid = fdja_ls(msg, "exid", NULL);
  char *fep = flon_exid_path(exid);

  char *lpath = flu_sprintf("var/run/%s/tsk.log", fep);

  FILE *log = fopen(lpath, "a");

  if (log == NULL)
  {
    fgaj_r("failed to open %s, logging to var/log/tsk.log", lpath);

    log = fopen("var/log/tsk.log", "a");

    if (log == NULL)
    {
      fgaj_r("failed to open var/log/tsk.log");
      goto _over;
    }
  }

  char *now = fgaj_now();
  fputs(now, log);
  fputc(' ', log);
  //fputs(msg->source, log); // ! bypasses any changes to msg
  fdja_to_d(log, msg, FDJA_F_COMPACT, 0);
  fputc('\n', log);

  if (fclose(log) != 0) { fgaj_r("failed to close %s", lpath); }

  free(now);

_over:

  free(exid);
  free(fep);
  free(lpath);
}

static char exe_task(fdja_value *node, fdja_value *exe)
{
  char r = 'k'; // for now, ok

  char *exid = execution_id;
  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *tsk = fdja_v("{ exid: \"%s\", nid: \"%s\" }", exid, nid);

  fdja_psetv(tsk, "point", "task");

  fdja_set(tsk, "task", fdja_object_malloc());
  fdja_psetv(tsk, "task.state", "created");
  fdja_psetv(tsk, "task.event", "creation");
  fdja_psetv(tsk, "task.from", "executor");
  fdja_pset(tsk, "task.for", fdja_lc(exe, "tree.1._0"));

  fdja_set(tsk, "tree", fdja_lc(exe, "tree"));

  fdja_set(tsk, "payload", payload_clone(exe));
  fdja_pset(tsk, "payload.args", fdja_lc(exe, "tree.1"));

  if (flon_lock_write(tsk, "var/spool/dis/tsk_%s-%s.json", exid, nid) != 1)
  {
    fgaj_r("failed writing to var/spool/dis/tsk_%s-%s.json", exid, nid);
    push_error(node, "failed to write tsk_ file", NULL);
    r = 'r';
  }
  else // wrote successfully to var/spool/dis/
  {
    fdja_set(node, "taskee", fdja_lc(exe, "tree.1._0"));
    log_task(tsk);
  }

  fdja_free(tsk);
  free(nid);

  return r;
}

static char rcv_task(fdja_value *node, fdja_value *rcv)
{
  //printf("=================== rcv_task()\n");
  //fdja_putdc(node);
  //fdja_putdc(rcv);

  log_task(rcv);

  fdja_value *state = fdja_l(rcv, "task.state");
  //fdja_value *event = fdja_l(rcv, "task.event");

  if (fdja_strcmp(state, "failed") == 0)
  {
    push_error_value(node, fdja_o("msg", fdja_lc(rcv, "task.msg"), NULL));

    return 'r';
  }

  if (fdja_strcmp(state, "completed") == 0)
  {
    fdja_pset(rcv, "payload.args", NULL);

    return 'v'; // over
  }

  char r = 'k';

  // if the msg is a new offer, echo it back, else shut up

  fdja_value *taskee = fdja_l(node, "taskee");
  fdja_value *tfor = fdja_l(rcv, "task.for");

  if (fdja_strcmp(state, "offered") == 0 && fdja_cmp(taskee, tfor) != 0)
  {
    char *exid = fdja_ls(rcv, "exid", NULL);
    char *nid = fdja_ls(rcv, "nid", NULL);

    fdja_psetv(rcv, "task.from", "executor");

    if (flon_lock_write(rcv, "var/spool/dis/tsk_%s-%s.json", exid, nid) != 1)
    {
      fgaj_r("failed writing to var/spool/dis/tsk_%s-%s.json", exid, nid);
      push_error(node, "failed to write tsk_ file", NULL);
      r = 'r';
    }

    free(exid);
    free(nid);
  }

  return r;
}

