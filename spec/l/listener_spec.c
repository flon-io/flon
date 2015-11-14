
//
// specifying flon-listener
//
// Tue Nov 11 13:25:47 JST 2014
//

#include "flutil.h"
#include "djan.h"
#include "shervin.h"
#include "shv_protected.h"
#include "fl_common.h"
#include "fl_listener.h"
#include "l_helpers.h"


context "flon-listener"
{
  before all
  {
    chdir("../tst");
    flon_configure(".");
  }

  before each
  {
    fshv_env *env = NULL;
    fdja_value *v = NULL;
    fdja_value *v1 = NULL;
  }
  after each
  {
    fdja_free(v);
    fdja_free(v1);
    fshv_env_free(env);
  }

  describe "flon_i_handler() /i"
  {
    it "lists the resources available"
    {
      env = fshv_env_malloc_x(
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        NULL);

      int r = flon_i_handler(env);

      expect(r i== 1);
      expect(env->res->status_code i== 200);

      v = fdja_parse((char *)env->res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      //flu_putf(fdja_todc(v));

      expect(fdja_lj(v, "_links.self") ===F fdja_vj(""
        "{ href: \"http://x.flon.io/i\" }"));
      expect(fdja_lj(v, "_links.home") ===F fdja_vj(""
        "{ href: \"http://x.flon.io/i/\" }"));

      expect(fdja_l(v, "tstamp") != NULL);
    }
  }

  describe "flon_in_handler() /in"
  {
    before each
    {
      hlp_clean_tst();

      char *exid = NULL;
    }
    after each
    {
      free(exid);
    }

    context "launch"
    {
      it "accepts launch requests"
      {
        env = fshv_env_malloc_x(
          "POST /i/in HTTP/1.1\r\n"
          "Host: x.flon.io\r\n"
          "\r\n",
          NULL);
        env->req->body =
          rdz_strdup(
            "{\n"
              "domain: org.example\n"
              "point: execute\n"
              "tree: [ invoke, { _0: stamp }, [] ]\n"
              "payload: {}\n"
            "}\n");
        flu_list_set(env->bag, "_flon_user", rdz_strdup("john"));

        int r = flon_in_handler(env);

        //puts((char *)env->res->body->first->item);
        expect(r i== 1);
        expect(env->res->status_code i== 200);

        v = fdja_parse((char *)env->res->body->first->item);
        if (v) v->sowner = 0; // the string is owned by the response

        expect(v != NULL);
        //flu_putf(fdja_todc(v));

        expect(fdja_ls(v, "message", NULL) ===f "launched");

        exid = fdja_ls(v, "exid", NULL);

        expect(exid ^== "org.example-u0-");
        expect(flu_fstat("var/spool/dis/exe_%s.json", exid) == 'f');

        expect(
          fdja_lj(
            v, "_links.http://flon\\.io/rels\\.html#execution.href"
          ) ===F fdja_vj(
            "\"http://x.flon.io/i/executions/%s\"", exid
          ));
            // the answer contains a link to the new execution

        expect(fdja_l(v, "tstamp") != NULL);

        v1 = fdja_parse_f("var/spool/dis/exe_%s.json", exid);

        expect(v1 != NULL);
        expect(fdja_ls(v1, "exid", NULL) ===f exid);
      }

      it "links to self and home"
      {
        env = fshv_env_malloc_x(
          "POST /i/in HTTP/1.1\r\n"
          "Host: x.flon.io\r\n"
          "\r\n",
          NULL);
        env->req->body =
          rdz_strdup("NADA\n");

        int r = flon_in_handler(env);

        expect(r i== 1);

        v = fdja_parse((char *)env->res->body->first->item);
        if (v) v->sowner = 0; // the string is owned by the response

        expect(v != NULL);
        expect(fdja_l(v, "tstamp") != NULL);

        expect(fdja_lj(v, "_links.self") ===F fdja_vj(""
          "{ href: \"http://x.flon.io/i/in\", method: POST }"));
        expect(fdja_lj(v, "_links.home") ===F fdja_vj(""
          "{ href: \"http://x.flon.io/i/\" }"));
      }

      it "rejects invalid launch requests"
      {
        env = fshv_env_malloc_x(
          "POST /i/in HTTP/1.1\r\n"
          "Host: x.flon.io\r\n"
          "\r\n",
          rdz_strdup("NADA\n"));

        int r = flon_in_handler(env);

        expect(r i== 1);
        expect(env->res->status_code i== 400);
      }

      it "rejects launch requests for unauthorized domains"
      {
        env = fshv_env_malloc_x(
          "POST /i/in HTTP/1.1\r\n"
          "Host: x.flon.io\r\n"
          "\r\n",
          NULL);
        env->req->body =
          rdz_strdup(
            "{\n"
              "domain: org.sample\n"
              "point: execute\n"
              "tree: [ invoke, { _0: stamp }, [] ]\n"
              "payload: {}\n"
            "}\n");
        flu_list_set(env->bag, "_flon_user", rdz_strdup("john"));

        int r = flon_in_handler(env);

        expect(r i== 1);
        expect(env->res->status_code i== 403);
      }
    }

    context "cancel"
    {
      it "accepts cancel requests"
    }
  }
}

