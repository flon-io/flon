
//
// specifying flon-listener
//
// Wed Nov 19 05:47:33 JST 2014
//

#include <stdlib.h>

#include "gajeta.h"
//#include "l_helpers.h"


context "flon-listener (vs metrics)"
{
  before all
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");

    //hlp_clean_tst();

    //hlp_start_execution("org.example");
    //hlp_start_execution("org.example.a");
    //hlp_start_execution("org.sample.b");
    //sleep(1);
  }

  before each
  {
    fshv_request *req = NULL;
    flu_dict *params = NULL;
    fshv_response *res = fshv_response_malloc(404);
  }

  after each
  {
    fshv_request_free(req);
    flu_list_free(params);
    fshv_response_free(res);
  }

  describe "flon_metrics_handler() /metrics"
  {
    it "details some metrics about the feu"
  }
}

