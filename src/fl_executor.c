
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

#include "flutim.h"
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
FILE *msgs_log = NULL;
fdja_value *execution = NULL;

static flu_list *msgs = NULL;

//static size_t counter = 0;
  // how many executions got carried out in this session?

static void reject(const char *reason, const char *fname, fdja_value *j)
{
  fgaj_d("reason: %s", reason);

  short own_fname = 0;

  if (fname == NULL)
  {
    fname = fdja_lookup_string(j, "fname", NULL);
    own_fname = 1;
  }
  if (fname == NULL)
  {
    fgaj_w("cannot reject msg without 'fname' key");
    char *s = fdja_to_json(j); fgaj_d("no fname in: %s", s); free(s);
    return;
  }

  flu_move("var/spool/exe/%s", fname, "var/spool/rejected/%s", fname);
  fgaj_i("%s, rejected %s", reason, fname);

  if (own_fname) free((char *)fname);
}

void flon_queue_msg(char *type, char *nid, char *from_nid, fdja_value *payload)
{
  fgaj_i("%s %s from %s", type, nid, from_nid);

  fdja_value *msg = fdja_v("{ point: %s, nid: '%s' }", type, nid);

  fdja_set(msg, *type == 'e' ? "parent" : "from", fdja_s(from_nid));
  fdja_set(msg, "payload", payload ? fdja_clone(payload) : fdja_v("{}"));

  flu_list_add(msgs, msg);
}


static fdja_value *create_node(
  const char *nid,
  const char *parent_nid,
  const char *instruction,
  fdja_value *tree)
{
  fdja_value *node = fdja_v("{ nid: '%s', inst: %s }", nid, instruction);

  fdja_set(
    node, "created", fdja_sym(flu_tstamp(NULL, 1, 'u')));
  fdja_set(
    node, "parent", parent_nid ? fdja_s((char *)parent_nid) : fdja_v("null"));

  if (tree && strcmp(nid, "0") == 0) fdja_set(node, "tree", fdja_clone(tree));

  fdja_pset(execution, "nodes.%s", nid, node);

  //puts(fdja_todc(execution));

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

static void do_log(fdja_value *msg)
{
  if (msgs_log == NULL) return;

  flon_stamp(msg, "\be"); // 'e'xecuted at... at the beginning (backslash-b)

  fdja_to_d(msgs_log, msg, FDJA_F_COMPACT, 0);
  fputc('\n', msgs_log);
  fflush(msgs_log);
}

static void handle(fdja_value *msg)
{
  //fgaj_i("%s", fdja_tod(msg));
  //flu_putf(fdja_todc(msg));

  char *nid = NULL;
  char *parent_nid = NULL;
  //fdja_value *action = NULL;
  fdja_value *tree = NULL;
  char *instruction = NULL;
  fdja_value *node = NULL;

  fdja_value *point = fdja_l(msg, "point");
  char *spoint = point ? fdja_srk(point) : "?";

  char a = 'x';
  if (*spoint == 'r') a = 'r'; // receive

  fgaj_i("a: %c", a);

  nid = fdja_lsd(msg, "nid", "0");
  parent_nid = fdja_ls(msg, "parent", NULL);

  //if (a == 'x') { tree = action; }
  tree = fdja_l(msg, "tree");
  if (tree == NULL || tree->type != 'a') tree = flon_node_tree(nid);
  if (tree == NULL) { reject("node not found", NULL, msg); goto _over; }

  instruction = fdja_ls(tree, "0", NULL);

  //fgaj_i("%c _ %s", a, instruction);

  if (a == 'x')
    node = create_node(nid, parent_nid, instruction, tree);
  else // a == 'r'
    node = fdja_l(execution, "nodes.%s", nid);

  if (a == 'x') fdja_set(msg, "tree", fdja_clone(tree));

  fgaj_d("%-*s%s %c %s", flon_nid_depth(nid) * 2, "", nid, a, instruction);

  flon_instruction *inst = flon_instruction_lookup(a, instruction);
  //if (inst == NULL) goto _over;

  char r = '?'; // error
  if (inst) r = inst(node, msg);

  fgaj_i("%c_%s --> %c", a, instruction, r);

  // v, k, r

  if (r == 'v') // over
  {
    if (a == 'x')
    {
      flon_queue_msg("receive", nid, nid, fdja_l(msg, "payload", NULL));
    }
    else // (a == 'r')
    {
      char *parent_nid = flon_node_parent_nid(nid);
      //if ( ! parent_nid) parent_nid = strdup("0");

      fgaj_i("parent_nid: %s", parent_nid);

      if (parent_nid)
      {
        flon_queue_msg(
          "receive", parent_nid, nid, fdja_l(msg, "payload", NULL));
        free(parent_nid);
      }
      else
      {
        char *tsp = fdja_ls(node, "created", NULL);
        //fgaj_d("tsp: %s", tsp);
        struct timespec *tsc = flu_parse_tstamp(tsp, 1);
        struct timespec *delta = flu_tdiff(NULL, tsc);
        char *sdelta = flu_ts_to_s(delta, 'n');
        fgaj_d("delta: %s", sdelta);
        free(sdelta);
        free(delta);
        free(tsc);
        free(tsp);
          // TODO: package most of that into a flu_ method
      }

      fdja_pset(execution, "nodes.%s", nid, NULL); // remove node
    }
  }
  else if (r == 'k') // ok
  {
    // TODO
  }
  else // error, 'r'?
  {
    // TODO
    fgaj_w("ERROR!!!!!!!!!!!!!!!!!!!!!!!");
  }

  move_to_processed(msg);
  do_log(msg);

_over:

  if (nid) free(nid);
  if (parent_nid) free(parent_nid);
  if (instruction) free(instruction);
}

static void load_execution(const char *exid)
{
  if (execution_id) return;

  //fgaj_d("exid: %s", exid);

  execution_id = strdup((char *)exid);
  execution_path = flon_exid_path(execution_id);

  // TODO: make msgs_log optional!!!

  char *log = flu_sprintf("var/run/%s/msgs.log", execution_path);
  char *blog = strdup(log);
  *(strrchr(blog, '/')) = '\0';

  if (flu_mkdir_p(blog, 0755) != 0)
  {
    fgaj_r("couldn't mkdir -p %s", blog);
  }
  free(blog);

  msgs_log = fopen(log, "a");

  if (msgs_log == NULL)
  {
    fgaj_r("couldn't open %s for appending", log);
  }
  free(log);

  execution = fdja_parse_f("var/run/%s/run.json", execution_path);

  if (execution == NULL)
  {
    execution = fdja_v("{ exid: \"%s\", nodes: {} }", exid);
  }

  msgs = flu_list_malloc();
}

static void unload_execution()
{
  if (execution_id) free(execution_id); execution_id = NULL;
  if (execution_path) free(execution_path); execution_path = NULL;
  if (msgs_log) { fclose(msgs_log); msgs_log = NULL; }
  if (execution) fdja_free(execution); execution = NULL;
  if (msgs) flu_list_free(msgs); msgs = NULL;
  //counter = 0;
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
  DIR *dir = opendir("var/spool/exe/");
  struct dirent *de;

  while ((de = readdir(dir)) != NULL)
  {
    if ( ! name_matches(de->d_name)) continue;

    //fgaj_t("from %s", de->d_name);

    fdja_value *j = fdja_parse_f("var/spool/exe/%s", de->d_name);

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

    char *fep = strdup(execution_path);
    *(strrchr(fep, '/')) = '\0';

    r = flu_mkdir_p("var/archive/%s", fep, 0755);

    if (r != 0)
    {
      fgaj_r("failed to mkdir -p var/archive/%s", execution_path);
      return;
    }

    r = flu_move(
      "var/run/%s", execution_path,
      "var/archive/%s", fep);

    free(fep);

    if (r != 0)
    {
      fgaj_r("failed to move execution to var/archive/%s/", execution_path);
    }
  }
}

static void execute()
{
  while (1)
  {
    load_msgs();

    if (msgs->size < 1) break;

    for (size_t i = 0; i < ROW_SIZE; ++i)
    {
      fdja_value *j = flu_list_shift(msgs);

      if (j == NULL) break;

      //fgaj_i(fdja_tod(j));
      //fgaj_i(fdja_to_djan(j, 0));

      if (fdja_l(j, "point"))
        handle(j);
      else
        reject("no 'point' key", NULL, j);

      fdja_free(j);
    }
  }

  persist();
}

int flon_execute(const char *exid)
{
  fgaj_i(exid);

  load_execution(exid);

  execute();

  unload_execution();

  return 0; // success
}

