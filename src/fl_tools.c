
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

#include "flutil.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"


  // Black       0;30     Dark Gray     1;30
  // Blue        0;34     Light Blue    1;34
  // Green       0;32     Light Green   1;32
  // Cyan        0;36     Light Cyan    1;36
  // Red         0;31     Light Red     1;31
  // Purple      0;35     Light Purple  1;35
  // Brown       0;33     Yellow        1;33
  // Light Gray  0;37     White         1;37
  //
static char *cclear = "[0;0m";
static char *cred = "[0;31m";
static char *cdred = "[1;31m";
//static char *cblue = "[0;34m";
static char *cdblue = "[1;34m";
static char *cgreen = "[0;32m";
static char *cdgrey = "[1;30m";
static char *cbrown = "[0;33m";
//static char *cyellow = "[1;33m";

static ssize_t count_lines(const char *path)
{
  FILE *f = fopen(path, "r");
  if (f == NULL) return -1;

  size_t r = 0;

  char *line = NULL;
  size_t len = 0;

  while (getline(&line, &len, f) != -1) ++r;

  free(line);
  fclose(f);

  return r;
}

static size_t lookup_anid(char **anids, const char *nid)
{
  size_t i = 0; while (1)
  {
    if (anids[i] == NULL) break;
    if (strcmp(anids[i], nid) == 0) return i;
    i++;
  }

  anids[i] = strdup(nid);
  return i;
}

static void print_msgs_timeline(const char *fpath)
{
  FILE *f = fopen(fpath, "r");

  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath);
    perror("reason:");
    return;
  }

  char *line = NULL;
  size_t len = 0;
  fdja_value *v = NULL;

  while (getline(&line, &len, f) != -1)
  {
    //printf("%.32s ", line);
    printf("%.26s ", line);
    char *br = strchr(line, '{');
    v = fdja_parse(br); if (v) v->sowner = 0;
    char p = v ? fdja_lk(v, "point") : '?';
    if (p == 'e' || p == 'f') // execute or failed
      flu_putf(fdja_todc(v));
    else
      printf(br);
    fdja_free(v);
  }
  free(line);
  fclose(f);
}

static void print_msgs_xmastree(const char *fpath)
{
  FILE *f = fopen(fpath, "r");

  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath);
    perror("reason:");
    return;
  }

  ssize_t msg_count = count_lines(fpath);

  char **anids = calloc(msg_count + 1, sizeof(char *));

  char *line = NULL;
  size_t len = 0;
  fdja_value *v = NULL;
  char *prevpl = NULL;

  while (getline(&line, &len, f) != -1)
  {
    printf("%s%.26s%s ", cdgrey, line, cclear);
    char *br = strchr(line, '{');
    v = fdja_parse(br);
    char *nid = v ? fdja_ls(v, "nid", NULL) : NULL;
    if (v == NULL)
    {
      printf(br);
    }
    else
    {
      v->sowner = 0;

      fdja_value *t = fdja_l(v, "tree");

      if (t)
      {
        long long line = fdja_li(t, "2", (long long)0);
        size_t anid = nid ? lookup_anid(anids, nid) : 0;
        printf("%s%3lli %s%3zx%s ", cbrown, line, cgreen, anid, cclear);
      }
      else
      {
        printf("        ");
      }

      int depth = nid ? flon_nid_depth(nid) : 0;
      printf("%*s", 2 * depth, "");

      char *point = fdja_ls(v, "point", NULL);
      char *color = *point == 'f' ? cdred : cclear;
      printf("%s%.2s%s ", color, point, cclear);
      free(point);

      if (nid)
      {
        //size_t anid = lookup_anid(anids, nid);
        //printf("%s%s %s%zx%s ", cdgrey, nid, cgreen, anid, cclear);
        printf("%s%s%s ", cdgrey, nid, cclear);
      }
      fdja_value *from = fdja_l(v, "from"); if (from)
      {
        char *f = fdja_to_string(from);
        size_t af = lookup_anid(anids, f);
        char *color = flon_is_plain_receive(v) ? cdgrey : cred;
        printf("f:%s%s%s:%s%zx%s ", color, f, cclear, cgreen, af, cclear);
        free(f);
      }

      if (t)
      {
        char *inst = fdja_ls(t, "0", NULL);
        char *atts = NULL;
        if (fdja_lz(t, "1") > 0)
        {
          atts = fdja_ld(t, "1");
          atts[0] = ' ';
          atts[strlen(atts) - 2] = 0;
        }
        printf(
          "%s%s%s%s ",
          cdblue, inst, atts ? atts + 1 : "", cclear);
        free(inst);
        free(atts);
      }

      if (fdja_l(v, "payload"))
      {
        char *payload = fdja_lj(v, "payload");
        if (prevpl == NULL || strcmp(prevpl, payload) != 0)
        {
          printf("%s%s%s ", cbrown, payload, cclear);
          free(prevpl); prevpl = payload;
        }
        else
        {
          free(payload);
        }
      }
      else
      {
        //free(prevpl); prevpl = NULL;
        printf("%s(nopl)%s ", cbrown, cclear);
      }

      printf("\n");
    }
    free(nid);
    fdja_free(v);
  }
  free(line);
  free(prevpl);
  fclose(f);

  size_t count = 0; for (; anids[count]; ++count) {};
  //
  printf(
    "%sseen %s%zu%s nodes%s\n",
    cdgrey, cgreen, count, cdgrey, cclear);

  for (size_t i = 0; anids[i]; ++i) free(anids[i]);
  free(anids);
}

static void print_msg_child(size_t indent, fdja_value *tr)
{
  for (fdja_value *ct = fdja_at(tr, 3)->child; ct; ct = ct->sibling)
  {
    printf("%*s", (int)indent, " ");

    char *head = fdja_ls(ct, "0", NULL);
    char *atts = fdja_ld(ct, "1");

    printf(" %s%s %s", cdgrey, head, atts);

    printf("%s\n", cclear);

    free(head);
    free(atts);
  }
}

static void print_msg_tree(const char *date, fdja_value *msg)
{
  fdja_value *tr = fdja_l(msg, "tree");
  if (tr == NULL) return;

  char *pt = fdja_ls(msg, "point", NULL); if (pt == NULL) pt = strdup("??");
  char *nid = fdja_ls(msg, "nid", NULL); if (nid == NULL) nid = strdup(" ");
  size_t l = fdja_li(tr, "2", (size_t)0);

  printf(
    "%s%s %s%.2s %s%zu %s%s",
    cdgrey, date, cclear, pt, cbrown, l, cdgrey, nid);

  char *head = fdja_ls(tr, "0", NULL);
  char *atts = fdja_ld(tr, "1");

  printf(" %s%s %s", cdblue, head, atts);

  printf("%s\n", cclear);

  char *s = flu_sprintf("%s %.2s %zu %s", date, pt, l, nid);

  print_msg_child(strlen(s) + 2, tr);

  free(pt);
  free(nid);
  free(head);
  free(atts);
  free(s);
}

static void print_tree(const char *fpath)
{
  FILE *f = fopen(fpath, "r");

  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath);
    perror("reason:");
    return;
  }

  char *line = NULL;
  size_t len = 0;
  fdja_value *v = NULL;

  while (getline(&line, &len, f) != -1)
  {
    char *date = strndup(line, 26);
    char *br = strchr(line, '{');
    v = fdja_parse(br); if (v) v->sowner = 0;

    print_msg_tree(date, v);

    free(date);
    fdja_free(v);

    //break;
  }
  free(line);
  fclose(f);
}

static void print_tsk_log(const char *path)
{
  char *fpath = flu_sprintf("%s/tsk.log", path);

  //char *s = flu_readall(fname);
  //if (s) puts(s);
  //free(s);

  FILE *f = fopen(fpath, "r");

  if (f == NULL)
  {
    printf("couldn't read file at %s\n", fpath);
    perror("reason:");
    free(fpath);
    return;
  }

  free(fpath);

  char *line = NULL;
  size_t len = 0;
  fdja_value *v = NULL;

  while (getline(&line, &len, f) != -1)
  {
    printf("%s%.26s ", cdgrey, line);

    char *br = strchr(line, '{');
    v = fdja_parse(br); if (v) v->sowner = 0;

    if (v && fdja_l(v, "task") == NULL) // raw tasker output
    {
      printf(br);
    }
    else if (v) // regular line
    {
      size_t line = fdja_li(v, "tree.2", (size_t)0);
      printf("%s%zu ", cbrown, line);

      char *nid = fdja_ls(v, "nid", NULL);
      printf("%s%s ", cdgrey, nid);
      free(nid);

      char *st = fdja_ls(v, "task.state", NULL);
      char *ev = fdja_ls(v, "task.event", NULL);
      char *fr = fdja_ls(v, "task.from", NULL);
      char *fo = fdja_ls(v, "task.for", NULL);
      char *frr = fr ? strstr(fr, "/tsk/") : NULL;
      printf("%sst:%s%s ", cdgrey, cclear, st);
      printf("%sev:%s%s ", cdgrey, cclear, ev);
      printf("%sfr:%s%s ", cdgrey, cclear, frr ? frr + 5 : fr);
      printf("%sfo:%s%s ", cdgrey, cgreen, fo);
      free(st); free(ev); free(fr); free(fo);

      //printf("\n  ");
      printf(cbrown);
      fdja_to_d(stdout, fdja_l(v, "payload"), FDJA_F_COMPACT, 0);

      printf("\n");
    }
    else // failed to parse
    {
      printf(br);
    }
    fdja_free(v);
  }
  free(line);
  fclose(f);
}

void flon_pp_execution(const char *exid)
{
  char *fep = flon_exid_path(exid);

  char *path = flu_sprintf("var/archive/%s", fep);
  if (flu_fstat(path) == '\0')
  {
    free(path);
    path = flu_sprintf("var/run/%s", fep);
  }

  printf("\n# %s/\n", path);

  puts("\n## trees\n#");
  flu_system("tree -h %s", path);
  flu_system("tree -h var/spool/ -P *%s*", exid);
  flu_system("tree -h var/log/%s", fep);

  puts("\n## dispatcher log\n#");
  printf(cgreen); fflush(stdout);
  flu_system(
    "cat var/log/dispatcher.log | grep --colour=never \"%s\"", exid);
  printf(cclear);

  puts("\n## execution log\n#");
  printf(cgreen); fflush(stdout);
  flu_system(
    "cat %s/exe.log", path);
  printf(cclear);

  puts("\n## invocation log\n#");
  printf(cgreen); fflush(stdout);
  flu_system(
    "find var/log/%s -name \"inv_%s-*.log\" | xargs tail -n +1", fep, exid);
  printf(cclear);

  char *fpath = flu_sprintf("%s/msgs.log", path);

  puts("\n## msgs.log (timeline view)\n#");
  print_msgs_timeline(fpath);

  puts("\n## msgs.log (xmas view)\n#");
  print_msgs_xmastree(fpath);

  puts("\n## msgs.log (forest view)\n#");
  print_tree(fpath);

  free(fpath);

  puts("\n## tsk.log\n#");
  print_tsk_log(path);

  puts("\n## run.json\n#");
  fdja_value *v = fdja_parse_f("%s/run.json", path);
  if (v) {
    flu_putf(fdja_todc(v));
    fdja_free(v);
  }
  else
  {
    flu_system("cat %s/run.json", path);
  }

  puts("\n## timers\n#");
  //flu_system("ls -lh var/spool/tdis/%s", fep);
  flu_list *l = flon_list_json("var/spool/tdis/%s", fep);
  if (l) for (flu_node *n = l->first; n; n = n->next)
  {
    char *fn = n->item;
    puts(strrchr(fn, '/') + 1);
    fdja_value *v = fdja_parse_obj_f(fn);
    if (v) flu_putf(fdja_todc(v)); else puts("(null)");
    fdja_free(v);
  }
  flu_list_free_all(l);

  puts("\n## processed\n#");
  printf(cgreen); fflush(stdout);
  flu_system("ls -lh %s/processed", path);
  printf(cclear);

  puts("\n## exe.pid\n#");
  flu_system("cat %s/exe.pid", path);

  // over

  printf("\n\n# %s/ .\n", path);
  puts("");

  free(fep);
  free(path);
}

