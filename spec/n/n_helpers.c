
//
// helpers for n_ specs
//
// Fri Oct 24 12:53:40 JST 2014
//

#define _POSIX_C_SOURCE 200809L

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "flutil.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "n_helpers.h"


void nlog(char *format, ...)
{
  va_list ap; va_start(ap, format);
  char *fo = flu_sprintf("[1;30m.:n:. %s[0;0m\n", format);
  vprintf(fo, ap);
  free(fo);
  va_end(ap);
}

char *dispatcher_path = "../tst/bin/flon-dispatcher";
pid_t dispatcher_pid = -1;

void dispatcher_start()
{
  if (dispatcher_pid > 0) return;

  //printf("[1;30m\n");
  //system("make -C ../tst webapp");
  //printf("[0;0m\n");

  dispatcher_pid = fork();

  if (dispatcher_pid == 0)
  {
    char *v = getenv("VALDIS");

    if (v != NULL && (strcmp(v, "1") == 0 || strcmp(v, "true") == 0))
    {
      char *env[] = { "FGAJ_HOST=l", "FGAJ_LEVEL=10", NULL };
      execle("/usr/bin/valgrind", "", dispatcher_path, "", NULL, env);
    }
    else
    {
      //char *env[] = { "FGAJ_HOST=g", "FGAJ_LEVEL=10", NULL };
      char *env[] = { "FGAJ_HOST=l", NULL };
      execle(dispatcher_path, "", NULL, env);
    }

    perror("execle failed"); exit(1);
  }
  else
  {
    nlog("dispatcher started pid: %i...", dispatcher_pid);

    sleep(1);
  }
}

void dispatcher_stop()
{
  if (dispatcher_pid < 1) return;

  nlog("stopping dispatcher pid: %i...", dispatcher_pid);

  kill(dispatcher_pid, SIGTERM);

  sleep(1);
}

fdja_value *launch(char *exid, char *flow, char *payload)
{
  char *fep = flon_exid_path(exid);

  // launch

  fdja_value *fl = fdja_parse_radial(strdup(flow));
  if (fl == NULL) { nlog("couldn't parse radial flow..."); return NULL; }

  fdja_value *pl = fdja_v(payload);
  if (pl == NULL) { nlog("couldn't parse payload..."); return NULL; }

  fdja_value *v = fdja_v("{ exid: %s }", exid);

  fdja_set(v, "execute", fl);
  fdja_set(v, "payload", pl);

  fdja_to_json_f(v, "var/spool/dis/exe_%s.json", exid);

  fdja_free(v);

  // wait for result

  struct timespec treq;
  treq.tv_sec = 0;
  treq.tv_nsec = 1000000;

  struct timespec trem;

  fdja_value *r = NULL;

  while (1)
  {
    nanosleep(&treq, &trem);

    printf(".");

    if (flu_fstat("var/archive/%s/msgs.log", fep) != 'f') continue;

    printf("\n");

    //system("tree var/");
    ////
    //char *s = NULL;
    ////
    //nlog("--8<-- exe.log");
    //s = flu_readall("var/archive/%s/exe.log", fep);
    //if (s) puts(s);
    //free(s);
    //nlog("-->8--");
    ////
    //nlog("--8<-- run.json");
    //s = flu_readall("var/archive/%s/run.json", fep);
    //if (s) puts(s);
    //free(s);
    //nlog("-->8--");
    ////
    //nlog("--8<-- msgs.log");
    //s = flu_readall("var/archive/%s/msgs.log", fep);
    //if (s) puts(s);
    //free(s);
    //nlog("-->8--");

    char *s = flu_readall("var/archive/%s/msgs.log", fep);
    *(strrchr(s, '}') + 1) = '\0';
    char *lf = strrchr(s, '\n');
    char *ss = strdup(lf ? lf + 1 : s);
    //puts(ss);
    r = fdja_parse(ss);
    free(s);

    break;
  }

  // return result

  return r;
}

