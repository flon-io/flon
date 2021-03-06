
//
// Copyright (c) 2013-2014, John Mettraux, jmettraux+flon@gmail.com
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static char *escape(char *s)
{
  size_t l = strlen(s);
  char *r = calloc(l * 2 + 1, sizeof(char));

  for (size_t i = 0, j = 0; i < l; ++i)
  {
    if (s[i] == '"') r[j++] = '\\';
    r[j++] = s[i];
  }

  return r;
}

int main(int argc, char *argv[])
{
  printf("{");

  char *k = NULL;
  for (int i = 1; i < argc; ++i)
  {
    if (k == NULL) { k = argv[i]; continue; }

    printf("\"%s\": \"%s\",", k, escape(argv[i]));
    k = NULL;
  }
  // yes, it's hand-rolled json.

  printf("}");

  return 0;
}

