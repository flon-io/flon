
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

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ev.h>

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "shervin.h"
#include "shv_protected.h"


//
// shv_response

shv_response *shv_response_malloc(short status_code)
{
  shv_response *r = calloc(1, sizeof(shv_response));
  r->status_code = status_code;
  r->headers = flu_list_malloc();
  r->body = flu_list_malloc();

  return r;
}

void shv_response_free(shv_response *r)
{
  flu_list_free_all(r->headers);
  flu_list_free_all(r->body);
  free(r);
}

//
// shv_respond

static char *shv_reason(short status_code)
{
  if (status_code == 200) return "OK";

  if (status_code == 400) return "Bad Request";
  if (status_code == 404) return "Not Found";

  if (status_code == 500) return "Internal Server Error";

  if (status_code == 100) return "Continue";
  if (status_code == 101) return "Switching Protocols";
  if (status_code == 201) return "Created";
  if (status_code == 202) return "Accepted";
  if (status_code == 203) return "Non-Authoritative Information";
  if (status_code == 204) return "No Content";
  if (status_code == 205) return "Reset Content";
  if (status_code == 206) return "Partial Content";
  if (status_code == 300) return "Multiple Choices";
  if (status_code == 301) return "Moved Permanently";
  if (status_code == 302) return "Found";
  if (status_code == 303) return "See Other";
  if (status_code == 304) return "Not Modified";
  if (status_code == 305) return "Use Proxy";
  if (status_code == 307) return "Temporary Redirect";
  if (status_code == 401) return "Unauthorized";
  if (status_code == 402) return "Payment Required";
  if (status_code == 403) return "Forbidden";
  if (status_code == 405) return "Method Not Allowed";
  if (status_code == 406) return "Not Acceptable";
  if (status_code == 407) return "Proxy Authentication Required";
  if (status_code == 408) return "Request Time-out";
  if (status_code == 409) return "Conflict";
  if (status_code == 410) return "Gone";
  if (status_code == 411) return "Length Required";
  if (status_code == 412) return "Precondition Failed";
  if (status_code == 413) return "Request Entity Too Large";
  if (status_code == 414) return "Request-URI Too Large";
  if (status_code == 415) return "Unsupported Media Type";
  if (status_code == 416) return "Requested range not satisfiable";
  if (status_code == 417) return "Expectation Failed";
  if (status_code == 501) return "Not Implemented";
  if (status_code == 502) return "Bad Gateway";
  if (status_code == 503) return "Service Unavailable";
  if (status_code == 504) return "Gateway Time-out";
  if (status_code == 505) return "HTTP Version not supported";
  return "(no reason-phrase)";
}

static char *shv_low_reason(short status_code)
{
  char *s = shv_reason(status_code);
  char *r = calloc(strlen(s) + 1, sizeof(char));
  for (char *rr = r; *s; ++rr, ++s) *rr = tolower(*s);

  return r;
}

static void shv_lower_keys(flu_dict *d)
{
  for (flu_node *n = d->first; n != NULL; n = n->next)
  {
    for (char *s = n->key; *s; ++s) *s = tolower(*s);
  }
}

static void shv_set_content_length(shv_con *con)
{
  char *s = flu_list_get(con->res->headers, "shv_content_length");

  if (s)
  {
    if (flu_list_get(con->req->headers, "x-real-ip")) s = strdup("");
    else s = strdup(s);
  }
  else
  {
    size_t r = 0;

    for (flu_node *n = con->res->body->first; n; n = n->next)
    {
      r += strlen((char *)n->item);
    }

    s = flu_sprintf("%zu", r);
  }

  flu_list_set(con->res->headers, "content-length", s);
}

static int pipe_file(char *path, FILE *dst)
{
  int r = 0;

  FILE *src = fopen(path, "r");
  if (src == NULL) return 1;

  char buffer[SHV_BUFFER_SIZE + 1];
  size_t rl, wl;

  while (1)
  {
    rl = fread(buffer, sizeof(char), SHV_BUFFER_SIZE, src);
    if (rl > 0)
    {
      wl = fwrite(buffer, sizeof(char), rl, dst);
      if (wl < rl) fgaj_w("wrote %zu of %zu chars :-(", wl, rl);
    }

    if (rl < SHV_BUFFER_SIZE)
    {
      if (feof(src)) break;

      fgaj_w("read only %zu of %zu chars, but not eof", rl, SHV_BUFFER_SIZE);
      r = 1; break;
    }
  }

  fclose(src);

  return r;
}

void shv_respond(struct ev_loop *l, struct ev_io *eio)
{
  shv_con *con = (shv_con *)eio->data;

  // prepare headers

  flu_list_set(con->res->headers, "date", flu_tstamp(NULL, 1, 'r'));

  flu_list_set_last(
    con->res->headers, "server", flu_sprintf("shervin %s", SHV_VERSION));
  flu_list_set_last(
    con->res->headers, "content-type", strdup("text/plain; charset=utf-8"));

  flu_list_set(
    con->res->headers, "location", strdup("northpole")); // FIXME

  long long now = flu_gets('u');
  //
  flu_list_set(
    con->res->headers,
    "x-flon-shervin",
    flu_sprintf(
      "c%.3fms;r%.3fms;rq%i",
      (now - con->startus) / 1000.0,
      (now - con->req->startus) / 1000.0,
      con->rqount));

  if (
    con->res->body->size == 0 &&
    con->res->status_code >= 400 &&
    con->res->status_code < 600 &&
    flu_list_get(con->res->headers, "shv_content_length") == NULL)
  {
    // set 4xx or 5xx reason as body
    flu_list_add(con->res->body, shv_low_reason(con->res->status_code));
  }

  shv_set_content_length(con);

  // write to eio->fd (if there is one)

  if (l == NULL) return; // only in spec/handle_spec.c

  // write headers

  FILE *f = fdopen(eio->fd, "w");

  if (f == NULL) { fgaj_r("couldn't open file back to client"); return; }

  fprintf(
    f,
    "HTTP/1.1 %i %s\r\n",
    con->res->status_code,
    shv_reason(con->res->status_code));

  char *xsf = NULL;

  shv_lower_keys(con->res->headers);
  flu_list *ths = flu_list_dtrim(con->res->headers);
  //
  for (flu_node *n = ths->first; n != NULL; n = n->next)
  {
    //printf("* %s: %s\n", n->key, (char *)n->item);

    if (strcmp(n->key, "shv_file") == 0) xsf = (char *)n->item;

    if (strncmp(n->key, "shv_", 4) == 0) continue;

    fprintf(f, "%s: %s\r\n", n->key, (char *)n->item);
  }
  flu_list_free(ths);

  fputs("\r\n", f);

  // write body

  if (xsf)
  {
    if ( ! flu_list_get(con->req->headers, "x-real-ip")) pipe_file(xsf, f);
  }
  else
  {
    for (flu_node *n = con->res->body->first; n; n = n->next)
    {
      fputs((char *)n->item, f);
    }
  }

  fclose(f);

  // done

  now = flu_gets('u');
  //
  fgaj_i(
    "i%p r%i %s %s %s %i l%s c%.3fms r%.3fms",
    eio, con->rqount,
    inet_ntoa(con->client->sin_addr),
    shv_char_to_method(con->req->method),
    con->req->uri,
    con->res->status_code,
    flu_list_get(con->res->headers, "content-length"),
    (now - con->startus) / 1000.0,
    (now - con->req->startus) / 1000.0);

  shv_con_reset(con);
}

