
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

#include <shervin.h>

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <ev.h>

#include "flutil.h"
#include "gajeta.h"
#include "shv_protected.h"


static void fshv_close(struct ev_loop *l, struct ev_io *eio)
{
  fgaj_sd(eio, NULL);

  fshv_con_free((fshv_con *)eio->data);

  ev_io_stop(l, eio);
  close(eio->fd);
  free(eio);
}

static void fshv_handle_cb(struct ev_loop *l, struct ev_io *eio, int revents)
{
  fgaj_sdr(eio, NULL);

  if (revents & EV_ERROR) { fgaj_r("invalid event"); return; }
  if ( ! (revents & EV_READ)) { fgaj_r("not a read"); ev_io_stop(l, eio); return; }

  if (fcntl(eio->fd, F_SETFL, fcntl(eio->fd, F_GETFL) | O_NONBLOCK) == -1)
  {
    fgaj_str(eio, "couldn't set nonblock"); fshv_close(l, eio); return;
  }
  //fgaj_t("eio->fd flags: %i", fcntl(eio->fd, F_GETFL));
  fgaj_sdr(eio, "socket nonblock set");

  fshv_con *con = (fshv_con *)eio->data;

  char buffer[FSHV_BUFFER_SIZE];

  errno = 0;
  ssize_t r = read(eio->fd, buffer, FSHV_BUFFER_SIZE);

  fgaj_sdr(eio, "read %d chars", r);

  if (r == 0)
  {
    fgaj_sdr(eio, "read eof, closing."); fshv_close(l, eio); return;
  }
  if (r < 0)
  {
    if (errno == EAGAIN || errno == EWOULDBLOCK) return;
    fgaj_sdr(eio, "read error"); fshv_close(l, eio); return;
  }

  buffer[r] = '\0';

  fgaj_st(eio, "in >>>\n%s<<< %i\n", buffer, r);

  ssize_t i = -1;
  if (con->hend < 4) for (i = 0; i < r; ++i)
  {
    if (con->hend == 4) break; // head found

    if (
      ((con->hend == 0 || con->hend == 2) && buffer[i] == '\r') ||
      ((con->hend == 1 || con->hend == 3) && buffer[i] == '\n')
    ) ++con->hend; else con->hend = 0;
  }

  fgaj_st(eio, "i %i, con->hend %i", i, con->hend);

  if (i < 0)
  {
    flu_sbwrite(con->body, buffer, r);
    con->blen += r;
  }
  else
  {
    if (con->head == NULL) con->head = flu_sbuffer_malloc();
    flu_sbwrite(con->head, buffer, i + 1);
    con->body = flu_sbuffer_malloc();
    flu_sbwrite(con->body, buffer + i, r - i);
    con->blen = r - i;
  }

  //fgaj_sd(eio, "con->blen %zu", con->blen);

  if (con->req == NULL)
  {
    if (con->hend < 4) return;
      // end of head not yet found

    char *head = flu_sbuffer_to_string(con->head);
    con->head = NULL;

    con->req = fshv_parse_request_head(head);
    con->req->startus = ev_now(l) * 1000000;
    con->rqount++;

    free(head);

    fgaj_si(eio, "%s", inet_ntoa(con->client->sin_addr));

    if (con->req->status_code != 200)
    {
      fgaj_sd(eio, "couldn't parse request head");

      con->res = fshv_response_malloc(con->req->status_code);
      fshv_respond(l, eio);
      return;
    }
  }

  //fgaj_sd(
  //  eio, "req content-length %zd", fshv_request_content_length(con->req));

  if (
    (con->req->method == 'p' || con->req->method == 'u') &&
    (con->blen < fshv_request_content_length(con->req))
  ) return; // request body not yet complete

  con->req->body = flu_sbuffer_to_string(con->body);
  con->body = NULL;

  fshv_handle(l, eio);
}

void fshv_handle(struct ev_loop *l, struct ev_io *eio)
{
  fshv_con *con = (fshv_con *)eio->data;

  con->res = fshv_response_malloc(404);

  int handled = 0;

  for (size_t i = 0; ; ++i)
  {
    fshv_route *route = con->routes[i];

    if (route == NULL) break; // end reached

    int flags = 0;
    if (handled) flags |= FSHV_F_HANDLED;

    int guarded = 0;
    //
    if (route->guard == NULL)
      guarded = 1;
    else if (handled == 0)
      guarded = route->guard(con->req, con->res, flags, route->params);
    //else if (handled == 1)
      //guarded = 0;

    if (guarded == 0) continue;

    if (route->guard == NULL) flags |= FSHV_F_NULL_GUARD;

    handled = route->handler(con->req, con->res, flags, route->params);
  }

  if (handled == 0) con->res->status_code = 404;

  fshv_respond(l, eio);
}

static void fshv_accept_cb(struct ev_loop *l, struct ev_io *eio, int revents)
{
  // remember: the eio here has the routes in its ->data

  fgaj_d("i%p d%d", eio, eio->fd);

  if (revents & EV_ERROR) { fgaj_r("invalid event"); return; }
  if ( ! (revents & EV_READ)) { fgaj_r("not a read"); ev_io_stop(l, eio); return; }

  socklen_t cal = sizeof(struct sockaddr_in);
  struct sockaddr_in *ca = calloc(1, cal); // client address

  struct ev_io *ceio = calloc(1, sizeof(struct ev_io));

  int csd = accept(eio->fd, (struct sockaddr *)ca, &cal);

  if (csd < 0) { fgaj_r("error"); return; }

  // client connected...

  fshv_con *con = fshv_con_malloc(ca, (fshv_route **)eio->data);
  con->startus = 1000 * 1000 * ev_now(l);
  ceio->data = con;

  fgaj_sd(ceio, "<- i%p d%d", eio, eio->fd);

  ev_io_init(ceio, fshv_handle_cb, csd, EV_READ);
  ev_io_start(l, ceio);
}

static ssize_t subjecter(
  char *buffer, size_t off,
  const char *file, int line, const char *func, const void *subject)
{
  size_t ooff = off;
  size_t rem = fgaj_conf_get()->subject_maxlen - off;
  int w = 0;

  if (subject)
  {
    struct ev_io *eio = (struct ev_io *)subject;
    w = snprintf(buffer + off, rem, "i%p d%i ", eio, eio->fd);
    if (w < 0) return -1; off += w; rem -= w;

    fshv_con *con = eio->data; if (con)
    {
      w = snprintf(buffer + off, rem, "c%p rq%li ", con, con->rqount);
      if (w < 0) return -1; off += w; rem -= w;

      fshv_request *req = con->req; if (req)
      {
        char *met = fshv_char_to_method(req->method);
        w = snprintf(buffer + off, rem, "%s %s ", met, req->uri);
        if (w < 0) return -1; off += w; rem -= w;
      }
    }
  }

  off += fgaj_default_subjecter(buffer, off, file, line, func, NULL);

  return off - ooff;
}

void fshv_serve(int port, fshv_route **routes)
{
  fgaj_conf_get()->subjecter = subjecter;

  int r;

  struct ev_io *eio = calloc(1, sizeof(struct ev_io));
  struct ev_loop *l = ev_default_loop(0);

  fgaj_dr("preparing");

  int sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd < 0) { fgaj_r("socket error"); exit(1); }
  fgaj_dr("prepared socket"); errno = 0;

  int v = 1; r = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
  if (r != 0) { fgaj_r("couldn't set SO_REUSEADDR"); exit(1); }
  fgaj_dr("set SO_REUSEADDR"); errno = 0;

  r = fcntl(sd, F_GETFL);
  if (r == -1) { fgaj_r("couldn't read main socket flags"); exit(1); }
  fgaj_dr("read socket flags"); errno = 0;

  r = fcntl(sd, F_SETFL, r | O_NONBLOCK);
  if (r != 0) { fgaj_r("couldn't set main socket to O_NONBLOCK"); exit(1); }
  fgaj_dr("set socket to O_NONBLOCK"); errno = 0;

  struct sockaddr_in a;
  memset(&a, 0, sizeof(struct sockaddr_in));
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = INADDR_ANY;

  r = bind(sd, (struct sockaddr *)&a, sizeof(struct sockaddr_in));
  if (r != 0) { fgaj_r("bind error"); exit(2); }
  fgaj_dr("bound");

  r = listen(sd, 2);
  if (r < 0) { fgaj_r("listen error"); exit(3); }
  fgaj_dr("listening");

  ev_io_init(eio, fshv_accept_cb, sd, EV_READ);
  eio->data = routes;
  ev_io_start(l, eio);

  fgaj_i("serving on %d...", port);

  ev_loop(l, 0);

  //fgaj_i("closing...");
  //r = close(sd);
  //if (r != 0) { fgaj_r("close error"); /*exit(4);*/ }
}

fshv_route *fshv_route_malloc(fshv_handler *guard, fshv_handler *handler, ...)
{
  va_list ap; va_start(ap, handler);
  flu_dict *params = flu_vd(ap);
  va_end(ap);

  fshv_route *r = calloc(1, sizeof(fshv_route));

  r->guard = guard;
  r->handler = handler;
  r->params = params;

  return r;
}

fshv_route *fshv_rp(char *path, fshv_handler *handler, ...)
{
  va_list ap; va_start(ap, handler);
  flu_dict *params = flu_vd(ap);
  va_end(ap);

  flu_list_set(params, "path", path);

  fshv_route *r = calloc(1, sizeof(fshv_route));

  r->guard = fshv_path_guard;
  r->handler = handler;
  r->params = params;

  return r;
}

//commit c80c5037e9f15d0e454d23cfd595b8bcc72d87a7
//Author: John Mettraux <jmettraux@gmail.com>
//Date:   Tue Jan 27 14:27:01 2015 +0900
//
//    add support for "application/pdf"
