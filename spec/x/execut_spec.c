
//
// specifying flon-executor
//
// Thu Feb  5 15:21:13 JST 2015
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"
#include "fl_common.h"
#include "fl_executor.h"


context "flon-executor"
{
  before each
  {
    fdja_value *msg = NULL;

    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }
  after each
  {
    fdja_free(msg);
  }

  describe "flon_execut()"
  {
    it "executes transiently"
    {
      fdja_value *tree = fdja_parse_radial(
        rdz_strdup(""
          "42"),
        "sfeu");
      fdja_value *payload = fdja_o(NULL);
      fdja_value *vars = fdja_o(NULL);

      msg = flon_execut("x.test", tree, payload, vars);

      //fdja_putdc(msg);
      expect(fdja_li(msg, "payload.ret", -1) lli== 42);
    }
  }
}

