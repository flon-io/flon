
//
// Copyright (c) 2013-2016, John Mettraux, jmettraux+flon@gmail.com
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

#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>

#include "flutil.h"
#include "flutim.h"
#include "djan.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"


static fdja_value *flon_configuration = NULL;

int flon_configure(char *root)
{
  fdja_value *v = fdja_parse_obj_f("%s/etc/flon.json", root);

  if (v == NULL) return 1;

  fdja_set(v, "_root", fdja_s(root));

  flon_configure_j(v);

  return 0;
}

void flon_configure_j(fdja_value *obj)
{
  if (flon_configuration) fdja_value_free(flon_configuration);

  flon_configuration = obj;
}

fdja_value *flon_conf(const char *key)
{
  return fdja_lookup(flon_configuration, key);
}

int flon_conf_boolean(const char *key, int def)
{
  return fdja_lookup_bool(flon_configuration, key, def);
}

long long flon_conf_int(const char *key, long long def)
{
  return fdja_lookup_int(flon_configuration, key, def);
}

char *flon_conf_string(const char *key, char *def)
{
  return fdja_lsd(flon_configuration, key, def);
}

char *flon_conf_path(const char *key, char *def)
{
  char *v = flon_conf_string(key, def);

  if (v == NULL) return NULL;

  char *r = flu_canopath(v);

  if (v != def) free(v);

  return r;
}

int flon_conf_is(const char *key, const char *val)
{
  fdja_value *v = flon_conf(key);
  if (v == NULL) { return (val == NULL); }
  char *s = fdja_to_string(v);

  int r = (strcmp(s, val) == 0);

  free(s);

  return r;
}

char *flon_conf_uid()
{
  char *gid = flon_conf_string("unit.gid", NULL);
  char *uid = flon_conf_string("unit.id", NULL);

  if (gid == NULL && uid == NULL) return strdup("u0");
  if (gid == NULL) return uid;

  char *r = flu_sprintf("%s.%s", gid, uid);

  free(gid); free(uid);

  return r;
}

static void setup_logging(fdja_value *v)
{
  if (v == NULL) return;

  char *l = fdja_lookup_string(v, "level", NULL);
  if (l)
  {
    fgaj_conf_get()->level = fgaj_parse_level(l);
    free(l);
  }

  char *h = fdja_lookup_string(v, "host", NULL);
  if (h)
  {
    if (fgaj_conf_get()->host) free(fgaj_conf_get()->host);
    fgaj_conf_get()->host = h;
  }

  char *u = fdja_lookup_string(v, "utc", NULL);
  if (u)
  {
    fgaj_conf_get()->utc = (tolower(*u) == 't' || *u == '1');
    free(u);
  }
}

void flon_setup_logging(const char *context)
{
  setup_logging(flon_conf("all.log"));
  setup_logging(fdja_lookup(flon_configuration, "%s.log", context));

  fgaj_read_env();

  // FLONLOG=hostname,dis10,exe30,inv10
  //   or
  // FLONLOG=hostname,all10
  // FLONLOG=hostname,allDEBUG

  //char *env = getenv("FLONLOG");

  // TODO use putenv() to set FGAJ_HOST and _LEVEL
  //      NO, use fgaj_cong_get()->x = y;

  fgaj_conf_get()->out = stderr;
}

fdja_value *flon_try_parse(char mode, const char *path, ...)
{
  errno = 0;

  va_list ap; va_start(ap, path);
  char *fname = flu_svprintf(path, ap);
  va_end(ap);

  fdja_value *r = NULL;

  FILE *f = fopen(fname, "r");

  if (f == NULL) goto _over;
  if (flock(fileno(f), LOCK_NB | LOCK_EX) != 0) goto _over;

  if (mode == 'o') r = fdja_fparse_obj(f);
  else if (mode == 'r') r = fdja_fparse_radial(f, fname);
  else r = fdja_fparse(f);

_over:

  //perror("flon_try_parse()");

  if (f) fclose(f);
  free(fname);

  return r;
}

int flon_lock_write(fdja_value *v, const char *path, ...)
{
  int r = 0; // failed, for now

  va_list ap; va_start(ap, path);
  char *fname = flu_svprintf(path, ap);
  va_end(ap);

  FILE *f = fopen(fname, "w");

  if (f == NULL) goto _over;
  if (flock(fileno(f), LOCK_NB | LOCK_EX) != 0) goto _over;

  fdja_to_j(f, v, 0);

  r = 1; // success

_over:

  fclose(f);
  free(fname);

  return r;
}

static int move_to(const char *fname, const char *todir, ...)
{
  va_list ap; va_start(ap, todir);
  char *td = flu_svprintf(todir, ap);
  va_end(ap);

  int r = 0; // success so far

  char *fn = strdup(strrchr(fname, '/') + 1);
  char *dot = strrchr(fn, '.'); if (dot) *dot = 0;
  char *suf = dot ? dot + 1 : ".json";

  char *t = NULL;
  char *extra = NULL;

  for (size_t i = 0; ; ++i)
  {
    if ((i + 1) % 100 == 0) fgaj_w("cycling like mad: %s", t);

    free(extra);
    extra = flu_sprintf("__%zu", i); if (i == 0) *extra = 0;
    free(t);
    t = flu_path("%s/%s%s.%s", td, fn, extra, suf);

    if (flu_fstat(t) == 0)
    {
      if (flu_move(fname, t) == 0)
      {
        fgaj_d("moved %s to %s", fname, t);
      }
      else
      {
        fgaj_r("failed to move %s to %s", fname, t);
        r = 1;
        // MAYBE move to rejected???
      }
      break;
    }
  }

  free(t);
  free(extra);

  free(td);
  free(fn);

  return r;
}

int flon_move_to_rejected(const char *path, ...)
{
  if (path == NULL) { fgaj_e("path is NULL"); return 1; }

  va_list ap; va_start(ap, path);
  char *pa = flu_svprintf(path, ap);
  char *re = flu_svprintf(va_arg(ap, char *), ap);
  va_end(ap);

  int r = 0; // success so far

  FILE *f = fopen(pa, "a");
  if (f)
  {
    char *ts = flu_tstamp(NULL, 1, 'u');
    fprintf(f, "\n# reason: %s (%s Z)\n", re, ts);
    fclose(f);
    free(ts);
  }
  else
  {
    fgaj_r("couldn't write reason '%s' to %s", re, pa);
  }

  r = move_to(pa, "var/spool/rejected/");

  if (r == 0)
  {
    fgaj_i("rejected %s: %s", pa, re);
  }

  free(pa);
  free(re);

  return r;
}

int flon_move_to_processed(const char *path, ...)
{
  va_list ap; va_start(ap, path); char *p = flu_svprintf(path, ap); va_end(ap);

  int r = 0; // success

  char *fn = strdup(strrchr(p, '/') + 1);
  char *exid = flon_get_exid(fn);
  char *fep = flon_exid_path(exid);

  char *d = (flu_fstat("var/archive/%s", fep) == 'd') ?  "archive" : "run";

  if (flu_fstat("var/%s/%s/processed", d, fep) == 0)
  {
    if (flu_mkdir_p("var/%s/%s/processed", d, fep, 0755) != 0)
    {
      fgaj_r("couldn't mkdir -p var/%s/%s/processed", d, fep);
      r = 1;
      goto _over;
    }
  }

  r = move_to(p, "var/%s/%s/processed/", d, fep);

_over:

  free(p);
  free(fn);
  free(exid);
  free(fep);

  return r;
}

