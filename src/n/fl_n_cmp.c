
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


static int rmatch(
  fdja_value *node, const char *op, fdja_value *l, fdja_value *r)
{
  int i = 0;

  char *str = fdja_to_string(l);

  char *reg = fdja_to_string(r);
  char *oreg = reg;

  int flags = REG_EXTENDED;

  if (*reg == '/')
  {
    char *rslash = strrchr(reg, '/');
    if (rslash && rslash != reg)
    {
      reg = strndup(reg + 1, rslash - reg - 1);
      if (strchr(rslash + 1, 'i')) flags |= REG_ICASE;
      free(oreg);
    }
  }

  regex_t regex;
  regmatch_t matches[128];

  int ii = regcomp(&regex, reg, flags);
  if (ii != 0)
  {
    char buf[128];
    regerror(ii, &regex, buf, 127);

    push_error(
      node, "regex compilation failed: %s in >%s<", buf, reg, NULL);

    goto _over;
  }

  //flags = REG_NOTBOL | REG_NOTEOL;
  flags = 0;
  ii = regexec(&regex, str, 128, matches, flags);

  i = (ii == 0);
  if (*op == '!') i = ! i;

  if (*op == '=' && i == 1 && matches[1].rm_so > -1)
  {
    fdja_value *vmatches = fdja_array_malloc();

    for (size_t j = 0; j < 129; ++j)
    {
      int so = matches[j].rm_so;
      if (so == -1) break;
      fdja_push(vmatches, fdja_sym(strndup(str + so, matches[j].rm_eo - so)));
    }

    set_var(node, 'l', "matches", vmatches);
  }

_over:

  free(str);
  free(reg);

  regfree(&regex);

  return i;
}

static int cmp_object(char *op, char *l, char *r)
{
  if (strcmp(op, "==") == 0) return strcmp(l, r) == 0;
  if (strcmp(op, "!=") == 0) return strcmp(l, r) != 0;
  if (strcmp(op, ">=") == 0) return strcmp(l, r) >= 0;
  if (strcmp(op, "<=") == 0) return strcmp(l, r) <= 0;
  if (strcmp(op, ">") == 0) return strcmp(l, r) > 0;
  if (strcmp(op, "<") == 0) return strcmp(l, r) < 0;
  return 0;
}

static int cmp_integer(char *op, char *l, char *r)
{
  long long ll = strtoll(l, NULL, 10);
  long long lr = strtoll(r, NULL, 10);

  if (strcmp(op, "==") == 0) return ll == lr;
  if (strcmp(op, "!=") == 0) return ll != lr;
  if (strcmp(op, ">=") == 0) return ll >= lr;
  if (strcmp(op, "<=") == 0) return ll <= lr;
  if (strcmp(op, ">") == 0) return ll > lr;
  if (strcmp(op, "<") == 0) return ll < lr;
  return 0;
}

static int cmp_double(char *op, char *l, char *r)
{
  //long double dl = strtold(l, NULL);
  //long double dr = strtold(l, NULL);

  // epsilon? ulp?
  return 0;
}

static int cmp_number(char *op, char *l, char *r)
{
  return strchr(l, '.') ? cmp_double(op, l, r) : cmp_integer(op, l, r);
}

static int cmp(
  fdja_value *node, char *op, fdja_value *l, fdja_value *r)
{
  char *jl = fdja_to_json(l);
  char *jr = fdja_to_json(r);

  int ret = 0;

  if (op[1] == '~')
    ret = rmatch(node, op, l, r);
  else if (l->type == 'n' && r->type == 'n')
    ret = cmp_number(op, jl, jr);
  else if (l->type == 'n')
    ret = 0;
  else
    ret = cmp_object(op, jl, jr);

  free(jl); free(jr);

  return ret;
}

static char rcv_cmp(fdja_value *node, fdja_value *rcv)
{
  char r = seq_rcv(node, rcv);

  if (r != 'v') return r;

  size_t rcount = error_count(node);

  char *op = fdja_ls(node, "inst", NULL);
  fdja_value *rets = fdja_l(node, "rets");
  size_t l = fdja_size(rets);

  int ret = 1;

  if (l > 1) for (fdja_value *r = rets->child; ; r = r->sibling)
  {
    if (--l == 0) break;

    fdja_value *left = r;
    fdja_value *right = r->sibling;

    ret = cmp(node, op, left, right);

    if (ret == 0) break;
  }

  fdja_psetv(rcv, "payload.ret", ret ? "true" : "false");

  free(op);

  if (error_count(node) > rcount) return 'r';
  return 'v';
}

static char exe_cmp(fdja_value *node, fdja_value *exe)
{
  return seq_exe(node, exe, 1); // 1 to collect the "ret"s
}

