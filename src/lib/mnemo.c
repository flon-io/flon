
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

// https://github.com/flon-io/mnemo

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <string.h>

#include "mnemo.h"


static char *fmne_syls[] = {
   "ba",  "bi",  "bu",  "be",  "bo",
  "cha", "chi", "chu", "che", "cho",
   "da",  "di",  "du",  "de",  "do",
   "fa",  "fi",  "fu",  "fe",  "fo",
   "ga",  "gi",  "gu",  "ge",  "go",
   "ha",  "hi",  "hu",  "he",  "ho",
   "ja",  "ji",  "ju",  "je",  "jo",
   "ka",  "ki",  "ku",  "ke",  "ko",
   "la",  "li",  "lu",  "le",  "lo",
   "ma",  "mi",  "mu",  "me",  "mo",
   "na",  "ni",  "nu",  "ne",  "no",
   "pa",  "pi",  "pu",  "pe",  "po",
   "ra",  "ri",  "ru",  "re",  "ro",
   "sa",  "si",  "su",  "se",  "so",
  "sha", "shi", "shu", "she", "sho",
   "ta",  "ti",  "tu",  "te",  "to",
  "tsa", "tsi", "tsu", "tse", "tso",
   "wa",  "wi",  "wu",  "we",  "wo",
   "ya",  "yi",  "yu",  "ye",  "yo",
   "za",  "zi",  "zu",  "ze",  "zo",
};
static size_t fmne_syl_count = 100;

//static char *fmne_neg = "wi";
static char *fmne_neg = "xa";


static void fmne_tos(long long i, FILE *f)
{
  long long mod = i % fmne_syl_count;
  long long rst = i / fmne_syl_count;
  if (rst > 0) fmne_tos(rst, f);
  fputs(fmne_syls[mod], f);
}

char *fmne_to_s(long long i)
{
  char *s = NULL;
  size_t l = 0;
  FILE *f = open_memstream(&s, &l);

  if (i < 0) { fputs(fmne_neg, f); i = i * -1; }

  fmne_tos(i, f);

  fclose(f);

  return s;
}

fmne_toi_result fmne_to_i(char *s)
{
  fmne_toi_result r = { 0, 0 };

  int sign = 1; if (strncmp(s, fmne_neg, 2) == 0) { sign = -1; s = s + 2; }

  while (1)
  {
    if (r.err || *s == '\0') break;

    for (size_t i = 0; i < fmne_syl_count; ++i)
    {
      char *syl = fmne_syls[i];
      size_t l = strlen(syl);

      if (strncmp(s, syl, l) == 0)
      {
        s = s + l;
        r.result = fmne_syl_count * r.result + i;
        break;
      }

      if (i == fmne_syl_count - 1) { r.err = 1; r.result = 0; }
    }
  }

  r.result = sign * r.result;

  //printf("r.err: %i, r.result: %lli\n", r.err, r.result);

  return r;
}

int fmne_is_mnemo(char *s)
{
  return (fmne_to_i(s).err == 0);
}

//commit dc0631162d3f34e68eec1921199b4f6c6207bf65
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Thu Sep 3 20:55:28 2015 +0900
//
//    2015
