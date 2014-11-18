
//
// specifying flon-listener
//
// Tue Nov 18 16:08:03 JST 2014
//

#include <stdlib.h>

#include "gajeta.h"
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
    shv_request *req = NULL;
    flu_dict *params = NULL;
    shv_response *res = shv_response_malloc(404);
    fdja_value *v = NULL;
    //fdja_value *v1 = NULL;
  }

  after each
  {
    if (req) shv_request_free(req);
    if (params) flu_list_free(params);
    if (v) fdja_free(v);
    //if (v1) fdja_free(v1);
    if (res) shv_response_free(res);
  }

  describe "flon_exes_handler() /executions"
  {
    it "lists executions (in readable domains)"
    {

      req = shv_parse_request_head(""
        "GET /i/executions HTTP/1.1\r\n"
        "Host: x.flon.io\r\n"
        "\r\n");
      flu_list_set(req->routing_d, "_user", rdz_strdup("john"));

      int r = flon_exes_handler(req, res, NULL);

      expect(r i== 1);

      //puts((char *)res->body->first->item);

      v = fdja_parse((char *)res->body->first->item);
      if (v) v->sowner = 0; // the string is owned by the response

      expect(v != NULL);
      flu_putf(fdja_todc(v));

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
}

