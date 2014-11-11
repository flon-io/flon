
//
// specifying flon-listener
//
// Tue Nov 11 13:25:47 JST 2014
//

#include "djan.h"
#include "shervin.h"
#include "shv_protected.h"
#include "fl_listener.h"


context "flon-listener"
{
  before all
  {
    chdir("../tst");
  }

  before each
  {
    shv_request *req = NULL;
    flu_dict *params = NULL;
    shv_response *res = shv_response_malloc(200);
    fdja_value *v = NULL;
  }
  after each
  {
    if (req) shv_request_free(req);
    if (params) flu_list_free(params);
    if (v) fdja_free(v);
    if (res) shv_response_free(res);
  }

  describe "flon_i_handler() /i"
  {
    it "lists the resources available"
    {
      req = shv_parse_request_head(""
        "GET /i HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");

      int r = flon_i_handler(req, res, NULL);

      expect(r i== 1);
      expect(res->status_code i== 200);

      v = fdja_parse((char *)res->body->first->item);

      expect(v != NULL);

      v->sowner = 0; // the string is owned by the response

      expect(fdja_lj(v, "_links.self") ===F fdja_vj(""
        "{ href: \"http://x.flon.io/i\" }"));
    }
  }

  describe "flon_in_handler() /in"
  {
    before each
    {
      int i = system("make -C .. ctst > /dev/null");
      if (i != 0) printf("... the clean up command failed ...");
    }

    context "launch"
    {
      it "accepts launch requests"
      {
        req = shv_parse_request_head(""
          "POST /i/in HTTP/1.1\r\n"
          "Host: x.flon.io\r\n"
          "\r\n");
        req->body = ""
          "{\n"
            "execute: [ invoke, { _0: stamp }, [] ]\n"
            "payload: {}\n"
          "}\n";

        int r = flon_in_handler(req, res, NULL);

        expect(r i== 1);
        expect(res->status_code i== 200);
      }

      it "rejects invalid launch requests"
      {
        req = shv_parse_request_head(""
          "POST /i/in HTTP/1.1\r\n"
          "Host: x.flon.io\r\n"
          "\r\n");
        req->body = "NADA\n";

        int r = flon_in_handler(req, res, NULL);

        expect(r i== 1);
        expect(res->status_code i== 400);
      }
    }

    context "cancel"
    {
      it "accepts cancel requests"
    }
  }
}

