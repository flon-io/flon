
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
#include "fl_ids.h"
#include "fl_executor.h"


#define ROW_SIZE 7
  // how many execute message are processed before the executor
  // scans var/spool/exe again


// vars:
//   the current execution and a list of
//   outgoing executes and invokes

char *execution_id = NULL;
char *execution_path = NULL;
fdja_value *execution = NULL;

static flu_list *msgs = NULL;

//static size_t counter = 0;
  // how many executions got carried out in this session?

void flon_executor_reset()
{
  if (execution_id) free(execution_id); execution_id = NULL;
  if (execution_path) free(execution_path); execution_path = NULL;
  if (execution) fdja_free(execution); execution = NULL;
  if (msgs) flu_list_free(msgs); msgs = NULL;
  //counter = 0;
}

static void reject(const char *reason, const char *fname, fdja_value *j)
{
  short own_fname = 0;

  if (fname == NULL)
  {
    fname = fdja_lookup_string(j, "fname", NULL);
    own_fname = 1;
  }
  if (fname == NULL)
  {
    fgaj_w("cannot reject msg without 'fname' key");
    char *s  = fdja_to_json(j); fgaj_d("no fname in: %s", s); free(s);
    return;
  }

  flu_move("var/spool/exe/%s", fname, "var/spool/rejected/%s", fname);
  fgaj_i("%s, rejected %s", reason, fname);

  if (own_fname) free((char *)fname);
}

void flon_queue_msg(char *type, char *nid, char *from_nid, fdja_value *payload)
{
  fdja_value *msg = fdja_v("{ %s: 1, nid: %s }", type, nid);

  fdja_set(msg, *type == 'e' ? "parent" : "from", fdja_s(from_nid));
  fdja_set(msg, "payload", payload ? fdja_clone(payload) : fdja_v("{}"));

  flu_list_add(msgs, msg);
}


static fdja_value *create_node(
  const char *nid, const char *instruction, fdja_value *tree)
{
  fdja_value *node = fdja_v("{ nid: %s, t: %s }", nid, instruction);
  if (strcmp(nid, "0") == 0) fdja_set(node, "tree", fdja_clone(tree));

  fdja_pset(execution, "nodes.%s", nid, node);

  return node;
}

static void move_to_processed(fdja_value *msg)
{
  char *fname = fdja_ls(msg, "fname", NULL);
  if (fname == NULL) return;

  char *fep = execution_path;

  if (flu_mkdir_p("var/run/%s/processed", fep, 0755) != 0)
  {
    fgaj_r("failed to mkdir -p var/run/%s/processed", fep);
  }

  if (flu_move("var/spool/exe/%s", fname, "var/run/%s/processed/", fep) != 0)
  {
    fgaj_r("failed to move %s to var/run/%s/processed", fname, fep);
  }

  free(fname);
}

static void handle(fdja_value *msg)
{
  //fgaj_i("%s", fdja_tod(msg));

  char *nid = NULL;
  fdja_value *action = NULL;
  fdja_value *tree = NULL;
  char *instruction = NULL;
  fdja_value *node = NULL;

  char a = 'x'; action = fdja_l(msg, "execute");
  if (action == NULL) { a = 'r'; action = fdja_l(msg, "receive"); }

  //fgaj_i("a: %c", a);

  nid = fdja_lsd(msg, "nid", "0");

  if (a == 'x') { tree = action; }
  if (tree == NULL || tree->type != 'a') tree = flon_node_tree(nid);
  if (tree == NULL) { reject("node not found", NULL, msg); goto _over; }

  instruction = fdja_ls(tree, "0", NULL);

  //fgaj_i("%c _ %s", a, instruction);

  if (a == 'x')
    node = create_node(nid, instruction, tree);
  else // a == 'r'
    node = fdja_l(execution, "nodes.%s", nid);

  if (a == 'x') fdja_set(msg, "tree", fdja_clone(tree));

  fgaj_d("%s%-11s %s", a == 'x' ? "exe_" : "rcv_", instruction, nid);

  flon_instruction *inst = flon_instruction_lookup(a, instruction);
  if (inst == NULL) goto _over;

  char r = inst(node, msg);

  //fgaj_d("%c %s --> %c", a, instruction, r);

  // v, k, r

  if (r == 'v') // over
  {
    char *parent_nid = flon_node_parent_nid(nid);
    //
    if (parent_nid)
    {
      flon_queue_msg("receive", parent_nid, nid, fdja_l(msg, "payload", NULL));
      free(parent_nid);
    }

    fdja_pset(execution, "nodes.%s", nid, NULL); // over
  }
  else if (r == 'k') // ok
  {
    // TODO
  }
  else // error
  {
    // TODO
  }

  move_to_processed(msg);

_over:

  if (nid) free(nid);
  if (instruction) free(instruction);
}

static void load_execution(const char *exid)
{
  if (execution_id) return;

  //fgaj_d("exid: %s", exid);

  execution_id = strdup((char *)exid);
  execution_path = flon_exid_path(execution_id);

  //char c = flu_fstat("var/run/%s.json", exid);
  //if (c == 0) c = '0';
  //fgaj_d("var/run/%s.json: %c", exid, c);

  execution = fdja_parse_f("var/run/%s/run.json", execution_path);

  if (execution == NULL)
  {
    execution = fdja_v(
      "{ exid: \"%s\", nodes: {}, errors: {} }", exid);
  }

  msgs = flu_list_malloc();
}

static int name_matches(const char *n)
{
  if (
    strncmp(n, "exe_", 4) != 0 &&
    strncmp(n, "ret_", 4) != 0 &&
    strncmp(n, "rcv_", 4) != 0
  ) return 0;

  size_t l = strlen(execution_id);

  if (n[4 + l] != '.' && n[4 + l] != '-') return 0;
  return strncmp(n + 4, execution_id, l) == 0;
}

static void load_msgs()
{
  //fgaj_d("exid: %s", execution_id);

  DIR *dir = opendir("var/spool/exe/");
  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if ( ! name_matches(de->d_name)) continue;

    fgaj_i("from %s", de->d_name);

    fdja_value *j = fdja_parse_obj_f("var/spool/exe/%s", de->d_name);

    if (j == NULL) { reject("couldn't parse", de->d_name, NULL); continue; }

    fdja_set(j, "fname", fdja_s(de->d_name));

    // TODO
    // it's probably better to put msgs from disk in front of the list
    // especially if they're cancel msgs

    flu_list_add(msgs, j);
  }

  fgaj_i("exid: %s, msgs: %zu", execution_id, msgs->size);

  closedir(dir);
}

static void persist()
{
  int r;

  r = fdja_to_json_f(execution, "var/run/%s/run.json", execution_path);

  if (r != 1)
  {
    fgaj_r(
      "failed to persist execution to var/run/%s/run.json", execution_path);

    // TODO: eventually persist to some dump?

    return;
  }

  if (fdja_size(fdja_lookup(execution, "nodes")) < 1)
  {
    // flow is over, archive it

    r = flu_mkdir_p("var/archive/%s", execution_path, 0755);
      // FIXME: prevent var/archive/{kk}/{exid}/{exid}/ path

    if (r != 0)
    {
      fgaj_r("failed to mkdir -p var/archive/%s", execution_path);
      return;
    }

    r = flu_move(
      "var/run/%s", execution_path,
      "var/archive/%s", execution_path);

    if (r != 0)
    {
      fgaj_r("failed to move execution to var/archive/%s/", execution_path);
    }
  }

  //r = fdja_to_json_f(execution, "var//%s.json", execution_id);
  //if (r != 1)
  //{
  //  fgaj_r(
  //    "failed to archive execution to var/run/processed/%s.json",
  //    execution_id);
  //  return;
  //}
  //r = flu_unlink("var/run/%s.json", execution_id);
  //if (r != 0)
  //{
  //  fgaj_r("failed to unlink execution at var/run/%s.json", execution_id);
  //}
}

static void execute()
{
  while (1)
  {
    load_msgs();

    for (size_t i = 0; i < ROW_SIZE; ++i)
    {
      fdja_value *j = flu_list_shift(msgs);

      if (j == NULL) break;

      //fgaj_i(fdja_tod(j));

      if (fdja_lookup(j, "execute") || fdja_lookup(j, "receive"))
        handle(j);
      else
        reject("no 'execute' or 'receive' key", NULL, j);

      fdja_free(j);
    }

    if (msgs->size < 1) break;
  }

  persist();
}

int flon_execute(const char *exid)
{
  fgaj_i(exid);

  load_execution(exid);
  execute();

  return 0; // success
}

