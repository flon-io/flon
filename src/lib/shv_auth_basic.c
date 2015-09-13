
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

// https://github.com/flon-io/shervin

#define _POSIX_C_SOURCE 200809L


#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "flu64.h"
#include "shv_protected.h"


static int is_logout_request(fshv_env *env)
{
  char *lo = flu_list_get(env->req->uri->qentries, "logout");

  return
    lo &&
    (
      strlen(lo) < 1 ||
      strcmp(lo, "1") == 0 ||
      strcasecmp(lo, "yes") == 0 ||
      strcasecmp(lo, "true") == 0
    );
}

int fshv_basic_auth(
  fshv_env *env, const char *realm, fshv_user_pass_authentifier *a)
{
  //printf("uri: %s\n", flu_list_to_sm(env->req->uri->qentries));
  //printf("bag: %s\n", flu_list_to_sm(env->bag));

  int authentified = 0;
  char *user = NULL;

  if (is_logout_request(env)) goto _over;

  char *auth = flu_list_get(env->req->headers, "authorization");
  if (auth == NULL) goto _over;

  if (strncmp(auth, "Basic ", 6) != 0) goto _over;

  user = flu64_decode(auth + 6, -1);
  char *pass = strchr(user, ':');
  if (pass == NULL) goto _over;

  *pass = 0; pass = pass + 1;

  char *nuser = a(env, realm, user, pass);
  authentified = (nuser != NULL);

  if (nuser)
  {
    fshv_set_user(env, realm, nuser);
    free(nuser);
  }

_over:

  free(user);

  if ( ! authentified)
  {
    env->res->status_code = 401;
      // users of the auth are free to override that downstream

    if (realm)
    {
      flu_list_set(
        env->res->headers,
        "WWW-Authenticate", flu_sprintf("Basic realm=\"%s\"", realm));
    }
  }

  return authentified;
}

//commit 2e039a2191f1ff3db36d3297a775c3a1f58841e0
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Sep 13 06:32:55 2015 +0900
//
//    bring back all specs to green
//    
//    (one yellow remaining though)
