
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


static void shv_close(struct ev_loop *l, struct ev_io *eio)
{
  shv_con_free((shv_con *)eio->data);

  ev_io_stop(l, eio);
  close(eio->fd);
  //fgaj_d(reason, eio);
  free(eio);
}

static void shv_handle_cb(struct ev_loop *l, struct ev_io *eio, int revents)
{
  if (EV_ERROR & revents) { fgaj_r("invalid event"); return; }

  if (fcntl(eio->fd, F_SETFL, fcntl(eio->fd, F_GETFL) | O_NONBLOCK) == -1)
  {
    fgaj_tr("couldn't set nonblock (i%p fd %i)", eio, eio->fd);
    shv_close(l, eio);
    return;
  }
  //fgaj_t("eio->fd flags: %i", fcntl(eio->fd, F_GETFL));

  shv_con *con = (shv_con *)eio->data;

  char buffer[SHV_BUFFER_SIZE + 1];

  ssize_t r = recv(eio->fd, buffer, SHV_BUFFER_SIZE, 0);

  fgaj_d("read: %li (i%p fd %i)", r, eio, eio->fd);

  if (r < 0 && errno == EAGAIN) return;

  else if (r <= 0) {
    if (r < 0) fgaj_r("read error (i%p fd %i)", eio, eio->fd);
    shv_close(l, eio);
    return;
  }

  buffer[r] = '\0';

  fgaj_t("i%p r%i in >>>\n%s<<< %i\n", eio, con->rqount, buffer, r);

  ssize_t i = -1;
  if (con->hend < 4) for (i = 0; i < r; ++i)
  {
    if (con->hend == 4) break; // head found

    if (
      ((con->hend == 0 || con->hend == 2) && buffer[i] == '\r') ||
      ((con->hend == 1 || con->hend == 3) && buffer[i] == '\n')
    ) ++con->hend; else con->hend = 0;
  }

  fgaj_t("i%p r%i i%i, con->hend %i", eio, con->rqount, i, con->hend);

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

  //printf("c%p con->blen %zu\n", eio, con->blen);

  if (con->req == NULL)
  {
    if (con->hend < 4) return;
      // end of head not yet found

    char *head = flu_sbuffer_to_string(con->head);
    con->head = NULL;

    con->req = shv_parse_request_head(head);
    con->rqount++;

    free(head);

    fgaj_i(
      "i%p r%i %s %s %s",
      eio, con->rqount,
      inet_ntoa(con->client->sin_addr),
      shv_char_to_method(con->req->method),
      con->req->uri);

    if (con->req->status_code != 200)
    {
      fgaj_d("c%p r%i couldn't parse request head", eio, con->rqount);

      con->res = shv_response_malloc(con->req->status_code);
      shv_respond(l, eio);
      return;
    }
  }

  //printf(
  //  "con->req content-length %zd\n", shv_request_content_length(con->req));

  if (
    (con->req->method == 'p' || con->req->method == 'u') &&
    (con->blen < shv_request_content_length(con->req))
  ) return; // request body not yet complete

  con->req->body = flu_sbuffer_to_string(con->body);
  con->body = NULL;

  shv_handle(l, eio);
}

void shv_handle(struct ev_loop *l, struct ev_io *eio)
{
  shv_con *con = (shv_con *)eio->data;

  con->res = shv_response_malloc(404);

  int filtering = 0;
  int guarded = 0;
  int handled = 0;

  for (size_t i = 0; ; ++i)
  {
    shv_route *route = con->routes[i];

    if (route == NULL) break; // end reached

    if ((void *)route->guard == (void *)shv_filter_guard) filtering = 1;

    if (handled && ! filtering) continue;

    if (route->guard && (void *)route->guard != (void *)shv_filter_guard)
    {
      filtering = 0;

      if (handled) continue;

      guarded = route->guard(con->req, con->res, route->params);
    }

    if ( ! (filtering || guarded)) continue;

    if (route->handler)
    {
      int h = route->handler(con->req, con->res, route->params);
      if (filtering != 1 && guarded != -1) handled = h;
    }
  }

  shv_respond(l, eio);
}

static void shv_accept_cb(struct ev_loop *l, struct ev_io *eio, int revents)
{
  socklen_t cal = sizeof(struct sockaddr_in);
  struct sockaddr_in *ca = calloc(1, cal); // client address

  struct ev_io *ceio = calloc(1, sizeof(struct ev_io));

  if (EV_ERROR & revents) { fgaj_r("invalid event"); return; }

  int csd = accept(eio->fd, (struct sockaddr *)ca, &cal);

  if (csd < 0) { fgaj_r("error"); return; }

  // client connected...

  ceio->data = shv_con_malloc(ca, (shv_route **)eio->data);

  ev_io_init(ceio, shv_handle_cb, csd, EV_READ);
  ev_io_start(l, ceio);
}

void shv_serve(int port, shv_route **routes)
{
  int r;

  struct ev_io *eio = calloc(1, sizeof(struct ev_io));
  struct ev_loop *l = ev_default_loop(0);

  int sd = socket(PF_INET, SOCK_STREAM, 0);
  if (sd < 0) { fgaj_r("socket error"); exit(1); }

  int v = 1; r = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
  if (r != 0) { fgaj_r("couldn't set SO_REUSEADDR"); exit(1); }

  r = fcntl(sd, F_GETFL);
  if (r == -1) { fgaj_r("couldn't read main socket flags"); exit(1); }

  r = fcntl(sd, F_SETFL, r | O_NONBLOCK);
  if (r != 0) { fgaj_r("couldn't set main socket to O_NONBLOCK"); exit(1); }

  struct sockaddr_in a;
  memset(&a, 0, sizeof(struct sockaddr_in));
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = INADDR_ANY;

  r = bind(sd, (struct sockaddr *)&a, sizeof(struct sockaddr_in));
  if (r != 0) { fgaj_r("bind error"); exit(2); }

  r = listen(sd, 2);
  if (r < 0) { fgaj_r("listen error"); exit(3); }

  ev_io_init(eio, shv_accept_cb, sd, EV_READ);
  eio->data = routes;
  ev_io_start(l, eio);

  fgaj_i("serving on %d...", port);

  ev_loop(l, 0);

  //fgaj_i("closing...");
  //r = close(sd);
  //if (r != 0) { fgaj_r("close error"); /*exit(4);*/ }
}

shv_route *shv_route_malloc(shv_handler *guard, shv_handler *handler, ...)
{
  va_list ap; va_start(ap, handler);
  flu_dict *params = flu_vd(ap);
  va_end(ap);

  shv_route *r = calloc(1, sizeof(shv_route));

  r->guard = guard;
  r->handler = handler;
  r->params = params;

  return r;
}

shv_route *shv_rp(char *path, shv_handler *handler, ...)
{
  va_list ap; va_start(ap, handler);
  flu_dict *params = flu_vd(ap);
  va_end(ap);

  flu_list_set(params, "path", path);

  shv_route *r = calloc(1, sizeof(shv_route));

  r->guard = shv_path_guard;
  r->handler = handler;
  r->params = params;

  return r;
}

