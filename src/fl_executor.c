
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
#include "fl_node.h"
#include "fl_executor.h"


#define ROW_SIZE 7
  // how many execute message are processed before the executor
  // scans var/spool/exe again


// global vars:
//   the current execution and a list of
//   outgoing executes and invokes

static char *execution_id = NULL;
static fdja_value *execution = NULL;

static flu_list *executes = NULL;
static flu_list *errors = NULL;

static size_t counter = 0;
  // how many executions got carried out in this session?

// used by the specs
//
void flon_executor_reset()
{
  execution_id = NULL;
  if (execution) fdja_free(execution); execution = NULL;
  if (executes) flu_list_free(executes); executes = NULL;
  if (errors) flu_list_free(errors); errors = NULL;
  counter = 0;
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

//
// execute and receive functions

typedef int flon_exe_func(fdja_value *, fdja_value *);

// *** INVOKE

static int exe_invoke(fdja_value *node, fdja_value *exe)
{
  char *nid = fdja_ls(node, "nid");

  fdja_value *inv = fdja_v("{ exid: \"%s\", nid: \"%s\" }", execution_id, nid);
  fdja_set(inv, "invoke", fdja_lc(exe, "tree"));
  fdja_set(inv, "payload", fdja_lc(exe, "payload"));

  fdja_pset(inv, "payload.args", fdja_lc(exe, "tree.1"));

  fdja_to_json_f(inv, "var/spool/inv/inv_%s-%s.json", execution_id, nid);

  fdja_free(inv);
  free(nid);

  return 0; // success
}

static int rcv_invoke(fdja_value *node, fdja_value *rcv)
{
  return 0; // success
}

// *** SEQUENCE

static int exe_sequence(fdja_value *node, fdja_value *exe)
{
  //printf("seq: node: %s\n", fdja_to_json(node));
  //printf("seq: exe: %s\n", fdja_to_json(exe));

  char *nid = fdja_ls(node, "nid", NULL);

  fdja_value *exe0 = fdja_v("{ nid: \"%s.0\", execute: 1 }", nid);
  fdja_set(exe0, "payload", fdja_lc(exe, "payload"));

  //printf("seq: exe0: %s\n", fdja_to_json(exe0));

  free(nid);

  flu_list_add(executes, exe0);

  return 0; // success
}

static int rcv_sequence(fdja_value *node, fdja_value *rcv)
{
  return 0; // success
}

// function table

typedef struct {
  char *name;
  flon_exe_func *exe;
  flon_exe_func *rcv;
} flon_name_funcs;

static flon_name_funcs *name_functions[] = {
  &(flon_name_funcs){ "invoke", exe_invoke, rcv_invoke },
  &(flon_name_funcs){ "sequence", exe_sequence, rcv_sequence },
  NULL
};

static flon_exe_func *find_function(const char *name, char dir)
{
  for (size_t i = 0; name_functions[i] != NULL; ++i)
  {
    flon_name_funcs *nf = name_functions[i];
    if (strcmp(nf->name, name) == 0) return dir == 'r' ? nf->rcv : nf->exe;
  }

  fgaj_e(
    "don't know how to %s \"%s\"",
    dir == 'r' ? "receive" : "execute", name);

  return NULL;
}

static void move_to_processed(fdja_value *msg)
{
  char *fname = fdja_ls(msg, "fname", NULL);
  if (fname == NULL) return;
  flu_move("var/spool/exe/%s", fname, "var/spool/processed");
  free(fname);
}

static int execute_j(fdja_value *msg)
{
  // TODO: string extrapolation

  char *nid = fdja_lsd(msg, "nid", "0");
  fdja_value *tree = fdja_l(msg, "execute");

  if (tree->type == 'n') tree = flon_node_tree(execution, nid);
  fdja_set(msg, "tree", fdja_clone(tree));
  char *name = fdja_ls(tree, "0", NULL);

  fgaj_d("node: \"%s\" %s", name, nid);

  flon_exe_func *func = find_function(name, 'x');
  free(name);
  if (func == NULL) return 1;

  fdja_value *node = fdja_v("{ nid: \"%s\" }", nid);
  if (strcmp(nid, "0") == 0) fdja_set(node, "tree", fdja_lc(msg, "execute"));

  fdja_pset(execution, "nodes.%s", nid, node);

  free(nid);

  int r = func(node, msg);

  // TODO: what if the func signals an error?
  move_to_processed(msg);

  return r;
}

static int receive_j(fdja_value *msg)
{
  //puts(fdja_to_json(msg));

  char *nid = fdja_ls(msg, "nid", NULL);
  fgaj_d("nid: %s", nid);

  fdja_value *node = fdja_l(execution, "nodes.%s", nid);

  if (node == NULL) { reject("node not found", NULL, msg); return 1; }

  //puts(fdja_to_json(node));
  char *name = fdja_ls(node, "tree.0");

  if (node == NULL) { reject("tree.0 not found", NULL, msg); return 1; }

  flon_exe_func *func = find_function(name, 'r');
  free(name);
  if (func == NULL) return 1;

  int r = func(node, msg);

  if (r == 0) fdja_pset(execution, "nodes.%s", nid, NULL);

  if (nid) free(nid);

  // TODO: what if the func signals an error?
  move_to_processed(msg);

  return r;
}

static void load_execution(const char *exid)
{
  if (execution_id) return;

  fgaj_d("exid: %s", exid);

  execution_id = (char *)exid;

  //char c = flu_fstat("var/run/%s.json", exid);
  //if (c == 0) c = '0';
  //fgaj_d("var/run/%s.json: %c", exid, c);

  execution = fdja_parse_f("var/run/%s.json", exid);

  if (execution == NULL)
  {
    execution = fdja_v(
      "{ exid: \"%s\", nodes: {}, errors: {} }", exid);
  }

  executes = flu_list_malloc();
  errors = flu_list_malloc();
}

static int name_matches(const char *n)
{
  // TODO: use fl_ids.c functions

  if (
    strncmp(n, "exe_", 4) != 0 &&
    strncmp(n, "ret_", 4) != 0 &&
    strncmp(n, "rcv_", 4) != 0
  ) return 0;

  size_t l = strlen(execution_id);

  if (n[4 + l] != '.' && n[4 + l] != '-') return 0;
  return strncmp(n + 4, execution_id, l) == 0;
}

static void load_executes()
{
  //fgaj_d("exid: %s", execution_id);

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

  fgaj_d("exid: %s, executes: %zu", execution_id, executes->size);

  closedir(dir);
}

static void persist()
{
  int r;

  if (fdja_size(fdja_lookup(execution, "nodes")) > 0)
  {
    // flow is running, persist it

    r = fdja_to_json_f(execution, "var/run/%s.json", execution_id);

    if (r == 1) return;

    fgaj_r("failed to persist execution to var/run/%s.json", execution_id);
    return;
  }

  // else, flow is over, archive it

  r = fdja_to_json_f(execution, "var/run/processed/%s.json", execution_id);

  if (r != 1)
  {
    fgaj_r(
      "failed to archive execution to var/run/processed/%s.json",
      execution_id);
    return;
  }

  r = flu_unlink("var/run/%s.json", execution_id);

  if (r != 0)
  {
    fgaj_r("failed to unlink execution at var/run/%s.json", execution_id);
  }
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

  persist();
}

int flon_execute(const char *exid)
{
  load_execution(exid);
  execute();

  return 0; // success
}

