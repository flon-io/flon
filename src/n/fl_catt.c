
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


static void eval_catt_pre_vars(
  char dir, fdja_value *node, fdja_value *msg, fdja_value *tree)
{
  fdja_value *vars = fdja_l(tree, "1.vars");

  if (vars == NULL) for (size_t i = 0; ; ++i)
  {
    char *s = fdja_ls(tree, "1._%d", i, NULL);
    if (s == NULL) break;

    if (strcmp(s, "vars") == 0)
    {
      vars = fdja_object_malloc();
    }
    free(s);

    if (vars) break;
  }

  if (vars == NULL) return;
  if (vars->type == 'f') return;

  if (vars->type == 't') vars = fdja_object_malloc();
  else if (vars->type == 'o') {} // vars = vars;
  else return;

  fdja_value *nvars = fdja_l(node, "vars");

  if (nvars == NULL)
    fdja_set(node, "vars", vars->key == NULL ? vars : fdja_clone(vars));
  else
    fdja_merge(nvars, vars);
}

static void eval_catt_pre(char dir, fdja_value *node, fdja_value *msg)
{
  if (dir != 'e') return;

  fdja_value *t = tree(node, msg);

  eval_catt_pre_vars(dir, node, msg, t);
}

static void eval_catt_post(char dir, fdja_value *node, fdja_value *msg)
{
}

