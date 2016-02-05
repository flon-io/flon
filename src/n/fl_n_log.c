
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


static char exe_log(fdja_value *node, fdja_value *exe)
{
  fdja_value *t = tree(node, exe);
  fdja_value *atts = fdja_at(t, 1);

  flu_sbuffer *sb = NULL;

  char level = 'i';
  if (atts->child)
  {
    level = tolower(*fdja_srk(atts->child));
    sb = flu_sbuffer_malloc();
  }

  size_t line = fdja_li(t, "2", 0);
  //char *file = fdja_ls(t, "4", NULL);
  char *file = radfile(node, exe);

  if (sb) for (fdja_value *a = atts->child->sibling; a; a = a->sibling)
  {
    char *s = fdja_tod(a); flu_sbputs(sb, s); free(s);
    if (a->sibling) flu_sbputc(sb, ' ');
  }
  char *msg = flu_sbuffer_to_string(sb);

  char *func = "log";

  fgaj_log(
    level, level == 'r',
    file, line, func, NULL, // TODO: adapt subject?
    msg);

  free(file);
  free(msg);

  return 'v'; // over
}

