
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
#include "djan.h"
#include "fl_common.h"
#include "fl_invoker.h"


void flon_invoke_j(fdja_value *j)
{
  fdja_value *invocation = fdja_lookup(j, "invocation");
  fdja_value *payload = fdja_lookup(j, "payload");

  // TODO invocation == NULL case

  char *invid = fdja_lookup_string(payload, "_invocation_id", NULL);

  char *invoked = fdja_to_string(invocation->child);

  char *d = flon_conf_string("invoker.dir", ".");
  char *dir = realpath(d, NULL);

  char *path = flu_sprintf("%s/usr/local/invokers/%s", dir, invoked);

  printf("invoker dir: %s\n", dir);
  printf("invoker path: %s\n", path);

  char *inv_conf_path = flu_sprintf("%s/flon.json", path);
  fdja_value *inv_conf = fdja_parse_obj_f(inv_conf_path);

  // TODO conf == NULL case

  //printf("inv_conf: %s\n", fdja_to_json(inv_conf));

  int pds[2];

  int r = pipe(pds);

  // TODO case where r != 0 (errno is set)

  pid_t i = fork();

  if (i < 0) // failure
  {
    // TODO
  }
  else if (i == 0) // child
  {
    char *out = flu_sprintf("%s/var/spool/in/inv_%s_ret.json", dir, invid);
    char *err = flu_sprintf("%s/var/log/invocations/%s.txt", dir, invid);

    close(pds[1]);
    dup2(pds[0], STDIN_FILENO);
    //close(pds[0]);

    chdir(path);
    freopen(out, "w", stdout);
    freopen(err, "w", stderr);

    i = setsid();
    //
    if (i == -1)
    {
      // TODO case where i == -1
    }

    execl(
      "/bin/sh", "",
      "-c", fdja_lookup_string(inv_conf, "invoke", "cat"),
      NULL);
    //_exit(127); // popen's lead
  }
  else // parent
  {
    close(pds[0]);

    FILE *f = fdopen(pds[1], "w");
    fputs(fdja_to_json(payload), f);
    fclose(f);

    // over, no wait
  }

  // no resource cleanup, we exit.
}

