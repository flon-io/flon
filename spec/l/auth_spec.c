
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

  describe "fshv_auth_filter() (was flon_auth_filter())"
  {
    before each
    {
      fshv_request *req = NULL;
      fshv_response *res = fshv_response_malloc(200);
      fdja_value *v = NULL;
      fdja_value *v1 = NULL;

      flu_dict *params =
        flu_d("func", flon_auth_enticate, "realm", "flon", NULL);
    }
    after each
    {
      fshv_request_free(req);
      fdja_free(v);
      fdja_free(v1);
      fshv_response_free(res);
      flu_list_free(params);
    }

    it "says 401 if auth fails (no authorization header)"
    {
      req = fshv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");

      int r = fshv_basic_auth_filter(req, res, 0, params);

      expect(r i== 0);

      expect(res->status_code i== 401);

      expect(flu_list_get(res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "says 401 if auth fails (wrong credentials)"
    {
      req = fshv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic nada\r\n"
        "\r\n");

      int r = fshv_basic_auth_filter(req, res, 0, params);

      expect(r i== 0);

      expect(res->status_code i== 401);

      expect(flu_list_get(res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "says 401 if ?logout"
    {
      req = fshv_parse_request_head(""
        "GET /i?logout=1 HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic am9objp3eXZlcm4=\r\n"
        "\r\n");

      int r = fshv_basic_auth_filter(req, res, 0, params);

      expect(r i== 0);

      expect(res->status_code i== 401);

      expect(flu_list_get(res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "sets _basic_user if auth succeeds"
    {
      req = fshv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic am9objp3eXZlcm4=\r\n"
        "\r\n");

      int r = fshv_basic_auth_filter(req, res, 0, params);

      expect(r i== 0);
      expect(flu_list_get(req->routing_d, "_basic_user") === "john");
    }
  }

  describe "flon_auth_enticate()"
  {
    it "returns 1 if the user and pass do match"
    {
      expect(flon_auth_enticate("john", "wyvern", NULL) i== 1);
    }

    it "returns 0 else"
    {
      expect(flon_auth_enticate("john", "wivern", NULL) i== 0);
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

