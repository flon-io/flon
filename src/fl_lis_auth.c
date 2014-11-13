
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

//#include <stdlib.h>

#include "gajeta.h"
#include "djan.h"
#include "tsifro.h"
#include "fl_listener.h"


// TODO: when receiving SIGHUP, reload passwd.json (and domain.json?)

static fdja_value *passwd = NULL;

static void load_passwd()
{
  passwd = fdja_parse_obj_f("etc/passwd.json");

  if (passwd) return;

  fgaj_r("couldn't load etc/passwd.json");
}

int flon_auth_enticate(char *user, char *pass)
{
  if (passwd == NULL) load_passwd();
  if (passwd == NULL) { fgaj_e("passwd not loaded"); return 0; }

  fdja_value *u = fdja_l(passwd, user);

  char *hash = u ? fdja_ls(u, "pass", NULL) : NULL;
  if (hash == NULL)
  {
    u = NULL;
    hash = "$2a$08$3xtQyM06K85SCaL2u8EadeO8FwLd5M1xEb4/mtxRaArfx1MFB9/QK";
      // dummy hash
  }

  // TODO: shouldn't the workfactor be set in the conf?

  int r = ftsi_bc_verify(pass, hash);

  return u ? r : 0;
}

