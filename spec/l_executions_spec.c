
//
// specifying flon-listener
//
// Tue Nov 18 16:08:03 JST 2014
//

#include <stdlib.h>

//#include "flutil.h"
//#include "djan.h"
//#include "shervin.h"
//#include "shv_protected.h"
//#include "fl_common.h"
//#include "fl_listener.h"


context "flon-listener (vs executions)"
{
  before all
  {
    chdir("../tst");
    flon_configure(".");

//    exid = flon_generate_exid("dtest.inv");
//    name = flu_sprintf("exe_%s.json", exid);
//
//    int r = flu_writeall(
//      name,
//      "{"
//        "execute: [ invoke { _0: null } [] ]\n"
//        "exid: %s\n"
//        "payload: {\n"
//          "hello: listener\n"
//        "}\n"
//      "}", exid
//    );
//    if (r != 1) { perror("failed to write exe_ file"); exit(1); }
//
//    r = flon_dispatch(name);
//    if (r != 2) { perror("failed to dispatch exe_ file"); exit(2); }
//
//    free(exid);
//    free(name);
  }

  before each
  {
    shv_request *req = NULL;
    flu_dict *params = NULL;
    shv_response *res = shv_response_malloc(404);
    fdja_value *v = NULL;
    fdja_value *v1 = NULL;
    char *exid = NULL;

    int i = system("make -C .. ctst > /dev/null");
    if (i != 0) printf("... the clean up command failed ...");
  }
  after each
  {
    if (req) shv_request_free(req);
    if (params) flu_list_free(params);
    if (v) fdja_free(v);
    if (v1) fdja_free(v1);
    if (res) shv_response_free(res);
    free(exid);
  }

  describe "flon_exes_handler() /executions"
  {
    it "lists executions (in readable domains)"
//    {
//      req = shv_parse_request_head(""
//        "POST /i/in HTTP/1.1\r\n"
//        "Host: x.flon.io\r\n"
//        "\r\n");
//      req->body = ""
//        "{\n"
//          "domain: org.example\n"
//          "execute: [ invoke, { _0: stamp }, [] ]\n"
//          "payload: {}\n"
//        "}\n";
//      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));
//
//      int r = flon_in_handler(req, res, NULL);
//
//      expect(r i== 1);
//      expect(res->status_code i== 200);
//
//      v = fdja_parse((char *)res->body->first->item);
//      if (v) v->sowner = 0; // the string is owned by the response
//
//      expect(v != NULL);
//      //flu_putf(fdja_todc(v));
//
//      expect(fdja_ls(v, "message", NULL) ===f "launched");
//
//      exid = fdja_ls(v, "exid", NULL);
//
//      expect(exid ^== "org.example-u0-");
//      expect(flu_fstat("var/spool/dis/exe_%s.json", exid) == 'f');
//
//      expect(
//        fdja_lj(
//          v, "_links.http://flon\\.io/rels\\.html#execution"
//        ) ===F fdja_vj(
//          "\"http://x.flon.io/i/executions/%s\"", exid
//        ));
//          // the answer contains a link to the new execution
//
//      expect(fdja_l(v, "tstamp") != NULL);
//
//      v1 = fdja_parse_f("var/spool/dis/exe_%s.json", exid);
//
//      expect(v1 != NULL);
//      expect(fdja_ls(v1, "exid", NULL) ===f exid);
//    }
  }
}

