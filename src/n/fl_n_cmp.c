
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


static int cmp_object(char *op, char *l, char *r)
{
  if (strcmp(op, "==") == 0) return strcmp(l, r) == 0;
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

static char rcv_cmp(fdja_value *node, fdja_value *rcv)
{
  char r = seq_rcv(node, rcv);

  if (r != 'v') return r;

puts("<over");
fdja_putdc(node);
fdja_putdc(rcv);
puts("</over");

  return 'v';
}

static char exe_cmp(fdja_value *node, fdja_value *exe)
{
  return seq_exe(node, exe, 1); // 1 to collect the "ret"s
//  char r = 'v'; // 'over' for now
//
//  fdja_value *atts = attributes(node, exe);
//
//  if (fdja_size(atts) != 3)
//  {
//    set_error_note(node, "needs exactly 3 atts", atts);
//    r = 'r'; goto _over;
//  }
//
//  fdja_value *left = atts->child;
//  char *op = fdja_to_string(atts->child->sibling);
//  fdja_value *right = atts->child->sibling->sibling;
//
//  char *jl = fdja_to_json(left);
//  char *jr = fdja_to_json(right);
//
//  int ret = 0; //false for now
//
//  if (left->type == 'n' && right->type == 'n')
//    ret = cmp_number(op, jl, jr);
//  else if (left->type == 'n')
//    ret = 0;
//  else
//    ret = cmp_object(op, jl, jr);
//
//  free(op); free(jl); free(jr);
//
//  fdja_psetv(exe, "payload.ret", ret ? "true" : "false");
//
//_over:
//
//  fdja_free(atts);
//
//  return r;
}

