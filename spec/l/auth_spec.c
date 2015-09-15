
//
// specifying flon-listener
//
// Wed Nov 12 15:42:05 JST 2014
//

#include "tsifro.h"
#include "shervin.h"
#include "shv_protected.h"
#include "fl_common.h"
#include "fl_listener.h"


context "flon-listener auth"
{
  before all
  {
    chdir("../tst");
    flon_configure(".");
  }

  describe "fshv_basic_auth()"
  {
    before each
    {
      fshv_env *env = NULL;
    }
    after each
    {
      fshv_env_free(env);
    }

    it "says 401 if auth fails (no authorization header)"
    {
      env = fshv_env_malloc(
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n",
        NULL);

      int r = fshv_basic_auth(env, "flon", flon_auth_enticate);

      expect(r i== 0);

      expect(env->res->status_code i== 401);

      expect(flu_list_get(env->res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "says 401 if auth fails (wrong credentials)"
    {
      env = fshv_env_malloc(
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic nada\r\n"
        "\r\n",
        NULL);

      int r = fshv_basic_auth(env, "flon", flon_auth_enticate);

      expect(r i== 0);

      expect(env->res->status_code i== 401);

      expect(flu_list_get(env->res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "says 401 if ?logout"
    {
      env = fshv_env_malloc(
        "GET /i?logout=1 HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic am9objp3eXZlcm4=\r\n"
        "\r\n",
        NULL);

      int r = fshv_basic_auth(env, "flon", flon_auth_enticate);

      expect(r i== 0);

      expect(env->res->status_code i== 401);

      expect(flu_list_get(env->res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "sets _basic_user if auth succeeds"
    {
      env = fshv_env_malloc(
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic am9objp3eXZlcm4=\r\n"
        "\r\n",
        NULL);

      int r = fshv_basic_auth(env, "flon", flon_auth_enticate);

      expect(r i== 0);
      expect(flu_list_get(env->bag, "_flon_user") === "john");
    }
  }

  describe "flon_auth_enticate()"
  {
    it "returns a copy of the username if the user and pass do match"
    {
      expect(flon_auth_enticate(NULL, "flon", "john", "wyvern") ===f "john");
    }

    it "returns NULL else"
    {
      expect(flon_auth_enticate(NULL, "flon", "john", "wivern") == NULL);
    }

    it "verifies 'wyvern'"
    {
      expect(
        ftsi_bc_verify(
          "wyvern",
          "$2a$07$hi1ZGrPNfHAX/a5p0oTu0edFxY3aCZBK.NfIx9RrIEPWaiidMV8ty"
        ) i== 1);
    }
  }

  describe "flon_dom_matches()"
  {
    it "returns 1 if the domain matches the pattern"
    {
      expect(flon_dom_matches("org.exa", "org.exa.**") i== 1);
      expect(flon_dom_matches("org.exa.a", "org.exa.*") i== 1);
      expect(flon_dom_matches("org.exa.a", "org.exa.**") i== 1);
      expect(flon_dom_matches("org.exa.b", "org.exa.**.b") i== 1);
      expect(flon_dom_matches("org.exa.a.b", "org.exa.**.b") i== 1);
      expect(flon_dom_matches("org.exa.a.a.b", "org.exa.**.b") i== 1);
      expect(flon_dom_matches("o.e.a.a.b.c", "o.e.**.b.*") i== 1);
    }

    it "returns 0 when it doesn't match"
    {
      expect(flon_dom_matches("org.exa.a", "org.exa") i== 0);
      expect(flon_dom_matches("org.exa.a.b", "org.exa.**.c") i== 0);
      expect(flon_dom_matches("o.e.a.a.b", "o.e.**.b.*") i== 0);
    }
  }
}

