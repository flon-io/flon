
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

// https://github.com/flon-io/flutil

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <locale.h>

#include "flutil.h"
#include "flutim.h"


// flutim.c


struct timespec *flu_now()
{
  struct timespec *r = calloc(1, sizeof(struct timespec));

  int i = clock_gettime(CLOCK_REALTIME, r);

  if (i != 0) { free(r); return NULL; }
  return r;
}

long long flu_gets(char level)
{
  struct timespec ts;
  int r = clock_gettime(CLOCK_REALTIME, &ts);

  if (r != 0) return 0;

  if (level == 'n') return ts.tv_sec * 1000000000 + ts.tv_nsec;
  if (level == 'u') return ts.tv_sec * 1000000 + ts.tv_nsec / 1000;
  if (level == 'm') return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
  return ts.tv_sec; // else, 's'
}

long long flu_msleep(long long milliseconds)
{
  struct timespec treq;
  treq.tv_sec = milliseconds / 1000;
  treq.tv_nsec = (milliseconds * 1000000) % 1000000000;

  struct timespec trem;
  trem.tv_sec = 0;
  trem.tv_nsec = 0;

  nanosleep(&treq, &trem);

  return trem.tv_sec * 1000 + trem.tv_nsec / 1000000;
}

long long flu_do_msleep(long long milliseconds)
{
  long long start = flu_gets('m');

  struct timespec treq;
  treq.tv_sec = milliseconds / 1000;
  treq.tv_nsec = (milliseconds * 1000000) % 1000000000;

  struct timespec trem;
  trem.tv_sec = 0;
  trem.tv_nsec = 0;

  while (1)
  {
    nanosleep(&treq, &trem);

    //printf("trem s: %llu, ns: %llu\n", trem.tv_sec, trem.tv_nsec);
    if (trem.tv_sec == 0 && trem.tv_nsec == 0) break;

    treq.tv_sec = trem.tv_sec; treq.tv_nsec = trem.tv_nsec;
    trem.tv_sec = 0; trem.tv_nsec = 0;
  }

  return flu_gets('m') - start;
}

char *flu_tstamp(struct timespec *ts, int utc, char format)
{
  if (ts == NULL)
  {
    struct timespec tss;
    int i = clock_gettime(CLOCK_REALTIME, &tss);
    if (i != 0) return NULL;
    ts = &tss;
  }

  if (
    format == 'z' || format == 'Z' || format == 'r' || format == 'g'
  ) utc = 1;

  struct tm *tm = utc ? gmtime(&ts->tv_sec) : localtime(&ts->tv_sec);

  if (tm == NULL) return NULL;

  char *r = calloc(32, sizeof(char));

  if (format == 'z' || format == 'Z')
  {
    strftime(r, 32, "%Y-%m-%dT%H:%M:%SZ", tm);
    return r;
  }

  if (format == 'r' || format == 'g' || format == '2')
  {
    char *loc = strdup(setlocale(LC_TIME, NULL)); setlocale(LC_TIME, "en_US");
    //
    if (format == '2') strftime(r, 32, "%a, %d %b %Y %T %z", tm);
    else strftime(r, 32, "%a, %d %b %Y %T UTC", tm);
    //
    setlocale(LC_TIME, loc); free(loc);

    if (format == 'g') strcpy(r + strlen(r) - 3, "GMT");
    return r;
  }

  strftime(r, 32, "%Y%m%d.%H%M%S", tm);
  size_t l = strlen(r);

  if (format == 'h') { *(r + l - 2) = '\0'; return r; }
  if (format == 's') { return r; }

  *(r + l) = '.';

  sprintf(r + l + 1, "%09li", ts->tv_nsec);

  size_t off = 9;
  //
  if (format == 'u') off = 6;
  else if (format == 'm') off = 3;
  //
  *(r + l + 1 + off) = '\0';

  return r;
}

static int ptime(char *s, struct tm *tm)
{
  char *d = strchr(s, 'T');
  if (d == NULL) d = strchr(s, '.');
  if (d == NULL) return 1;

  if (*d == 'T')
  {
    tm->tm_hour = flu_stoll(d + 1, 2, 10);
    tm->tm_min = flu_stoll(d + 4, 2, 10);
    tm->tm_sec = flu_stoll(d + 7, 2, 10);
    d = strchr(s, '-'); tm->tm_year = flu_stoll(s + 0, d - s, 10) - 1900;
    tm->tm_mon = flu_stoll(d + 1, 2, 10) - 1;
    d = strchr(d + 1, '-'); tm->tm_mday = flu_stoll(d + 1, 2, 10);
  }
  else //if (*d == '.')
  {
    tm->tm_hour = flu_stoll(d + 1, 2, 10);
    tm->tm_min = flu_stoll(d + 3, 2, 10);
    tm->tm_sec = flu_stoll(d + 5, 2, 10);
    tm->tm_mday = flu_stoll(d - 2, 2, 10);
    tm->tm_mon = flu_stoll(d - 4, 2, 10) - 1;
    tm->tm_year = flu_stoll(s, d - 4 - s, 10) - 1900;
  }

  tm->tm_wday = 0; tm->tm_yday = 0; tm->tm_isdst = 0;

  return 0; // success
}

struct timespec *flu_parse_tstamp(char *s, int utc)
{
  struct tm tm = {};
  char *subseconds = NULL;

  if (strchr(s, '-'))
  {
    utc = 1;
  }
  else
  {
    char *a = strchr(s, '.');
    char *b = strrchr(s, '.');

    if (a == NULL) return NULL;
    if (a != b) subseconds = b + 1;
  }

  int r = ptime(s, &tm);
  if (r != 0) return NULL;

  //printf(
  //  "tm: sec:%i, min:%i, hour:%i, mday:%i, mon:%i, year:%i,"
  //  " wday:%i, yday:%i, isdst:%i\n",
  //  tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon, tm.tm_year,
  //  tm.tm_wday, tm.tm_yday, tm.tm_isdst);
  ////
  //// disrupt...
  //tm.tm_wday = -1;
  //tm.tm_yday = -1;

  char *tz = NULL;
  if (utc) { tz = getenv("TZ"); setenv("TZ", "UTC", 1); tzset(); }
    //
  time_t t = mktime(&tm);
    //
  if (utc) { if ( ! tz) unsetenv("TZ"); else setenv("TZ", tz, 1); tzset(); }
    //
    // /!\ not thread-safe /!\.

  struct timespec *ts = calloc(1, sizeof(struct timespec));
  ts->tv_sec = t;
  ts->tv_nsec = 0;

  if (subseconds)
  {
    size_t st = strlen(subseconds);

    ts->tv_nsec = strtoll(subseconds, NULL, 10);
    if (st == 3) ts->tv_nsec = ts->tv_nsec * 1000 * 1000;
    else if (st == 6) ts->tv_nsec = ts->tv_nsec * 1000;
  }

  return ts;
}

struct timespec *flu_tdiff(struct timespec *t1, struct timespec *t0)
{
  short t1null = 0;
  struct timespec *t2 = calloc(1, sizeof(struct timespec));

  if (t1 == NULL) { t1null = 1; t1 = flu_now(); }

  t2->tv_sec = t1->tv_sec - t0->tv_sec;
  t2->tv_nsec = t1->tv_nsec - t0->tv_nsec;

  if (t2->tv_nsec < 0) { --t2->tv_sec; t2->tv_nsec += 1000000000; }

  if (t1null) free(t1);

  return t2;
}

char *flu_ts_to_s(struct timespec *ts, char format)
{
  char *r = calloc(10 + 1 + 9 + 1, sizeof(char));

  snprintf(r, 20, "%lis%09li", ts->tv_sec, ts->tv_nsec);

  ssize_t off = -1;
  if (format == 's') off = 0;
  else if (format == 'm') off = 3;
  else if (format == 'u') off = 6;
  //
  if (off > -1) *(strchr(r, 's') + 1 + off) = '\0';

  return r;
}

struct timespec *flu_parse_ts(const char *s)
{
  struct timespec *ts = calloc(1, sizeof(struct timespec));
  long long sign = 1;
  char prev = 0;

  size_t l = strlen(s); if (l < 9) l = 9;

  char *ss = calloc(l + 1, sizeof(char));
  for (size_t k = 0; k < l; ) { ss[k++] = '0'; } ss[l] = '\0';

  for (size_t i = 0, j = 0; ; ++i)
  {
    char c = s[i];

    if (c == '.')
    {
      prev = '.';
    }
    else if (c >= '0' && c <= '9')
    {
      ss[j++] = c; ss[j] = '\0';
    }
    else if (c == '\0' || strchr("-+yMwdhms", c))
    {
      short sub = 0;
      long long mod = 1; // s and \0
      //
      if (c == 'm') mod = 60;
      else if (c == 'h') mod = 60 * 60;
      else if (c == 'd') mod = 24 * 60 * 60;
      else if (c == 'w') mod = 7 * 24 * 60 * 60;
      else if (c == 'M') mod = 30 * 24 * 60 * 60;
      else if (c == 'y') mod = 365 * 24 * 60 * 60;

      if (prev == '.' || (c == '\0' && prev == 's')) sub = 1;

      if (sub)
      {
        ss[j] = '0'; ss[9] = '\0';
        ts->tv_nsec += sign * strtoll(ss, NULL, 10);

        while (ts->tv_sec > 0 && ts->tv_nsec < 0)
        {
          ts->tv_sec--; ts->tv_nsec += 1000000000;
        }
      }
      else
      {
        ts->tv_sec += sign * strtoll(ss, NULL, 10) * mod;
      }

      j = 0; for (size_t k = 0; k < l; ) ss[k++] = '0';

      if (c == '\0') break;

      if (c == '+') sign = 1; else if (c == '-') sign = -1;

      prev = c;
    }
    else
    {
      free(ss); free(ts); return NULL;
    }
  }

  free(ss);

  return ts;
}

long long flu_parse_t(const char *s)
{
  struct timespec *ts = flu_parse_ts(s);
  if (ts == NULL) { errno = EINVAL; return 0; }

  long long r = ts->tv_sec;
  free(ts);

  return r;
}

