
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

#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "djan.h"
#include "gajeta.h"
#include "fl_executor.h"


#define ROW_SIZE 7
  // how many execute message are processed before the executor
  // scans /var/spool/exe again


// global vars:
//   the current execution and a list of
//   outgoing executes and invokes

static char *execution_id = NULL;
static fdja_value *execution = NULL;

static flu_list *executes = NULL;
static flu_list *errors = NULL;

static size_t counter = 0;
  // how many executions got carried out in this session?


//
// execute and receive functions

typedef int flon_exe_func(fdja_value *);

// *** INVOKE

static int exe_invoke(fdja_value *exe)
{
  char *nid = "0-0";

  fdja_value *inv = fdja_v("{ exid: \"%s\", nid: \"%s\" }", execution_id, nid);
  fdja_set(inv, "invoke", fdja_lookup_c(exe, "execute"));
  fdja_set(inv, "payload", fdja_lookup_c(exe, "payload"));

  fdja_pset(inv, "payload.args", fdja_lookup_c(exe, "execute.1"));

  fdja_to_json_f(inv, "var/spool/inv/inv_%s-%s.json", execution_id, nid);

  fdja_value_free(inv);

  return 0; // success
}

static int rcv_invoke(fdja_value *rcv)
{
  return 1; // failure
}

// function table

typedef struct {
  char *name;
  flon_exe_func *exe;
  flon_exe_func *rcv;
} flon_name_funcs;

static flon_name_funcs *name_functions[] = {
  &(flon_name_funcs){ "invoke", exe_invoke, rcv_invoke },
  NULL
};

static int receive_j(fdja_value *msg)
{
  // TODO

  return 1; // failure
}

static int execute_j(fdja_value *msg)
{
  fdja_value *x = fdja_lookup(msg, "execute");
  char *name = fdja_lookup_string(x, "0", NULL);
  flon_exe_func *func = NULL;

  fgaj_d("node name: '%s'", name);

  for (size_t i = 0; name_functions[i] != NULL; ++i)
  {
    flon_name_funcs *nf = name_functions[i];
    if (strcmp(nf->name, name) == 0) { func = nf->exe; break; }
  }

  if (func == NULL)
  {
    fgaj_e("don't know how to execute \"%s\"", name);
    free(name);
    return 1;
  }

  // TODO: create node...

  int r = func(msg);

  char *fname = fdja_lookup_string(msg, "fname", NULL);
  if (fname) flu_move("var/spool/exe/%s", fname, "var/spool/processed");

  free(name);
  if (fname) free(fname);

  return r;
}

static void load_execution(const char *exid)
{
  if (execution_id) return;

  execution_id =
    (char *)exid;
  execution =
    fdja_v("{ exid: \"%s\", trees: {}, nodes: {}, errors: {} }", exid);

  executes = flu_list_malloc();
  errors = flu_list_malloc();
}

static void reject(const char *reason, const char *fname, fdja_value *j)
{
  if (fname == NULL)
  {
    fname = fdja_lookup_string(j, "fname", NULL);
  }
  if (fname == NULL)
  {
    fgaj_w("cannot reject msg without 'fname' key");
    char *s  = fdja_to_json(j); fgaj_d("no fname in: %s", s); free(s);
    return;
  }

  flu_move("var/spool/exe/%s", fname, "var/spool/rejected/%s", fname);
  fgaj_i("%s, rejected %s", reason, fname);
}

static int name_matches(const char *n)
{
  if (strncmp(n, "exe_", 4) != 0 && strncmp(n, "rcv_", 4) != 0) return 0;
  size_t l = strlen(execution_id);
  if (n[4 + l] != '.') return 0;
  return strncmp(n + 4, execution_id, l) == 0;
}

static void load_executes()
{
  fgaj_d("exid: %s", execution_id);

  DIR *dir = opendir("var/spool/exe/");
  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if ( ! name_matches(de->d_name)) continue;

    fgaj_d("fname: %s", de->d_name);

    fdja_value *j = fdja_parse_obj_f("var/spool/exe/%s", de->d_name);

    if (j == NULL) { reject("couldn't parse", de->d_name, NULL); continue; }

    fdja_set(j, "fname", fdja_s(de->d_name));

    flu_list_add(executes, j);
  }

  closedir(dir);
}

static void execute()
{
  while (1)
  {
    load_executes();

    for (size_t i = 0; i < ROW_SIZE; ++i)
    {
      fdja_value *j = flu_list_shift(executes);

      if (j == NULL) break;

      if (fdja_lookup(j, "execute"))
        execute_j(j);
      else if (fdja_lookup(j, "receive"))
        receive_j(j);
      else
        reject("no 'execute' or 'receive' key", NULL, j);

      fdja_free(j);
    }

    if (executes->size < 1) break;
  }

  // TODO: persist to /var/run/
  //       or archive to /var/run/processed/
}

int flon_execute(const char *exid)
{
  load_execution(exid);
  execute();

  return 0; // success
}

