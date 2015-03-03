
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

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "flutim.h"
#include "djan.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_executor.h"


#define ROW_SIZE 7
  // how many execute message are processed before the executor
  // scans var/spool/exe again


char *execution_id = NULL;
char *execution_path = NULL;
FILE *msg_log = NULL;
fdja_value *execution = NULL;

static flu_list *msgs = NULL;


short flon_is_transient_execution()
{
  return (execution_path == NULL && msg_log == NULL);
}

char *flon_execution_domain()
{
  if (execution_id == NULL) return NULL;

  char *dash = strchr(execution_id, '-');

  return strndup(execution_id, dash - execution_id);
}

char *flon_execution_domain_delta(int i)
{
  if (execution_id == NULL) return NULL;

  char *r = flon_execution_domain();

  if (i == 0) return r;

  char *end = r;

  for (size_t j = 0; ; ++j)
  {
    if (i > 0)
    {
      if (j == i) break;
      end = strchr(end + 1, '.');
    }
    else
    {
      if (j == -i) break;
      end = strrchr(r, '.'); if (end == NULL) break;
      *end = 0;
    }
  }

  if (end) *end = 0;

  return r;
}

static void do_log(fdja_value *msg)
{
  if (msg_log == NULL) return;

  //flon_stamp(msg, "\be"); // 'e'xecuted at... at the beginning (backslash-b)

  char *now = fgaj_now();

  fputs(now, msg_log);
  fputc(' ', msg_log);
  fdja_to_d(msg_log, msg, FDJA_F_COMPACT, 0);
  fputc('\n', msg_log);

  fflush(msg_log);

  free(now);
}

void flon_queue_msg(
  const char *type, const char *nid, const char *from_nid, fdja_value *m)
{
  fgaj_i("%s %s from %s", type, nid, from_nid);

  fdja_value *msg = m ? m : fdja_object_malloc();

  fdja_psetv(msg, "point", type);

  if (nid)
    fdja_set(msg, "nid", fdja_s(nid));
  else
    fdja_set(msg, "nid", fdja_v("null"));

  fdja_set(
    msg,
    *type == 'e' ? "parent" : "from", fdja_s(from_nid));

  if (fdja_l(msg, "payload") == NULL && (*type == 'e' || *type == 'r'))
  {
    fdja_set(msg, "payload", fdja_object_malloc());
  }

  flu_list_add(msgs, msg);
}

void flon_schedule_msg(
  const char *type, const char *ts, const char *nid,
  fdja_value *tree0, fdja_value *tree1,
  fdja_value *msg)
{
  if (flon_is_transient_execution()) return;
    // no schedules when running transient executions

  //flu_putf(fdja_todc(msg));

  fgaj_i("%sschedule %s %s from -%s", msg ? "" : "un", type, ts, nid);

  fdja_value *m = fdja_object_malloc();
  fdja_psetv(m, "point", msg ? "schedule" : "unschedule");
  fdja_set(m, type, fdja_s(ts));
  if (msg)
  {
    fdja_set(m, "tree0", tree0);
    fdja_set(m, "tree1", tree1);
    fdja_set(m, "msg", msg);
  }

  if (
    flon_lock_write(m, "var/spool/dis/sch_%s-%s.json", execution_id, nid) != 1)
  {
    fgaj_r(
      "failed to write to var/spool/dis/sch_%s-%s.json", execution_id, nid);
  }

  do_log(m);

  fdja_free(m);
}

void flon_unschedule_msg(
  const char *type, const char *ts, const char *nid)
{
  flon_schedule_msg(type, ts, nid, NULL, NULL, NULL);
}

static fdja_value *create_node(
  fdja_value *msg, char *nid, char *parent_nid, fdja_value *tree)
{
  fdja_value *node = fdja_object_malloc();

  //fdja_set(
  //  node, "inst", fdja_lc(tree, "0"));
    // done at the rewrite step now

  fdja_set(
    node, "nid", fdja_s(nid));
  fdja_set(
    node, "ctime", fdja_sym(flu_tstamp(NULL, 1, 'u')));
  fdja_set(
    node, "parent", parent_nid ? fdja_s(parent_nid) : fdja_v("null"));

  if (strcmp(nid, "0") == 0) fdja_set(node, "tree", fdja_clone(tree));

  fdja_value *vars = fdja_l(msg, "vars");
  if (vars) fdja_set(node, "vars", fdja_clone(vars));
  else if (strcmp(nid, "0") == 0) fdja_set(node, "vars", fdja_object_malloc());

  fdja_pset(execution, "nodes.%s", nid, node);

  //puts(fdja_todc(execution));

  return node;
}

static void handle_execute(char order, fdja_value *msg)
{
  fgaj_i("%c", order);

  char *fname = fdja_ls(msg, "fname", NULL);
  char *nid = fdja_lsd(msg, "nid", "0");

  fdja_value *tree = fdja_l(msg, "tree");

  if (tree == NULL || tree->type != 'a')
  {
    tree = flon_node_tree(nid);
  }
  if (tree == NULL)
  {
    flon_move_to_rejected("var/spool/exe/%s", fname, "tree not found");
    free(fname); free(nid);
    return;
  }

  char *parent_nid = fdja_ls(msg, "parent", NULL);
  fdja_value *payload = fdja_l(msg, "payload");
  fdja_value *node = create_node(msg, nid, parent_nid, tree);

  fdja_set(msg, "tree", fdja_clone(tree));

  /*int rewritten = */flon_rewrite_tree(node, msg);

  if (parent_nid == NULL && strcmp(nid, "0") == 0)
  {
    flon_queue_msg(
      "launched", nid, NULL, fdja_o("payload", fdja_clone(payload), NULL));
  }

  //
  // perform instruction

  char r = flon_call_instruction(order, node, msg);

  //
  // v, k, r, handle instruction result

  if (r == 'v') // over
  {
    flon_queue_msg(
      "receive", nid, nid, fdja_o("payload", fdja_clone(payload), NULL));
  }
  else if (r == 'k') // ok
  {
    // nichts
  }
  else // error, 'r' or '?'
  {
    flon_queue_msg(
      "failed", nid, parent_nid,
      fdja_o(
        "payload", fdja_clone(payload),
        "error", fdja_lc(node, "errors.-1"),
        NULL));
  }

  if (fname) flon_move_to_processed("var/spool/exe/%s", fname);

  do_log(msg);

  free(fname);
  free(nid);
  free(parent_nid);
}

static void log_delta(fdja_value *node)
{
  char *tsp = fdja_ls(node, "ctime", NULL);
  //fgaj_d("tsp: %s", tsp);
  struct timespec *tsc = flu_parse_tstamp(tsp, 1);
  struct timespec *delta = flu_tdiff(NULL, tsc);
  char *sdelta = flu_ts_to_hs(delta, 'n');

  fgaj_d("delta: %s", sdelta);

  free(sdelta);
  free(delta);
  free(tsc);
  free(tsp);
}

static void handle_return(char order, fdja_value *msg)
{
  fgaj_i("%c", order);

  char *nid = fdja_lsd(msg, "nid", "0");
  char *fname = fdja_ls(msg, "fname", NULL);
  char *parent_nid = NULL;

  fdja_value *node = fdja_l(execution, "nodes.%s", nid);

  if (node == NULL)
  {
    flon_move_to_rejected("var/spool/exe/%s", fname, "node not found");
    goto _over;
  }

  parent_nid = fdja_ls(msg, "parent", NULL);

  fdja_value *payload = fdja_l(msg, "payload");

  //fgaj_d("%c %s", order, instruction);

  //
  // perform instruction

  char r = flon_call_instruction(order, node, msg);

  //
  // v, k, r, handle instruction result

  if (r == 'v') // over
  {
    free(parent_nid); parent_nid = flon_parent_nid(nid);

    if (parent_nid)
    {
      flon_queue_msg(
        "receive", parent_nid, nid,
        fdja_o("payload", fdja_clone(payload), NULL));
    }
    else
    {
      log_delta(node); // log (debug) the age of the execution

      if (strcmp(nid, "0") == 0)
      {
        flon_queue_msg(
          "terminated", nid, NULL,
          fdja_o("payload", fdja_clone(payload), NULL));
      }
      else
      {
        flon_queue_msg(
          "ceased", nid, NULL,
          fdja_o("payload", fdja_clone(payload), NULL));
      }
    }

    fdja_pset(execution, "nodes.%s", nid, NULL); // remove node
  }
  else if (r == 'k') // ok
  {
    // nichts
  }
  else // error, 'r' or '?'
  {
    flon_queue_msg(
      "failed", nid, parent_nid,
      fdja_o(
        "payload", fdja_clone(payload),
        "error", fdja_lc(node, "errors.-1"),
        NULL));
  }

  if (fname) flon_move_to_processed("var/spool/exe/%s", fname);

  do_log(msg);

_over:

  free(nid);
  free(parent_nid);
  free(fname);
}

static void handle_event(char event, fdja_value *msg)
{
  //printf("event: '%c'\n", event);
  //flu_putf(fdja_todc(msg));

  do_log(msg);
}

static void reject_or_discard_msg(char point, fdja_value *msg)
{
  char *fname = fdja_ls(msg, "fname", NULL);

  if (fname)
    flon_move_to_rejected(fname, "no 'point' key");
  else
    fgaj_w("no 'point' key in message, discarding.");

  free(fname);
}

static void load_execution(const char *exid)
{
  if (execution_id) return;

  //fgaj_d("exid: %s", exid);

  execution_id = strdup((char *)exid);
  execution_path = flon_exid_path(execution_id);

  // TODO: make msg_log optional!!!

  char *log = flu_sprintf("var/run/%s/msg.log", execution_path);
  char *blog = strdup(log);
  *(strrchr(blog, '/')) = '\0';

  if (flu_mkdir_p(blog, 0755) != 0)
  {
    fgaj_r("couldn't mkdir -p %s", blog);
  }
  free(blog);

  msg_log = fopen(log, "a");

  if (msg_log == NULL)
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
  if (msg_log) { fclose(msg_log); msg_log = NULL; }
  if (execution) fdja_free(execution); execution = NULL;
  if (msgs) flu_list_free(msgs); msgs = NULL;
}

static int name_matches(const char *n)
{
  if (
    strncmp(n, "exe_", 4) != 0 &&
    strncmp(n, "rcv_", 4) != 0 &&
    strncmp(n, "tsk_", 4) != 0 &&
    strncmp(n, "can_", 4) != 0
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

    if (j == NULL)
    {
      flon_move_to_rejected("var/spool/exe/%s", de->d_name, "couldn't parse");
      continue;
    }

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
      //fdja_putdc(j);

      fdja_value *point = fdja_l(j, "point");
      char p = point ? *fdja_srk(point) : 0;

      if (p == 'e')
        handle_execute(p, j);
      else if (p == 'r' || p == 'c') // receive or cancel
        handle_return(p, j);
      else if (p)
        handle_event(p, j);
      else
        reject_or_discard_msg(p, j);

      fdja_free(j);
    }
  }

  persist();
}

void flon_execute(const char *exid)
{
  fgaj_i(exid);

  load_execution(exid);

  execute();

  unload_execution();
}

fdja_value *flon_execut(
  const char *domain, fdja_value *tree, fdja_value *payload, fdja_value *vars)
{
  execution_id = flu_sprintf("%s-transient", domain);
  execution_path = NULL;
  execution = fdja_v("{ exid: \"%s\", nodes: {} }", execution_id);

  msgs = flu_list_malloc();

  fdja_value *msg = fdja_v("{ point: execute, exid: \"%s\" }", execution_id);
  fdja_set(msg, "tree", tree);
  fdja_set(msg, "payload", payload);
  fdja_set(msg, "vars", vars);

  flu_list_add(msgs, msg);

  fdja_value *terminated = NULL;

  while (1)
  {
    fdja_value *m = flu_list_shift(msgs); if (m == NULL) break;

    fdja_value *point = fdja_l(m, "point");
    char p = point ? *fdja_srk(point) : 0;

    if (p == 'e')
      handle_execute(p, m);
    else if (p == 'r') // receive
      handle_return(p, m);
    else if (p == 't') // terminated
      { terminated = m; break; }
    //else
      // simply discard

    fdja_free(m);
  }

  unload_execution();

  return terminated;
}

