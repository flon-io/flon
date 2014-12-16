
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

// https://github.com/flon-io/flutil

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>


static char sixfours[64] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789"
  "+/";
static int foursixes[122 - 43] =
  {};

static void init_foursixes()
{
  for (size_t i = 0; i < 64; ++i) foursixes[sixfours[i] - 43] = i;
}

void flu64_do_encode(char *in, size_t l, char *out)
{
  unsigned char a, b, c;
  int w, x, y, z;
  int bpad, cpad;

  for (size_t i = 0, j = 0; i < l; )
  {
    a = in[i++];
    b = 0; if (i < l) b = in[i]; bpad = (i >= l); i++;
    c = 0; if (i < l) c = in[i]; cpad = (i >= l); i++;

    w = a >> 2;
    x = ((0x03 & a) << 4) | (b >> 4);
    y = ((0x0f & b) << 2) | (c >> 6);
    z = 0x3f & c;

    out[j++] = sixfours[w];
    out[j++] = sixfours[x];
    out[j++] = bpad ? '=' : sixfours[y];
    out[j++] = cpad ? '=' : sixfours[z];
  }
}

void flu64_do_decode(char *in, size_t l, char *out)
{
  if (foursixes[0] == 0) init_foursixes();

  int w, x, y, z;

  for (size_t i = 0, j = 0; i < l; )
  {
    w = foursixes[in[i++] - 43]; x = foursixes[in[i++] - 43];
    y = foursixes[in[i++] - 43]; z = foursixes[in[i++] - 43];

    out[j++] = (w << 2) | (x >> 4);
    if (in[i - 2] != '=') out[j++] = ((x & 0x0f) << 4) | (y >> 2);
    if (in[i - 1] != '=') out[j++] = ((y & 0x03) << 6) | z;
  }
}

char *flu64_encode(char *in, ssize_t l)
{
  if (l < 0) l = strlen(in);
  char *out = calloc(l * 2, sizeof(char));

  flu64_do_encode(in, l, out);

  return out;
}

char *flu64_decode(char *in, ssize_t l)
{
  if (l < 0) l = strlen(in);
  char *out = calloc(l, sizeof(char));

  flu64_do_decode(in, l, out);

  return out;
}

char *flu64_encode_for_url(char *in, ssize_t l)
{
  char *r = flu64_encode(in, l);

  for (size_t i = 0; ; ++i)
  {
    if (r[i] == 0) break;

    if (r[i] == '=') r[i] = '.';
    else if (r[i] == '+') r[i] = '-';
    else if (r[i] == '/') r[i] = '_';
  }

  return r;
}

char *flu64_decode_from_url(char *in, ssize_t l)
{
  char *r = NULL;

  char *s = strdup(in);
  if (l < 0) l = strlen(in);

  for (size_t i = 0; i < l; ++i)
  {
    if (s[i] == '.') s[i] = '=';
    else if (s[i] == '-') s[i] = '+';
    else if (s[i] == '_') s[i] = '/';
  }

  r = flu64_decode(s, l);

  free(s);

  return r;
}

