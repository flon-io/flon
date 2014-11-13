
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

  describe "flon_auth_filter()"
  {
    before each
    {
      shv_request *req = NULL;
      flu_dict *params = NULL;
      shv_response *res = shv_response_malloc(200);
      fdja_value *v = NULL;
      fdja_value *v1 = NULL;
    }
    after each
    {
      if (req) shv_request_free(req);
      if (params) flu_list_free(params);
      if (v) fdja_free(v);
      if (v1) fdja_free(v1);
      if (res) shv_response_free(res);
    }

    it "returns 1 and 401 if auth fails (no authorization header)"
    {
      req = shv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");

      int r = flon_auth_filter(req, res, NULL);

      expect(r i== 1);

      expect(res->status_code i== 401);

      expect(flu_list_get(res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "returns 1 and 401 if auth fails (wrong credentials)"
    {
      req = shv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic nada\r\n"
        "\r\n");

      int r = flon_auth_filter(req, res, NULL);

      expect(r i== 1);

      expect(res->status_code i== 401);

      expect(flu_list_get(res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "returns 1 and 401 if ?logout"
    {
      req = shv_parse_request_head(""
        "GET /i?logout=1 HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic am9objp3eXZlcm4=\r\n"
        "\r\n");

      int r = flon_auth_filter(req, res, NULL);

      expect(r i== 1);

      expect(res->status_code i== 401);

      expect(flu_list_get(res->headers, "WWW-Authenticate") === ""
        "Basic realm=\"flon\"");
    }

    it "returns 0 if auth succeeds"
    {
      req = shv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "Authorization: Basic am9objp3eXZlcm4=\r\n"
        "\r\n");

      int r = flon_auth_filter(req, res, NULL);

      expect(r i== 0);
      expect(flu_list_get(req->routing_d, "_user") === "john");
    }
  }

  describe "flon_auth_enticate()"
  {
    it "returns 1 if the user and pass do match"
    {
      expect(flon_auth_enticate("john", "wyvern") i== 1);
    }

    it "returns 0 else"
    {
      expect(flon_auth_enticate("john", "wivern") i== 0);
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
}

