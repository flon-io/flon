
//
// Copyright (c) 2013-2014, John Mettraux, jmettraux+flon@gmail.com
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

#include <dollar.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "flutil.h"
#include "gajeta.h"
#include "djan.h"
#include "fl_common.h"
#include "fl_invoker.h"


int flon_invoke(const char *path)
{
  fdja_value *inv = fdja_parse_obj_f(path);

  if (inv == NULL)
  {
    fgaj_r("couldn't read inv msg at %s", path); return 1;
  }

  //printf(">>>\n%s\n<<<\n", fdja_to_json(inv));

  fdja_value *invocation = fdja_lookup(inv, "invoke");

  if (invocation == NULL)
  {
    fgaj_e("no 'invoke' key in the message"); return 1;
  }

  fdja_value *payload = fdja_lookup(inv, "payload");
  if (payload == NULL) payload = fdja_v("{}");

  char *exid = fdja_ls(inv, "exid", NULL);
  char *nid = fdja_ls(inv, "nid", NULL);

  char *invoked = fdja_ls(invocation, "0", NULL);
  if (strcmp(invoked, "invoke") == 0)
  {
    invoked = fdja_ls(invocation, "1._0", NULL);
  }

  char *invoker_path = flu_sprintf("usr/local/inv/%s", invoked);
  char *ret = flu_sprintf("var/spool/dis/ret_%s-%s.json", exid, nid);

  char cwd[1024 + 1]; getcwd(cwd, 1024);
  fgaj_i("cwd: %s", cwd);

  fgaj_i("exid: %s, nid: %s", exid, nid);
  //fgaj_i("invocation: %s", fdja_to_json(invocation));
  fgaj_i("invoker at %s", invoker_path);

  fdja_value *inv_conf = fdja_parse_obj_f("%s/flon.json", invoker_path);

  if (inv_conf == NULL)
  {
    fgaj_r("didn't find invoker conf at %s/flon.json", invoker_path);
    return 1;
  }

  char *cmd = fdja_ls(inv_conf, "invoke", NULL);

  char *out = fdja_ls(inv_conf, "out", NULL);
  if (out && strcmp(out, "discard") == 0)
  {
    free(ret); ret = strdup("/dev/null");
  }
  free(out);

  if (cmd == NULL)
  {
    fgaj_e("no 'invoke' key in invoker conf at %s/flon.json", invoker_path);
    return 1;
  }

  if (strstr(cmd, "$("))
  {
    flu_dict *d = flu_d("exid", exid, "nid", nid, NULL);
    char *cmd1 = fdol_expand(cmd, fdol_dlup, d);
    flu_list_free(d);
    char *o = cmd;
    cmd = cmd1;
    free(o);
  }

  fgaj_i("invoking >%s<", cmd);

  int pds[2];

  int r = pipe(pds);

  if (r != 0)
  {
    fgaj_r("failed to setup pipe between invoker and invoked");
    return 1;
  }

  pid_t i = fork();

  if (i < 0) // failure
  {
    fgaj_r("failed to fork invoked");
  }
  else if (i == 0) // child
  {
    fgaj_i("child, pid %i", getpid());

    close(pds[1]);
    dup2(pds[0], STDIN_FILENO);
    close(pds[0]);

    if (setsid() == -1)
    {
      fgaj_r("setsid() failed");
      return 127;
    }

    if (freopen(ret, "w", stdout) == NULL)
    {
      fgaj_r("failed to reopen child stdout to %s", ret);
      return 127;
    }

    if (chdir(invoker_path) != 0)
    {
      fgaj_r("failed to chdir to %s", invoker_path);
      return 127;
    }

    fflush(stderr);

    r = execl("/bin/sh", "", "-c", cmd, NULL);

    fgaj_r("execl failed (%i)", r);

    return 127;
  }
  else // parent
  {
    close(pds[0]);

    FILE *f = fdopen(pds[1], "w");
    fdja_to_j(f, payload, 0);
    fclose(f);

    // over, no wait

    fgaj_i("invoked >%s< pid %i", cmd, i);
  }

  // resource cleanup
  //
  // not really necessary, but it helps when debugging / checking...
  // 0 lost, like all the others (hopefully).

  fdja_free(inv);
  fdja_free(inv_conf);
  free(exid);
  free(nid);
  free(invoker_path);
  free(cmd);
  free(invoked);
  free(ret);

  // exit

  return 0;
}

