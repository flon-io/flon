
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


// * unmapped arguments go into `variables.args`
// * mapped arguments go to fields, unless prefixed with "v."

static char do_call(fdja_value *node, fdja_value *msg, fdja_value *cargs)
{
  char r = 'k'; // ok for now

  char *nid = NULL;
  char *cnid = NULL;

  fdja_value *carg0 = cargs->child;
  fdja_value *callable = NULL;
  char *cname = NULL;

  if (carg0 == NULL)
  {
    push_error(node, "nothing to call", cargs); r = 'r'; goto _over;
  }

  if (is_callable(carg0))
  {
    callable = carg0;
  }
  else
  {
    cname = fdja_to_string(carg0);
    callable = lookup_var(node, 'l', cname); // 'l' for "local"

    if ( ! is_callable(callable))
    {
      push_error(node, "not callable '%s'", cname, NULL); r = 'r'; goto _over;
    }
  }

  // prepare new node's nid

  nid = fdja_ls(callable, "nid");
  cnid = flu_sprintf("%s-%x", nid, counter_next());
  //fgaj_d("nid >%s<, cnid >%s<", nid, cnid);

  // prepare tree

  fdja_value *tree = flon_node_tree_clone(nid);
  fdja_psetv(tree, "0", "sequence");
  fdja_psetv(tree, "1", "{}");

  // map arguments

  fdja_value *tvars = fdja_object_malloc();
  fdja_value *targs = fdja_set(tvars, "args", fdja_array_malloc());

  fdja_value *dargs = fdja_lc(callable, "args"); // define args

  fdja_push(targs, fdja_clone(cargs->child));

  for (fdja_value *a = cargs->child->sibling; a; a = a->sibling)
  {
    char *key = NULL;
    fdja_value *val = fdja_clone(a);

    if (is_index(a->key)) // regular attribute
    {
      fdja_value *arg = dargs->child;
      if (arg)
      {
        key = fdja_to_string(arg);
        fdja_splice(dargs, 0, 1, NULL); // shift first elt...
      }
    }
    else // named attribute
    {
      key = a->key;
      fdja_unpush(dargs, a->key);
    }

    if (key)
    {
      char p = extract_prefix(key);
      char *k = key; if (p != 0) k = strchr(key, '.') + 1;
      if (p == 'v')
        fdja_set(tvars, k, val);
      else
        fdja_pset(msg, "payload.%s", k, val);
    }
    else // no more keys, push to vars.args (targs)
    {
      fdja_push(targs, val);
    }

    if (key != a->key) free(key);
  }

  fdja_free(dargs);

  // trigger execution

  queue_child_execute(cnid, node, msg, tree, tvars);

_over:

  free(nid);
  free(cnid);
  free(cname);
  fdja_free(cargs);

  return r;
}

