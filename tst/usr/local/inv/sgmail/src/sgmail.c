
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
#include <string.h>
#include <stdlib.h>

#include "flutil.h"
#include "djan.h"
#include "bocla.h"


#define SENDGRID "https://api.sendgrid.com/api/mail.send.json"


static int fail(fdja_value *j, char *msg)
{
  fdja_value *r = j;
  if (r == NULL) r = fdja_v("{_r:0}");
  fdja_set(r, "msg", fdja_s(msg));

  puts(fdja_to_json(r));

  if (j == NULL) fdja_free(r);

  return 1; // not 0
}

static void set(
  flu_sbuffer *sb, char *sgkey, fdja_value *j, char *jkey, char *def, int amp)
{
  char *val = NULL;

  // TODO lookup in `args` first

  if (j) val = fdja_ls(j, jkey, NULL);
  else val = strdup(jkey);

  if (val == NULL && def == NULL) return;
  if (val == NULL) val = strdup(def);

  char *eval = flu_urlencode(val);

  flu_sbputs(sb, sgkey);
  flu_sbputc(sb, '=');
  flu_sbputs(sb, eval);

  if (amp) flu_sbputc(sb, '&');

  free(val);
  free(eval);
}

int main(int argc, char *argv[])
{
  char *in = flu_freadall(stdin);
  fdja_value *pl = fdja_parse(in);

  if (pl == NULL) return fail("couldn't parse payload");

  //
  // prepare email

  char *user = flu_readall("sendgrid.credentials");
  if (user == NULL) return fail("couldn't read sendgrid.credentials");

  *(strchr(user, '\n')) = '\0'; // trim right
  char *col = strchr(user, ':');
  if (col == NULL) return fail("couldn't split sendgrid.credentials");
  *col = '\0';
  char *pass = col + 1;

  char *s = NULL;
  flu_sbuffer *sb = flu_sbuffer_malloc();

  set(sb, "api_user", NULL, user, NULL, 1);
  set(sb, "api_key", NULL, pass, NULL, 1);

  free(user);

  set(sb, "to", j, "to", "jmettraux+flon@gmail.com", 1);
  //set(sb, "toname", j, "toname", NULL, 1);
  set(sb, "subject", j, "subject", "(missing subject)", 1);
  set(sb, "text", j, "text", "(missing text)", 1);
  set(sb, "from", j, "from", "jmettraux+flon@gmail.com", 0);

  form = flu_sbuffer_to_string(sb);

  //
  // post email

  fcla_response *res = fcla_post(SENDGRID, NULL, form);

  //
  // deal with result

  fdja_set(j, "_sendgrid", fdja_v("{}"));
  fdja_set(j, "_sendgrid.status", fdja_v("%i", res->status_code));
  fdja_set(j, "_sendgrid.body", fdja_s(res->body));

  fcla_response_free(res);

  //
  // respond (to dispatcher)

  fdja_set(j, "_r", fdja_v("1")); // success

  puts(fdja_to_json(j));

  return 0;
}

