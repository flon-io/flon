
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


static char exe_set(fdja_value *node, fdja_value *exe)
{
  fdja_value *pl = payload(exe);

  fdja_value *atts = attributes(node, exe);

  for (fdja_value *a = atts->child; a; )
  {
    char *key = a->key;
    fdja_value *sibling = a->sibling;

    //a->sibling = NULL;
      // done by djan itself

    //fgaj_d("key: 0 >%s<", key);

    char k = extract_prefix(a->key);
    if (k != 0) key = strchr(key, '.') + 1;

    //fgaj_d("key: 1 >%s<", key);

    if (k == 'f' || k == 0)
      fdja_pset(pl, key, a);
    else if (k == 'v')
      set_var(node, *a->key, key, a);

    a = sibling;
  }

  //fdja_putdc(execution);

  atts->child = NULL; fdja_free(atts);

  return 'v'; // over
}

