
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

// https://github.com/flon-io/bocla

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <curl/curl.h>

#include "bocla.h"


void fcla_response_free(fcla_response *r)
{
  //if (r->headers != NULL) flu_list_free(r->headers); // not enough
  if (r->headers != NULL) flu_list_and_items_free(r->headers, free);
  if (r->body != NULL) free(r->body);
  free(r);
}

static size_t fcla_w(void *v, size_t s, size_t n, void *b)
{
  return flu_sbfwrite(b, v, s, n);
}

static short fcla_extract_status(char *head)
{
  return strtol(strchr(head, ' '), NULL, 10);
}

static char *fcla_crlf(char *s)
{
  char *s0 = strchr(s, '\r');
  if (s0 != NULL && s0[1] == '\n' && s0[2] != '\0') return s0 + 2;
  return NULL;
}

flu_list *fcla_extract_headers(char *head)
{
  flu_list *l = flu_list_malloc();
  char *s = head;
  while (1)
  {
    s = fcla_crlf(s);
    if (s == NULL || s[0] == '\r') break;
    char *c = strchr(s, ':');
    char *r = strchr(c + 1, '\r');
    char *k = strndup(s, c - s);
    char *v = strndup(c + 1, r - c - 1);
    flu_list_set(l, k, flu_strtrim(v));
    free(k);
    free(v);
  }
  return l;
}

static fcla_response *fcla_request(
  char meth,
  char *uri,
  flu_list *headers,
  char *sbody,
  FILE *fbody)
{
  fcla_response *res = calloc(1, sizeof(fcla_response));
  res->status_code = -1;

  char *buffer = NULL;
  flu_sbuffer *bhead = NULL;
  flu_sbuffer *bbody = NULL;

  CURL *curl;
  struct curl_slist *cheaders = NULL;

  curl = curl_easy_init();

  if (curl == NULL) { res->body = "curl initialization failed"; goto _done; }

  buffer = calloc(512, sizeof(char));

  if (meth == 'h') curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
  //else if (meth == 'p') curl_easy_setopt(curl, CURLOPT_POST, 1);
  //else if (meth == 'u') curl_easy_setopt(curl, CURLOPT_PUT, 1);
  else if (meth == 'p') curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
  else if (meth == 'u') curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
  else if (meth == 'd') curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");

  curl_easy_setopt(curl, CURLOPT_URL, uri);

  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, buffer);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
  curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, fcla_w);

  bhead = flu_sbuffer_malloc();
  bbody = flu_sbuffer_malloc();
  //
  curl_easy_setopt(curl, CURLOPT_HEADERDATA, bhead);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, bbody);

  if (meth == 'p' || meth == 'u')
  {
    if (sbody)
    {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sbody);
    }
    else // fbody
    {
      struct stat fi; fstat(fileno(fbody), &fi);
      curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
      curl_easy_setopt(curl, CURLOPT_READDATA, fbody);
      curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fi.st_size);
    }
  }

  if (headers != NULL)
  {
    flu_list *tl = flu_list_dtrim(headers);
    for (flu_node *n = tl->first; n != NULL; n = n->next)
    {
      char *s = flu_sprintf("%s: %s", n->key, (char *)n->item);
      cheaders = curl_slist_append(cheaders, s);
      free(s);
    }
    flu_list_free(tl);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, cheaders);
  }

  //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

  CURLcode r = curl_easy_perform(curl);

  if (r != CURLE_OK) { res->body = buffer; goto _done; }

  char *shead = flu_sbuffer_to_string(bhead); bhead = NULL;
  //printf("\n-->\n%s<--\n", shead);

  res->status_code = fcla_extract_status(shead);
  res->headers = fcla_extract_headers(shead);

  free(shead);

  res->body = flu_sbuffer_to_string(bbody); bbody = NULL;

_done:

  if (curl != NULL) curl_easy_cleanup(curl);
  if (res->status_code > -1 && buffer != NULL) free(buffer);
  if (bhead != NULL) flu_sbuffer_free(bhead);
  if (bbody != NULL) flu_sbuffer_free(bbody);
  if (cheaders != NULL) curl_slist_free_all(cheaders);

  return res;
}

fcla_response *fcla_get_h(char *uri, flu_list *headers)
{
  return fcla_request('g', uri, headers, NULL, NULL);
}

fcla_response *fcla_get(char *uri)
{
  return fcla_get_h(uri, NULL);
}

fcla_response *fcla_head(char *uri)
{
  return fcla_request('h', uri, NULL, NULL, NULL);
}

fcla_response *fcla_delete(char *uri)
{
  return fcla_request('d', uri, NULL, NULL, NULL);
}

fcla_response *fcla_post(char *uri, flu_dict *headers, char *body)
{
  return fcla_request('p', uri, headers, body, NULL);
}

fcla_response *fcla_post_f(char *uri, flu_dict *headers, char *path)
{
  FILE *f = fopen(path, "r");
  // TODO: error handling

  return fcla_request('p', uri, headers, NULL, f);
}

fcla_response *fcla_put(char *uri, flu_dict *headers, char *body)
{
  return fcla_request('u', uri, headers, body, NULL);
}

fcla_response *fcla_put_f(char *uri, flu_dict *headers, char *path)
{
  FILE *f = fopen(path, "r");
  // TODO: error handling

  return fcla_request('u', uri, headers, NULL, f);
}

