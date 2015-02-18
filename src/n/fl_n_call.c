
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


// TODO: consider moving this to fl_call.c

static char *locate_lib(const char *libname)
{
  char *dash = strchr(execution_id, '-');
  if (dash == NULL) return NULL;

  char *dom = strndup(execution_id, dash - execution_id);
  char s = 0;

  while (1)
  {
    s = flu_fstat("usr/local/lib/%s/%s", dom, libname);

    if (s == 'f') break;
    if (strcmp(dom, "any") == 0) break;

    char *dot = strrchr(dom, '.');

    if (dot == NULL) { free(dom); dom = strdup("any"); }
    else { char *d = dom; dom = strndup(d, dot - d); free(d); }
  }

  char *r = NULL;

  if (s == 'f') r = flu_sprintf("usr/local/lib/%s/%s", dom, libname);

  free(dom);

  return r;
}

static char call_lib(
  fdja_value *node, fdja_value *exe,
  fdja_value *cargs, const char *libname)
{
  // Note: for now, we don't care how many times a lib is read and bound
  // in the current execution...

  char r = 'r'; // 'error' for now
  char *path = NULL;
  char *cnid = NULL;

  // 1. find lib

  if (execution_id == NULL)
  {
    push_error(node, "cannot call lib, execution_id is NULL", NULL);
    goto _over;
  }

  path = locate_lib(libname);

  if (path == NULL)
  {
    push_error(node, "couldn't find lib '%s'", libname, NULL);
    goto _over;
  }

  log_i(node, exe, "lib at %s", path);

  fdja_value *t = fdja_parse_radial_f(path);
  //fdja_putdc(t);

  if (t == NULL)
  {
    push_error(node, "couldn't parse lib at %s", path, NULL);
    goto _over;
  }

  // 2. place lib in nodes{}

  size_t lc = libcounter_next();
  fdja_value *n = fdja_o("tree", t, NULL);

  fdja_pset(execution, "nodes.%zx", lc, n);

  // 4. call lib

  cnid = flu_sprintf("%zu-%zu", lc, counter_next());
  queue_child_execute(cnid, node, exe, fdja_clone(t), NULL);

  r = 'k';

_over:

  fdja_free(cargs);
  free(path);
  free(cnid);

  return r;
}

static char exe_call(fdja_value *node, fdja_value *exe)
{
  char r = 0;

  fdja_value *cargs = attributes(node, exe);
  char *cname = cargs->child ? fdja_to_string(cargs->child) : NULL;

  if (cname && flu_strends(cname, ".rad"))
    r = call_lib(node, exe, cargs, cname);
  else
    r = do_call(node, exe, cargs);

  free(cname);

  return r;
}

