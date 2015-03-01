
//
// specifying flon-listener
//
// Tue Nov 18 16:08:03 JST 2014
//

#include <stdlib.h>

#include "gajeta.h"
#include "fl_ids.h"
#include "l_helpers.h"


context "flon-listener (vs executions)"
{
  before all
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");

    hlp_clean_tst();

    hlp_start_execution("org.example");
    hlp_start_execution("org.example.a");
    hlp_start_execution("org.sample.b");
    sleep(1);
    //
    //puts("---8<---");
    //system("tree var/run/");
    //puts("--->8---");
  }

  before each
  {
    char *exid = NULL;
    fshv_request *req = NULL;
    flu_dict *params = NULL;
    fshv_response *res = fshv_response_malloc(404);
    fdja_value *v = NULL;
    //fdja_value *v1 = NULL;
  }

  after each
  {
    free(exid);
    fshv_request_free(req);
    flu_list_free(params);
    fdja_free(v);
    //fdja_free(v1);
    fshv_response_free(res);
  }

  describe "flon_exes_handler() /executions"
  {
    it "lists executions (in readable domains)"
    {
      req = fshv_parse_request_head(
        "GET /i/executions HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));

      int r = flon_exes_handler(req, res, 0, NULL);

      expect(r i== 1);

      //puts((char *)res->body->first->item);

      v = fdja_parse((char *)res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      fdja_value *exes = fdja_l(v, "_embedded.executions");
      expect(fdja_size(exes) zu== 2);

      char *href0 = fdja_ls(v, "_embedded.executions.0._links.self.href");
      char *href1 = fdja_ls(v, "_embedded.executions.1._links.self.href");

      fdja_value *i0 = flon_parse_nid(href0);
      expect(i0 != NULL);
      //flu_putf(fdja_todc(i0));
      expect(fdja_ls(i0, "domain", NULL) ===f "org.example.a");
      fdja_free(i0);
      free(href0);

      fdja_value *i1 = flon_parse_nid(href1);
      expect(i1 != NULL);
      //flu_putf(fdja_todc(i1));
      expect(fdja_ls(i1, "domain", NULL) ===f "org.example");
      fdja_free(i1);
      free(href1);
    }
  }

  describe "flon_exe_handler() /executions/:domain"
  {
    it "lists the executions in a domain"
    {
      exid = hlp_lookup_exid("john", "org.example.a", 0);
      //printf("exid: %s\n", exid);

      req = fshv_parse_request_head_f(
        "GET /i/executions/org.example.a HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");
      fshv_do_route("GET /i/executions/:id", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));

      int r = flon_exe_handler(req, res, 0, NULL);

      expect(r i== 1);

      v = fdja_parse((char *)res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      expect(fdja_size(fdja_l(v, "_embedded.executions")) zu== 1);
      expect(fdja_ls(v, "_embedded.executions.0.exid", NULL) ===f exid);
    }

    it "doesn't list executions in an off-limits domain"
    {
      req = fshv_parse_request_head_f(
        "GET /i/executions/org.sample HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");
      fshv_do_route("GET /i/executions/:id", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));

      int r = flon_exe_handler(req, res, 0, NULL);

      expect(r i== 1);

      v = fdja_parse((char *)res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      expect(fdja_size(fdja_l(v, "_embedded.executions")) zu== 0);
    }
  }

  describe "flon_exe_handler() /executions/:exid"
  {
    it "details an execution"
    {
      exid = hlp_lookup_exid("john", "org.example", 0);
      //printf("exid: %s\n", exid);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));

      int r = flon_exe_handler(req, res, 0, NULL);

      expect(r i== 1);

      v = fdja_parse((char *)res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "exid", NULL) ===f exid);
      expect(fdja_ls(v, "nodes.0.inst", NULL) ===f "invoke");
      expect(fdja_ls(v, "_links.self.href", NULL) $===f exid);
    }

    it "doesn't detail off-limits domain executions"
    {
      exid = hlp_lookup_exid(NULL, "org.sample", 0);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));

      int r = flon_exe_handler(req, res, 0, NULL);

      expect(r i== 0);
    }
  }

  describe "flon_exe_sub_handler() /executions/:exid/log"
  {
    it "serves the exe.log"
    {
      exid = hlp_lookup_exid("john", "org.example", 0);
      //printf("exid: %s\n", exid);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s/log HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id/:sub", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_exe_sub_handler(req, res, 0, params);

      expect(r i== 1);
      expect(res->status_code i== 200);

      //for (flu_node *n = res->headers->first; n; n = n->next)
      //  printf("* %s: '%s'\n", n->key, (char *)n->item);

      expect(flu_list_get(res->headers, "content-type") === "text/plain");

      char *f = flu_list_get(res->headers, "fshv_file");
      expect(flu_fstat(f) == 'f');
      expect(f $=== "/exe.log");

      char *s = flu_readall(f);
      //puts("---8<---");
      //puts(s);
      //puts("--->8---");
      expect(s >== "double_fork pointed executor");

      free(s);
    }

    it "doesn't serve the exe.log from an off-limits domain execution"
    {
      exid = hlp_lookup_exid(NULL, "org.sample", 0);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s/log HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id/:sub", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_exe_sub_handler(req, res, 0, params);

      expect(r i== 0);
    }
  }

  describe "flon_exe_sub_handler() /executions/:exid/msg-log"
  {
    it "serves the msg.log"
    {
      exid = hlp_lookup_exid("john", "org.example.a", 0);
      //printf("exid: %s\n", exid);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s/msg-log HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id/:sub", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_exe_sub_handler(req, res, 0, params);

      expect(r i== 1);
      expect(res->status_code i== 200);

      //for (flu_node *n = res->headers->first; n; n = n->next)
      //  printf("* %s: '%s'\n", n->key, (char *)n->item);

      expect(flu_list_get(res->headers, "content-type") === "text/plain");

      char *f = flu_list_get(res->headers, "fshv_file");
      expect(flu_fstat(f) == 'f');
      expect(f $=== "/msg.log");

      char *s = flu_readall(f);
      //puts("---8<---");
      //puts(s);
      //puts("--->8---");
      expect(s >== "{point:execute,tree:[invoke,{_0:null},[]],");

      free(s);
    }

    it "doesn't serve the msg.log from an off-limit domain execution"
    {
      exid = hlp_lookup_exid(NULL, "org.sample", 0);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s/msg-log HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id/:sub", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_exe_sub_handler(req, res, 0, params);

      expect(r i== 0);
    }
  }

  describe "flon_exe_sub_handler() /executions/:exid/msgs"
  {
    it "lists the msgs in processed/"
    {
      exid = hlp_lookup_exid("john", "org.example", 0);
      //printf("exid: %s\n", exid);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s/msgs HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id/:sub", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_exe_sub_handler(req, res, 0, params);

      expect(r i== 1);

      v = fdja_parse((char *)res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      expect(fdja_size(fdja_l(v, "_embedded.msgs")) zu== 1);

      char *fn = flu_sprintf("http://x.flon.io/i/msgs/exe_%s.json", exid);

      expect(fdja_ls(v, "_embedded.msgs.0._links.self.href", NULL) $===F fn);
    }

    it "doesn't list msgs from an execution in an off-limits domain"
    {
      exid = hlp_lookup_exid(NULL, "org.sample", 0);

      req = fshv_parse_request_head_f(
        "GET /i/executions/%s/msgs HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        exid);
      fshv_do_route("GET /i/executions/:id/:sub", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_exe_sub_handler(req, res, 0, params);

      expect(r i== 0);
    }
  }

  describe "flon_msg_handler() /msgs/:id"
  {
    it "details a processed msg"
    {
      exid = hlp_lookup_exid("john", "org.example", 0);
      char *mid = flu_sprintf("exe_%s.json", exid);

      req = fshv_parse_request_head_f(
        "GET /i/msgs/%s HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        mid);
      fshv_do_route("GET /i/msgs/:id", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_msg_handler(req, res, 0, params);

      expect(r i== 1);

      //for (flu_node *n = res->headers->first; n; n = n->next)
      //  printf("* %s: '%s'\n", n->key, (char *)n->item);

      expect(flu_list_get(res->headers, "content-type") === ""
        //"application/json; charset=utf-8"); // TODO
        "application/json");

      char *f = flu_list_get(res->headers, "fshv_file");
      expect(flu_fstat(f) == 'f');
      expect(f $=== mid);

      v = fdja_parse_f(f);
      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "point", NULL) ===f ""
        "execute");
      expect(fdja_lj(v, "tree") ===F fdja_vj(""
        "[ invoke, { _0: \"null\" }, [] ]"));

      free(mid);
    }

    it "doesn't serve msgs from off-limits domains"
    {
      char *mid =
        flu_pline("find var/ -name processed | grep sample | xargs ls");

      //puts(mid);
      expect(mid $=== ".json");

      req = fshv_parse_request_head_f(
        "GET /i/msgs/%s HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        mid);
      fshv_do_route("GET /i/msgs/:id", req);
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
      params = flu_list_malloc();

      int r = flon_msg_handler(req, res, 0, params);

      expect(r i== 0);

      free(mid);
    }
  }
}

