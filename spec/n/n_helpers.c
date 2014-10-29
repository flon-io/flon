
//
// helpers for n_ specs
//
// Fri Oct 24 12:53:40 JST 2014
//

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

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
      // seems necessary, else the ev io watch doesn't seem to get in
  }
}

void dispatcher_stop()
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

  //sleep(1);
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

  int i = fdja_to_json_f(v, "var/spool/dis/exe_%s.json", exid);
  if (i != 1)
  {
    nlog("failed to write launch file at var/spool/dis/exe_%s.json", exid);
    return NULL;
  }
  //flu_system("touch var/spool/dis/");

  fdja_free(v);

  // wait for result

  fdja_value *r = NULL;

  for (size_t i = 0; i < 3 * 10; ++i) // max 3 seconds
  {
    flu_msleep(100);

    printf("."); fflush(stdout);

    //printf("fep: %s\n", fep);
    //flu_system("cat var/run/%s/exe.log", fep);
    //flu_system("cat var/run/%s/msgs.log", fep);

    if (flu_fstat("var/archive/%s/msgs.log", fep) != 'f') continue;

    printf("\n");

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

  free(fep);

  // return result

  return r;
}

void dump_execution_state(const char *exid)
{
  char *fep = flon_exid_path(exid);

  char *path = flu_sprintf("var/run/%s", fep);
  if (flu_fstat(path) == '\0')
  {
    free(path);
    path = flu_sprintf("var/archive/%s", fep);
  }

  nlog("");
  nlog("--8<-- execution: %s", exid);

  printf("\n# %s/\n", path);

  puts("\n## execution log\n#");
  printf("[0;32m"); fflush(stdout);
  flu_system("cat %s/exe.log", path);
  printf("[0;0m");

  puts("\n## msgs log\n#");
  char *fpath = flu_sprintf("%s/msgs.log", path);
  FILE *f = fopen(fpath, "r");
  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath);
    perror("reason:");
  }
  else
  {
    char *line = NULL;
    size_t len = 0;
    fdja_value *v = NULL;

    while (getline(&line, &len, f) != -1)
    {
      v = fdja_parse(line);
      if (v) { puts(fdja_todc(v)); fdja_free(v); }
      else { puts(line); }
    }
    fclose(f);
  }
  free(fpath);

  puts("\n## run.json\n#");
  fdja_value *v = fdja_parse_f("%s/run.json", path);
  if (v) { puts(fdja_todc(v)); fdja_free(v); }
  else { flu_system("cat %s/run.json", path); }

  puts("\n## processed\n#");
  printf("[0;32m"); fflush(stdout);
  flu_system("ls -lh %s/processed", path);
  printf("[0;0m");

  puts("");

  nlog("-->8--");
  nlog("");
}

