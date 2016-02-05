
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


static flu_list *list_numbers(fdja_value *rets)
{
  flu_list *r = flu_list_malloc();

  for (fdja_value *v = rets->child; v; v = v->sibling)
  {
    flu_list_add(r, fdja_to_string(v));
  }

  return r;
}

static char determine_type(flu_list *numbers)
{
  for (flu_node *n = numbers->first; n; n = n->next)
  {
    if (strchr(n->item, '.')) return 'd'; // double
  }
  return 'i'; // integer
}

static long long arith_fold_i(fdja_value *node, char *op, flu_list *numbers)
{
  if (numbers->first == NULL) return 0;

  long long r = strtoll(numbers->first->item, NULL, 10);

  for (flu_node *n = numbers->first->next; n; n = n->next)
  {
    long long nn = strtoll(n->item, NULL, 10);
    if (*op == '+') r += nn;
    else if (*op == '-') r -= nn;
    else if (*op == '*') r *= nn;
    else if (*op == '/')
    {
      if (nn == 0) { push_error(node, "division by zero", NULL); return 0; }
      r /= nn;
    }
    // else do nothing about it...
  }

  return r;
}

////static int is_ldzero(long double n)
//static int is_ldzero(char *n)
//{
//  //return (nn == 0.0);
//  for (size_t i = 0; ; ++i)
//  {
//    if (n[i] == 0) break;
//    if (n[i] != '0' && n[i] == '.') return 0;
//  }
//  return 1;
//
//  // well...
//}

static double arith_fold_d(fdja_value *node, char *op, flu_list *numbers)
{
  if (numbers->first == NULL) return 0;

  long double r = strtold(numbers->first->item, NULL);

  for (flu_node *n = numbers->first->next; n; n = n->next)
  {
    long double nn = strtold(n->item, NULL);
    if (*op == '+') r += nn;
    else if (*op == '-') r -= nn;
    else if (*op == '*') r *= nn;
    else if (*op == '/') r /= nn;
    //else if (*op == '/')
    //{
    //  if (is_ldzero(n->item))
    //  {
    //    push_error(node, "division by zero", NULL); return 0;
    //  }
    //  r /= nn;
    //}
  }

  return r;
}

static char rcv_arith(fdja_value *node, fdja_value *rcv)
{
  char r = seq_rcv(node, rcv);

  if (r != 'v') return r;

  size_t rcount = error_count(node);

  char *op = fdja_ls(node, "inst", NULL);
  flu_list *ns = list_numbers(fdja_l(node, "rets"));
  char t = determine_type(ns);

  fdja_value *ret = NULL;
  if (t == 'i')
    ret = fdja_v("%d", arith_fold_i(node, op, ns));
  else // 'd'
    ret = fdja_v("%f", arith_fold_d(node, op, ns));

  fdja_pset(rcv, "payload.ret", ret);

  flu_list_free_all(ns);
  free(op);

  if (error_count(node) > rcount) return 'r';
  return 'v';
}

static char exe_arith(fdja_value *node, fdja_value *exe)
{
  return seq_exe(node, exe, 1); // 1 to collect the "ret"s
}

