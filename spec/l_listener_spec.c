
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

      v = fdja_parse((char *)res->body->first->item);

      expect(v != NULL);

      v->sowner = 0; // the string is owned by the response

      expect(fdja_lj(v, "_links.self") ===F fdja_vj(""
        "{ href: \"http://x.flon.io/i\" }"));
    }
  }

  describe "flon_in_handler() /in"
  {
    it "accepts launch requests"
    it "accepts cancel requests"
  }
}

