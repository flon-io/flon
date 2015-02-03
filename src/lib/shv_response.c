
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

#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ev.h>

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "shervin.h"
#include "shv_protected.h"


//
// fshv_response

fshv_response *fshv_response_malloc(short status_code)
{
  fshv_response *r = calloc(1, sizeof(fshv_response));
  r->status_code = status_code;
  r->headers = flu_list_malloc();
  r->body = flu_list_malloc();

  return r;
}

void fshv_response_free(fshv_response *r)
{
  if (r == NULL) return;

  flu_list_free_all(r->headers);
  flu_list_free_all(r->body);
  free(r);
}

//
// fshv_respond

static char *fshv_reason(short status_code)
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

static char *fshv_low_reason(short status_code)
{
  char *s = fshv_reason(status_code);
  char *r = calloc(strlen(s) + 1, sizeof(char));
  for (char *rr = r; *s; ++rr, ++s) *rr = tolower(*s);

  return r;
}

static void fshv_lower_keys(flu_dict *d)
{
  for (flu_node *n = d->first; n != NULL; n = n->next)
  {
    for (char *s = n->key; *s; ++s) *s = tolower(*s);
  }
}

static void fshv_set_content_length(fshv_con *con)
{
  char *s = flu_list_get(con->res->headers, "fshv_content_length");

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

static void fshv_respond_cb(struct ev_loop *l, struct ev_io *eio, int revents)
{
  if (revents & EV_ERROR) { fgaj_r("invalid event"); return; }
  if ( ! (revents & EV_WRITE)) { fgaj_r("not a read"); ev_io_stop(l, eio); return; }

  fshv_con *con = (fshv_con *)eio->data;

  fgaj_sd(
    eio,
    "hout %p bout %p", con ? con->hout : NULL, con ? con->bout : NULL);

  if (con->hout)
  {
    while (1)
    {
      errno = 0;

      size_t l = con->houtlen - con->houtoff;
      if (l == 0) break;
      if (l > FSHV_BUFFER_SIZE) l = FSHV_BUFFER_SIZE;
      ssize_t w = write(eio->fd, con->hout + con->houtoff, l);

      fgaj_sdr(eio, "hout wrote %d / %zu / %zu chars", w, l, con->houtlen);

      if (w < 0) { fgaj_sr(eio, "failed to write header"); return; }
        // TODO: should I give up here???

      con->houtoff += w;

      if (w < l) return;
      //if (con->houtoff < con->houtlen) break;
    }

    free(con->hout); con->hout = NULL;
  }

  if (con->bout)
  {
    char buf[FSHV_BUFFER_SIZE];

    while (1)
    {
      errno = 0;
      size_t r = fread(buf, sizeof(char), FSHV_BUFFER_SIZE, con->bout);
      if (r == 0) break;

      for (size_t off = 0; off < r; )
      {
        //fgaj_sdr(eio, "pre-write");
        errno = 0;
        ssize_t w = write(eio->fd, (char *)buf + off, r - off);
        fgaj_sdr(eio, "bout wrote %d / %d / %d", w, off, r);
        if (w < 0)
        {
          if (errno != 0 && errno != EAGAIN && errno != EWOULDBLOCK)
            fgaj_sr(eio, "write error while sending file");
          if (fseek(con->bout, off - r, SEEK_CUR) != 0)
            fgaj_sr(eio, "fseek failed SEEK_CUR %d", off - r);
          return;
        }
        off += w;
      }
    }

    fclose(con->bout); con->bout = NULL;
  }

  // done

  char asrc = 'i'; char *addr = flu_list_get(con->req->headers, "x-real-ip");
  if (addr == NULL)
  {
    asrc = 'f'; addr = flu_list_get(con->req->headers, "x-forwarded-for");
  }
  if (addr == NULL)
  {
    asrc = 'a'; addr = inet_ntoa(con->client->sin_addr);
  }

  long long nowus = flu_gets('u');

  fgaj_si(
    eio,
    "%c%s %i l%s c%.3fms r%.3fms done.",
    asrc, addr,
    con->res->status_code,
    flu_list_get(con->res->headers, "content-length"),
    (nowus - con->startus) / 1000.0,
    (nowus - con->req->startus) / 1000.0);

  ev_io_stop(l, eio); fgaj_sd(eio, "ev_io_stop() (w)"); free(eio);
  fshv_con_reset(con);
}

static int prepare_response(fshv_con *con)
{
  con->hout = NULL;
  con->houtlen = 0;
  con->houtoff = 0;

  con->bout = NULL;

  FILE *fout = open_memstream(&con->hout, &con->houtlen);

  if (fout == NULL) { fgaj_r("couldn't prepare response"); return 1; }

  int r = 0;
  flu_list *ths = NULL;

  r = fprintf(
    fout,
    "HTTP/1.1 %i %s\r\n",
    con->res->status_code,
    fshv_reason(con->res->status_code));
  if (r < 0) goto _over;

  char *xsf = NULL;

  fshv_lower_keys(con->res->headers);
  ths = flu_list_dtrim(con->res->headers);
  //
  for (flu_node *n = ths->first; n != NULL; n = n->next)
  {
    //printf("* %s: %s\n", n->key, (char *)n->item);

    if (strcmp(n->key, "fshv_file") == 0) xsf = (char *)n->item;

    if (strncmp(n->key, "fshv_", 4) == 0) continue;

    r = fprintf(fout, "%s: %s\r\n", n->key, (char *)n->item);
    if (r < 0) goto _over;
  }

  r = fputs("\r\n", fout);
  if (r < 0) goto _over;

  if ( ! xsf)
  {
    for (flu_node *n = con->res->body->first; n; n = n->next)
    {
      r = fputs((char *)n->item, fout);
      if (r < 0) goto _over;
    }
  }

  r = fclose(fout);
  if (r != 0) { r = -1; goto _over; }

  if (xsf && ! flu_list_get(con->req->headers, "x-real-ip"))
  {
    con->bout = fopen(xsf, "r");
    //if (con->bout == NULL) { r = -1; goto _over; }
    if (con->bout == NULL) r = -1;
  }

_over:

  flu_list_free(ths);

  if (r < 0) fgaj_r("problem preparing response");

  return r > -1 ? 0 : r;
}

void fshv_respond(struct ev_loop *l, struct ev_io *eio)
{
  fshv_con *con = (fshv_con *)eio->data;

  // prepare headers

  flu_list_set(
    con->res->headers, "date", flu_sstamp(l ? ev_now(l) : 0, 1, 'r'));

  flu_list_set_last(
    con->res->headers, "server", flu_sprintf("shervin %s", FSHV_VERSION));
  flu_list_set_last(
    con->res->headers, "content-type", strdup("text/plain; charset=utf-8"));

  if (flu_list_get(con->res->headers, "location") == NULL)
  {
    flu_list_set(
      con->res->headers, "location", strdup("northpole")); // FIXME
  }

  //long long nowus = l ? ev_now(l) * 1000000 : flu_gets('u');
  long long nowus = flu_gets('u');
  //
  flu_list_set(
    con->res->headers,
    "x-flon-shervin",
    flu_sprintf(
      "c%.3fms;r%.3fms;rq%i",
      (nowus - con->startus) / 1000.0,
      (nowus - con->req->startus) / 1000.0,
      con->rqount));

  if (
    con->res->body->size == 0 &&
    con->res->status_code >= 400 &&
    con->res->status_code < 600 &&
    flu_list_get(con->res->headers, "fshv_content_length") == NULL)
  {
    // set 4xx or 5xx reason as body
    flu_list_add(con->res->body, fshv_low_reason(con->res->status_code));
  }

  fshv_set_content_length(con);

  // write to eio->fd (if there is one)

  if (l == NULL) return; // only in spec/handle_spec.c

  // prepare response

  prepare_response(con);

  // respond

  struct ev_io *weio = calloc(1, sizeof(struct ev_io));
  weio->data = con;

  //ev_io_stop(l, eio); fgaj_sd(eio, "ev_io_stop() (r)");
    // NO, because this eio may be re-used :-)

  ev_io_init(weio, fshv_respond_cb, eio->fd, EV_WRITE);
  ev_io_start(l, weio);
}

//commit c80c5037e9f15d0e454d23cfd595b8bcc72d87a7
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Tue Jan 27 14:27:01 2015 +0900
//
//    add support for "application/pdf"
