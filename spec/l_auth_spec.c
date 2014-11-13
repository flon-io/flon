
//
// specifying flon-listener
//
// Wed Nov 12 15:42:05 JST 2014
//

#include "fl_common.h"
#include "fl_listener.h"


context "flon-listener auth"
{
  before all
  {
    chdir("../tst");
    flon_configure(".");
  }

  //before each
  //{
  //  shv_request *req = NULL;
  //  flu_dict *params = NULL;
  //  shv_response *res = shv_response_malloc(200);
  //  fdja_value *v = NULL;
  //  fdja_value *v1 = NULL;
  //}
  //after each
  //{
  //  if (req) shv_request_free(req);
  //  if (params) flu_list_free(params);
  //  if (v) fdja_free(v);
  //  if (v1) fdja_free(v1);
  //  if (res) shv_response_free(res);
  //}

  describe "flon_auth_filter()"
  {
    it "flips burgers"

    //it "lists the resources available"
    //{
    //  req = shv_parse_request_head(""
    //    "GET /i HTTP/1.1\r\n"
    //    "Host: x.flon.io\r\n"
    //    "\r\n");

    //  int r = flon_i_handler(req, res, NULL);

    //  expect(r i== 1);
    //  expect(res->status_code i== 200);

    //  v = fdja_parse((char *)res->body->first->item);

    //  expect(v != NULL);

    //  v->sowner = 0; // the string is owned by the response

    //  //flu_putf(fdja_todc(v));

    //  expect(fdja_lj(v, "_links.self") ===F fdja_vj(""
    //    "{ href: \"http://x.flon.io/i\" }"));
    //}
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
  }
}

