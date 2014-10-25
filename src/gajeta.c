
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

// https://github.com/flon-io/gajeta

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "flutil.h"
#include "gajeta.h"


//
// conf

fgaj_conf *fgaj__conf = NULL;

static char *fgaj_getenv(const char *k0, const char *k1)
{
  char *s = getenv(k0);
  if (s == NULL) s = getenv(k1);

  return s;
}

static void fgaj_init()
{
  fgaj__conf = calloc(1, sizeof(fgaj_conf));

  char *s = NULL;

  // color or not?

  s = fgaj_getenv("FLON_LOG_COLOR", "FGAJ_COLOR");

  fgaj__conf->color = s == NULL ? 't' : s[0];

  // utc or not?

  s = fgaj_getenv("FLON_LOG_UTC", "FGAJ_UTC");

  fgaj__conf->utc = (s != NULL && (s[0] == '1' || tolower(s[0]) == 't'));

  // determine host

  fgaj__conf->host = fgaj_getenv("FLON_LOG_HOST", "FGAJ_HOST");

  if (fgaj__conf->host == NULL)
  {
    char *h = calloc(16, sizeof(char));
    gethostname(h, 16);
    fgaj__conf->host = h;
  }

  // determine level

  fgaj__conf->level = 30;

  s = fgaj_getenv("FLON_LOG_LEVEL", "FGAJ_LEVEL");
  //
  if (s != NULL)
  {
    if (s[0] > '0' && s[1] < '9') fgaj__conf->level = atoi(s);
    else fgaj__conf->level = fgaj_normalize_level(s[0]);
  }
  //printf("level: %i\n", fgaj__level);

  // determine logger

  fgaj__conf->logger = fgaj_color_file_logger;
  fgaj__conf->out = NULL;

  // no need for params with fgaj_color_file_logger (for now)

  fgaj__conf->params = NULL;
}

fgaj_conf *fgaj_conf_get()
{
  if (fgaj__conf == NULL) fgaj_init();
  return fgaj__conf;
}

void fgaj_conf_reset()
{
  if (fgaj__conf == NULL) return;
  if (fgaj__conf->host) free(fgaj__conf->host);
  free(fgaj__conf);
  fgaj__conf = NULL;
}

//
// misc functions

char fgaj_normalize_level(char level)
{
  if (level >= 'A' && level <= 'Z') level = tolower(level);

  if (level == 't') return 10;
  if (level == 'd') return 20;
  if (level == 'i') return 30;
  if (level == 'w') return 40;
  if (level == 'e' || level == 'r') return 50;
  return level;
}

char *fgaj_level_to_string(char level)
{
  level = fgaj_normalize_level(level);

  if (level == 10) return "TRACE";
  if (level == 20) return "DEBUG";
  if (level == 30) return "INFO";
  if (level == 40) return "WARN";
  if (level == 50) return "ERROR";
  return flu_sprintf("%d", level);
}

void fgaj_level_string_free(char *s)
{
  if (s[0] >= '0' && s[0] <= '9') free(s);
}

//
// loggers


// PS1="\[\033[1;34m\][\$(date +%H%M)][\u@\h:\w]$\[\033[0m\] "
//
// Black       0;30     Dark Gray     1;30
// Blue        0;34     Light Blue    1;34
// Green       0;32     Light Green   1;32
// Cyan        0;36     Light Cyan    1;36
// Red         0;31     Light Red     1;31
// Purple      0;35     Light Purple  1;35
// Brown       0;33     Yellow        1;33
// Light Gray  0;37     White         1;37

static short fgaj_color(FILE *f)
{
  if (fgaj__conf->color == 'T') return 1;
  if (fgaj__conf->color == 'f') return 0;
  return isatty(fileno(f));
}

static char *fgaj_red(int c) { return c ? "[0;31m" : ""; }
static char *fgaj_green(int c) { return c ? "[0;32m" : ""; }
static char *fgaj_brown(int c) { return c ? "[0;33m" : ""; }
static char *fgaj_blue(int c) { return c ? "[0;34m" : ""; }
static char *fgaj_cyan(int c) { return c ? "[0;36m" : ""; }
static char *fgaj_white(int c) { return c ? "[0;37m" : ""; }
static char *fgaj_yellow(int c) { return c ? "[1;33m" : ""; }
static char *fgaj_clear(int c) { return c ? "[0;0m" : ""; }

char *fgaj_now()
{
  if (fgaj__conf == NULL) fgaj_init();

  struct timeval tv;
  struct tm *tm;

  gettimeofday(&tv, NULL);
  tm = fgaj__conf->utc ? gmtime(&tv.tv_sec) : localtime(&tv.tv_sec);

  char *s = calloc(33, sizeof(char));
  strftime(s, 33, "%F %T.000000 %z", tm);
  snprintf(s + 20, 7, "%06ld", tv.tv_usec);
  s[26] = ' ';

  return s;
}

static FILE *fgaj_determine_out_file(FILE *def)
{
  FILE *f = (FILE *)fgaj__conf->out;

  if (f) return f;
  if (f == NULL && def != NULL) return def;
  return stdout;
}

void fgaj_color_file_logger(char level, const char *pref, const char *msg)
{
  char *now = fgaj_now();

  FILE *f = fgaj_determine_out_file(stdout);

  int c = fgaj_color(f);

  char *lcolor = NULL;
  if (level >= 50) lcolor = fgaj_red(c);          // error
  else if (level >= 40) lcolor = fgaj_yellow(c);  // warn
  else if (level >= 30) lcolor = fgaj_white(c);   // info
  else if (level >= 20) lcolor = fgaj_cyan(c);    // debug
  else if (level >= 10) lcolor = fgaj_green(c);   // trace
  else lcolor = fgaj_blue(c);                     // ...

  char *lstr = fgaj_level_to_string(level);

  fprintf(
    f,
    "%s%s %s%s %s%d/%d %s%5s %s%s %s%s%s\n",
    fgaj_brown(c), now,
    fgaj_white(c), fgaj__conf->host,
    fgaj_brown(c), getppid(), getpid(),
    lcolor, lstr,
    fgaj_green(c), pref,
    fgaj_white(c), msg,
    fgaj_clear(c)
  );

  free(now);
  fgaj_level_string_free(lstr);
}

void fgaj_string_logger(char level, const char *pref, const char *msg)
{
  char *l = fgaj_level_to_string(level);
  char *s = flu_sprintf("*** %s %s %s", l, pref, msg);
  fgaj_level_string_free(l);
  fgaj__conf->out = s;
}

void fgaj_grey_logger(char level, const char *pref, const char *msg)
{
  FILE *f = fgaj_determine_out_file(stdout);

  char *lstr = fgaj_level_to_string(level);

  char *cgrey = "[1;30m";
  char *cclear = "[0;0m";
  if (isatty(fileno(f)) != 1) { cgrey = ""; cclear = ""; }

  int indent = 10;
  char *pid = "";
  //
  if (fgaj__conf->params)
  {
    indent = strtol(fgaj__conf->params, NULL, 10);
    if (indent < 0) indent = 0;

    if (strchr(fgaj__conf->params, 'p'))
    {
      pid = flu_sprintf(" %i", getpid());
    }
  }

  fprintf(
    f,
    "%s%*s%s %-*s %s%s\n",
    cgrey, indent + 6, lstr, pid, 0, pref, msg, cclear);

  if (*pid != 0) free(pid);
  fgaj_level_string_free(lstr);
}


//
// logging functions

static void fgaj_do_log(
  char level,
  const char *file, int line, const char *func,
  const char *format, va_list ap, short err)
{
  if (fgaj__conf == NULL) fgaj_init();

  if (fgaj__conf->logger == NULL) return;

  level = fgaj_normalize_level(level);
  if (level < fgaj__conf->level && level <= 50) return;

  flu_sbuffer *b = NULL;

  b = flu_sbuffer_malloc();
  flu_sbputs(b, file);
  if (line > -1)
  {
    flu_sbprintf(b, ":%d", line);
    if (func != NULL) flu_sbprintf(b, ":%s", func);
  }
  //
  char *subject = flu_sbuffer_to_string(b);

  b = flu_sbuffer_malloc();
  flu_sbvprintf(b, format, ap);
  if (err) flu_sbprintf(b, ": %s", strerror(errno));
  //
  char *msg = flu_sbuffer_to_string(b);

  fgaj__conf->logger(level, subject, msg);

  free(subject);
  free(msg);
}

void fgaj_log(
  char level,
  const char *file, int line, const char *func,
  const char *format, ...)
{
  va_list ap; va_start(ap, format);
  fgaj_do_log(level, file, line, func, format, ap, tolower(level) == 'r');
  va_end(ap);
}

