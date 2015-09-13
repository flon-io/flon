
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

// https://github.com/flon-io/tsifro

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ow-crypt.h"
#include "tsifro.h"

#define BC_RANDSIZE 16
#define BC_HASHSIZE 64


char *ftsi_generate_bc_salt(char *prefix, int work_factor)
{
  if (work_factor < 4 || work_factor > 31) { errno = EINVAL; return NULL; }

  char rand[BC_RANDSIZE];

  FILE *f = fopen("/dev/urandom", "r");
  if (f == NULL) return NULL;

  size_t r = fread(rand, sizeof(char), BC_RANDSIZE, f);
  if (r < BC_RANDSIZE) return NULL;

  if (fclose(f) != 0) return NULL;

  char *salt = calloc(BC_HASHSIZE + 1, sizeof(char));

  if (prefix == NULL) prefix = "$2a$";

  char *s = crypt_gensalt_rn(
    prefix, work_factor, rand, BC_RANDSIZE, salt, BC_HASHSIZE);

  if (s == NULL) { free(salt); return NULL; }

  return salt;
}

char *ftsi_bc_hash(const char *s, const char *salt)
{
  char *hash = calloc(BC_HASHSIZE + 1, sizeof(char));

  char *r = crypt_rn(s, salt, hash, BC_HASHSIZE);

  if (r == NULL) { free(hash); return NULL; }

  return hash;
}

int ftsi_bc_verify(const char *pass, const char *hash)
{
  char h[BC_HASHSIZE + 1];
  strncpy(h, hash, BC_HASHSIZE + 1); // since strncpy pads with nulls

  char hh[BC_HASHSIZE + 1];
  memset(hh, 0, BC_HASHSIZE + 1);

  crypt_rn(pass, h, hh, BC_HASHSIZE);

  int r = 1;
  for (size_t i = 0; i < BC_HASHSIZE; ++i) { r = (h[i] == hh[i]) ? r : 0; }

  return r;
}

//commit ed046108f04b22ddb69795b05b71087be7a56ab9
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Jul 19 10:05:06 2015 +0900
//
//    2015
