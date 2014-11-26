
//
// helpers for n/ specs
//
// Fri Oct 24 12:53:40 JST 2014
//

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
//#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "flutil.h"
#include "flutim.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "n_helpers.h"


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
    //if (logtoterm())
    //{
    nlog("stopped dispatcher pid: %i status: %i", dispatcher_pid, status);
    //}
  }
  //else if (logtoterm())
  else
  {
    nlog("stopped dispatcher pid: %i -> %s", dispatcher_pid, strerror(errno));
  }

  dispatcher_pid = -1;

  //sleep(1);
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
    char *v = getenv("FLONVAL");

    if (v && strstr(v, "dis"))
    {
      execl(
        "/usr/bin/valgrind", "v n_flon-dispatcher",
        "--leak-check=full", "-v", dispatcher_path, NULL);
    }
    else
    {
      if ( ! logtoterm())
      {
        freopen("var/log/dispatcher.log", "a", stdout);
        fflush(stdout);
        freopen("var/log/dispatcher.log", "a", stderr);
        fflush(stderr);
      }

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
    flu_do_msleep(210);
  }
}

void hlp_launch(char *exid, char *flow, char *payload)
{
  char *fep = flon_exid_path(exid);

  // launch

  fdja_value *fl = fdja_parse_radial(strdup(flow));
  if (fl == NULL) { nlog("couldn't parse radial flow..."); return; }

  fdja_value *pl = fdja_v(payload);
  if (pl == NULL) { nlog("couldn't parse payload..."); return; }

  fdja_value *v = fdja_v("{ exid: %s }", exid);

  fdja_psetv(v, "point", "execute");
  fdja_set(v, "tree", fl);
  fdja_set(v, "payload", pl);

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

static fdja_value *scan(char *s, char *action, char *nid)
{
  fdja_value *r = NULL;
  char *ss = NULL;
  char *n = NULL;

  size_t la = strlen(action);

  *(strrchr(s, '}') + 1) = '\0';
  char *lf = strrchr(s, '\n');
  ss = strdup(strchr(lf ? lf : s, '{'));

  //printf("scan() >>%s<\n", ss);

  fdja_value *v = fdja_parse(ss);

  if (v == NULL) goto _prev;

  fdja_value *point = fdja_l(v, "point");
  if (strncmp(action, fdja_src(point), la) != 0) goto _prev;

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
  return scan(s, action, nid);
}

fdja_value *hlp_wait(char *exid, char *action, char *nid, int maxsec)
{
  size_t la = strlen(action);

  char *fep = flon_exid_path(exid);
  if (nid == NULL) nid = "0";

  fdja_value *r = NULL;

  for (size_t i = 0; i < maxsec * 10; ++i) // approx...
  {
    flu_msleep(100);

    char *path = flu_sprintf("var/archive/%s/msgs.log", fep);
    if (flu_fstat(path) != 'f') { free(path); path = NULL; }
    if ( ! path) path = flu_sprintf("var/run/%s/msgs.log", fep);
    if (flu_fstat(path) != 'f') { free(path); continue; }

    char *s = flu_readall(path);
    //printf("hlp_wait() -------------------------------------- -\n");
    r = scan(s, action, nid);
    free(s);
    free(path);

    if (r) break;
  }

  free(fep);

  // return result

  return r;
}

fdja_value *hlp_read_run_json(char *exid)
{
  char *fep = flon_exid_path(exid);
  fdja_value *v = fdja_parse_f("var/run/%s/run.json", fep);
  free(fep);

  return v;
}

void hlp_cat_inv_log(char *exid)
{
  char *fep = flon_exid_path(exid);
  flu_system("find var/log/%s -name \"*.log\" | xargs tail -n +1", fep);
  free(fep);
}

