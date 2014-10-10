
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


// TODO run invoker through Strace (which outputs by default to stderr)
// TODO run invoker through Valgrind

int flon_invoke(const char *path)
{
  fdja_value *inv = fdja_parse_obj_f(path);

  //printf(">>>\n%s\n<<<\n", fdja_to_json(inv));

  fdja_value *invocation = fdja_lookup(inv, "invocation");

  if (invocation == NULL) {
    fgaj_e("no 'invocation' key in the message");
    return 1;
  }

  fdja_value *payload = fdja_lookup(inv, "payload");
  if (payload == NULL) payload = fdja_v("{}");

  char *id = fdja_lookup_string(inv, "id", NULL);
  char *invoked = fdja_to_string(invocation->child);
  char *invoker_path = flu_sprintf("usr/local/inv/%s", invoked);

  fgaj_i("cwd: %s", getcwd(NULL, 0));
  fgaj_i("id: %s", id);
  fgaj_i("invocation: %s", fdja_to_json(invocation));
  fgaj_i("invoker at %s", invoker_path);

  char *inv_conf_path = flu_sprintf("%s/flon.json", invoker_path);
  fdja_value *inv_conf = fdja_parse_obj_f(inv_conf_path);

  if (inv_conf == NULL)
  {
    fgaj_r("didn't find invoker conf at %s", inv_conf_path);
    return 1;
  }

  char *cmd = fdja_lookup_string(inv_conf, "invoke", NULL);

  if (cmd == NULL)
  {
    fgaj_e("no 'invoke' key in invoker conf at %s", inv_conf_path);
    return 1;
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

    char *out = flu_sprintf("var/spool/dis/ret_%s.json", id);

    if (freopen(out, "w", stdout) == NULL)
    {
      //fgaj_r("cwd: %s", getcwd(NULL, 0));
      fgaj_r("failed to reopen child stdout to %s", out);
      return 127;
    }

    if (chdir(invoker_path) != 0)
    {
      fgaj_r("failed to chdir to %s", invoker_path);
      return 127;
    }

    r = execl("/bin/sh", "", "-c", cmd, NULL);

    fgaj_r("execl failed (%i)", r);

    return 127;
  }
  else // parent
  {
    close(pds[0]);

    FILE *f = fdopen(pds[1], "w");
    fputs(fdja_to_json(payload), f);
    fclose(f);

    // over, no wait

    fgaj_i("invoked >%s< pid %i", cmd, i);
  }

  // no resource cleanup, we exit.

  return 0;
}

