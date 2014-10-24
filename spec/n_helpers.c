
//
// helpers for n_ specs
//
// Fri Oct 24 12:53:40 JST 2014
//

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include "n_helpers.h"


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
    sleep(1);
  }
}

void dispatcher_stop()
{
  if (dispatcher_pid < 1) return;

  printf("stopping %i...\n", dispatcher_pid);
  kill(dispatcher_pid, SIGTERM);

  sleep(1);
}

