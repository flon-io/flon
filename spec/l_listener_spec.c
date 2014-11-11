
//
// specifying flon-listener
//
// Tue Nov 11 13:25:47 JST 2014
//

//#include "flutil.h"
//#include "gajeta.h"
//#include "djan.h"
//#include "fl_ids.h"
//#include "fl_common.h"
#include "fl_listener.h"


context "flon-listener"
{
  before each
  {
    //fgaj_conf_get()->logger = fgaj_grey_logger;
    //fgaj_conf_get()->level = 5;
    //fgaj_conf_get()->out = stderr;
    //fgaj_conf_get()->params = "5p";

    //chdir("../tst");
    //flon_configure(".");
  }

  describe "flon_i_handler() /i"
  {
    it "lists the resources available"
  }

  describe "flon_in_handler() /in"
  {
    it "accepts launch requests"
    it "accepts cancel requests"
  }
}

