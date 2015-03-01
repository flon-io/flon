
//
// helpers for n/ and z/ specs
//
// Fri Oct 24 12:53:40 JST 2014
//


#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "flutil.h"
#include "flutim.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"


static void nlog(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *fo = flu_sprintf("[1;30m.:n:. %s[0;0m\n", format);
  vprintf(fo, ap);
  free(fo);
  va_end(ap);
}

char *dispatcher_path = "../tst/bin/flon-dispatcher";
pid_t dispatcher_pid = -1;

static int logtoterm()
{
  char *s = getenv("FLONLOG");
  return s && strstr(s, "term");
}

static void hlp_dispatcher_stop()
{
  if (dispatcher_pid < 1) return;

  if (kill(dispatcher_pid, SIGTERM) == 0)
  {
    int status; waitpid(dispatcher_pid, &status, 0);

    nlog("stopped dispatcher pid: %i status: %i", dispatcher_pid, status);
  }
  else
  {
    nlog("stopped dispatcher pid: %i -> %s", dispatcher_pid, strerror(errno));
  }

  dispatcher_pid = -1;
}

void hlp_dispatcher_start()
{
  if (dispatcher_pid > 0) return;

  if (atexit(hlp_dispatcher_stop) != 0)
  {
    puts("couldn't register dispatcher_stop() from atexit()");
    exit(1);
  }

  chdir("../tst");
  flon_configure(".");

  dispatcher_pid = fork();

  if (dispatcher_pid == 0)
  {
    if ( ! logtoterm())
    {
      freopen("var/log/dispatcher.log", "a", stdout);
      fflush(stdout);
      freopen("var/log/dispatcher.log", "a", stderr);
      fflush(stderr);
    }

    char *v = getenv("FLONVAL");

    if (v && (strstr(v, "dis") || strstr(v, "all")))
    {
      //execle(
      //  "/usr/bin/valgrind", "v n_flon-dispatcher",
      //  "--leak-check=full", "-v", dispatcher_path, NULL,
      //  (char *[]){ flu_sprintf("FLONVAL=%s", v), NULL });
      execl(
        "/usr/bin/valgrind", "v n_flon-dispatcher",
        "--leak-check=full", "-v", dispatcher_path, NULL);
    }
    else
    {
      execl(
        dispatcher_path, "n_flon-dispatcher",
        NULL);
    }

    // fail zone...

    perror("execl failed"); exit(1);
  }
  else
  {
    //if (logtoterm()) nlog("dispatcher started pid: %i...", dispatcher_pid);
    nlog("dispatcher started pid: %i...", dispatcher_pid);

    //sleep(1);
    flu_do_msleep(700);
  }
}

void hlp_dispatcher_sighup()
{
  if (dispatcher_pid < 1) return;

  if (kill(dispatcher_pid, SIGHUP) == 0) return;

  char *s =
    flu_sprintf("sending SIGHUP to dispatcher %i failed", dispatcher_pid);

  perror(s);

  free(s);
}

void hlp_launch_v(char *exid, char *flow, char *payload, char *vars)
{
  char *fep = flon_exid_path(exid);

  // launch

  fdja_value *fl = fdja_parse_radial(strdup(flow), "sfeu");
  if (fl == NULL) { nlog("couldn't parse radial flow..."); return; }

  fdja_value *pl = fdja_v(payload);
  if (pl == NULL) { nlog("couldn't parse payload..."); return; }

  fdja_value *vs = vars ? fdja_v(vars) : NULL;
  if (vars && vs == NULL) { nlog("couldn't parse vars..."); return; }

  fdja_value *v = fdja_v("{ exid: %s }", exid);

  fdja_psetv(v, "point", "execute");
  fdja_set(v, "tree", fl);
  fdja_set(v, "payload", pl);
  if (vs) fdja_set(v, "vars", vs);

  int i = fdja_to_json_f(v, "var/spool/dis/exe_%s.json", exid);
  if (i != 1)
  {
    nlog("failed to write launch file at var/spool/dis/exe_%s.json", exid);
    return;
  }
  //flu_system("touch var/spool/dis/");

  fdja_free(v);
  free(fep);
}

void hlp_launch(char *exid, char *flow, char *payload)
{
  hlp_launch_v(exid, flow, payload, NULL);
}

int hlp_cancel(char *exid, char *nid)
{
  if (nid == NULL) nid = "0";
  fdja_value *msg = fdja_v("{ point: cancel }");
  int r = flon_lock_write(msg, "var/spool/dis/can_%s-%s.json", exid, nid);
  fdja_free(msg);

  return r;
}

static fdja_value *scan(char *s, char *point, char *nid)
{
  fdja_value *r = NULL;
  char *ss = NULL;
  char *n = NULL;

  char *br = strrchr(s, '}');
  if (br == NULL) return NULL;

  *(br + 1) = 0;
  char *lf = strrchr(s, '\n');
  ss = strdup(strchr(lf ? lf : s, '{'));

  //printf("scan() >>%s<\n", ss);

  fdja_value *v = fdja_parse(ss);

  if (v == NULL) goto _prev;

  size_t la = strlen(point);
  fdja_value *pt = fdja_l(v, "point");
  if (strncmp(point, fdja_src(pt), la) != 0) goto _prev;

  if (nid)
  {
    n = fdja_lsd(v, "nid", "0");
    if (n && strcmp(n, nid) != 0) goto _prev;
  }

  r = v;

_prev: // because we read from the end line to the first line

  free(n);

  if (r) return r;

  fdja_free(v);
  if (v == NULL) free(ss);

  if (lf == NULL) return NULL;

  *lf = 0;
  return scan(s, point, nid);
}

fdja_value *hlp_wait(char *exid, char *point, char *nid, int maxsec)
{
  char *fep = flon_exid_path(exid);
  //if (nid == NULL) nid = "0";

  fdja_value *r = NULL;

  for (size_t i = 0; i < maxsec * 50; ++i) // approx...
  {
    flu_msleep(20);

    char *path = flu_sprintf("var/archive/%s/msg.log", fep);
    if (flu_fstat(path) != 'f') { free(path); path = NULL; }
    if ( ! path) path = flu_sprintf("var/run/%s/msg.log", fep);
    if (flu_fstat(path) != 'f') { free(path); continue; }

    char *s = flu_readall(path);
    //printf("hlp_wait() -------------------------------------- -\n");
    r = scan(s, point, nid);
    free(s);
    free(path);

    if (r) break;
  }

  free(fep);

  return r;
}

static char *find_path(char *exid)
{
  char *fep = flon_exid_path(exid);

  char *path = flu_sprintf("var/archive/%s", fep);
  if (flu_fstat(path) == 'd') { free(fep); return path; }

  free(path);
  path = flu_sprintf("var/run/%s", fep);
  if (flu_fstat(path) == 'd') { free(fep); return path; }

  return NULL;
}

fdja_value *hlp_read_run_json(char *exid)
{
  char *fep = flon_exid_path(exid);
  fdja_value *v = fdja_parse_f("var/run/%s/run.json", fep);
  free(fep);

  return v;
}

fdja_value *hlp_read_archive_run_json(char *exid)
{
  char *fep = flon_exid_path(exid);
  fdja_value *v = fdja_parse_f("var/archive/%s/run.json", fep);
  free(fep);

  return v;
}

fdja_value *hlp_read_node(char *exid, char *nid)
{
  fdja_value *v = hlp_read_run_json(exid);
  if (v == NULL) return NULL;

  fdja_value *r = fdja_lc(v, "nodes.%s", nid);

  fdja_free(v);

  return r;
}

fdja_value *hlp_read_timer(char *exid, char *nid, char *type, char *ts)
{
  char *fep = flon_exid_path(exid);

  fdja_value *v = fdja_parse_f(
    "var/spool/tdis/%s/%s-%s-%s-%s.json", fep, type, ts, exid, nid);

  free(fep);

  return v;
}

fdja_value *hlp_read_tsk_log(char *exid)
{
  char *fep = flon_exid_path(exid);

  char *fpath = flu_sprintf("var/run/%s/tsk.log", fep);
  if (flu_fstat(fpath) != 'f')
  {
    free(fpath); fpath = flu_sprintf("var/archive/%s/tsk.log", fep);
  }

  fdja_value *r = fdja_array_malloc();

  FILE *f = fopen(fpath, "r"); if (f == NULL) return r;

  char *line = NULL;
  size_t len = 0;
  fdja_value *v = NULL;

  while (getline(&line, &len, f) != -1)
  {
    char *br = strchr(line, '{');
    char *time = strndup(line, br - line - 1);
    char *s = strdup(br);
    v = fdja_parse(s);
    fdja_set(v, "_time", fdja_sym(time));
    fdja_push(r, v);
  }

  free(line);
  fclose(f);
  free(fep);
  free(fpath);

  return r;
}

double hlp_determine_delta(char *exid)
{
  double r = -1.0;

  char *fep = flon_exid_path(exid);
  char *line = flu_pline("grep delta var/archive/%s/exe.log", fep);
  char *delta = strrchr(line, ' ');

  if (delta == NULL) goto _over;

  r = flu_parse_d(delta + 1);

  if (errno != 0) r = -1.0;

_over:

  free(fep);
  free(line);

  //printf("delta: %f\n", r);

  return r;
}

void hlp_cat_tsk_log(char *exid)
{
  char *fep = flon_exid_path(exid);
  flu_system("find var/log/%s -name \"*.log\" | xargs tail -n +1", fep);
  free(fep);
}

char *hlp_last_msg(char *exid)
{
  char *path = find_path(exid);
  char *s = flu_pline("tail -1 %s/msg.log", path);
  free(path);

  return s;
}

void hlp_reset_tst(char flavour)
{
  if (flavour == 't')
    flu_system("make -C .. clean-tst-time 2>&1 > /dev/null");
  else
    flu_system("make -C .. ctst 2>&1 > /dev/null");
}

size_t hlp_count_jsons(const char *path, ...)
{
  va_list ap; va_start(ap, path); char *p = flu_svprintf(path, ap); va_end(ap);

  flu_list *l = flon_find_json(p);
  size_t r = l->size;
  flu_list_free(l);

  free(p);

  return r;
}

