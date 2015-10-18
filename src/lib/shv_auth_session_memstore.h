
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

#ifndef FLON_SHV_AUTH_SESSION_MEMSTORE_H
#define FLON_SHV_AUTH_SESSION_MEMSTORE_H

#include "flutil.h"
#include "shv_protected.h"


/* : pushing with all the parameters set and expiry time:
 *   start or refreshes a session
 *   returns the new session in case of success, NULL else
 * : pushing with only the sid set and now:
 *   queries and expires,
 *   returns a session in case of success, NULL else
 * : pushing with only the sid set and -1:
 *   stops the session and returns NULL
 * : pushing with all NULL and -1:
 *   resets the store and returns NULL
 */
fshv_session *fshv_session_memstore_push(
  fshv_env *e, // can't remember why...
  const char *sid,
  const char *user,
  const char *id,
  long long tus);

/* For specs only.
 */
flu_list *fshv_session_memstore();

/* For specs only.
 */
size_t fshv_session_memstore_clear();

#endif //FLON_SHV_AUTH_SESSION_MEMSTORE_H

//commit 4f600185cfdd86e14d35ea326de3121ffa4ea561
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Sun Oct 18 15:19:12 2015 +0900
//
//    implement fshv_malloc_x()
