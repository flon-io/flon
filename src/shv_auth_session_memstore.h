
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

// https://github.com/flon-io/shervin

#ifndef FLON_SHV_AUTH_SESSION_MEMSTORE_H
#define FLON_SHV_AUTH_SESSION_MEMSTORE_H

#include "flutil.h"
#include "shv_protected.h"


fshv_session *fshv_session_memstore_push(
  const char *sid, const char *user, const char *id, long long tus);

/* For specs only.
 */
flu_list *fshv_session_memstore();

#endif //FLON_SHV_AUTH_SESSION_MEMSTORE_H

//commit bbf5177a47f469da4e5f4f03c09b16e4d2b0e2b5
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Tue Dec 23 17:40:30 2014 +0900
//
//    pass the request to fshv_authenticate()
