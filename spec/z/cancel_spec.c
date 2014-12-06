
//
// specifying flon
//
// Sat Dec  6 10:19:55 JST 2014
//

#include "flutil.h"
#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and cancel:"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    fdja_value *res = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(res);
  }

  describe "an execution"
  {
    it "may be cancelled"
    {
      exid = flon_generate_exid("z.cancel.0");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace x\n"
        "  wait 1h\n"
        "  trace y\n"
        "",
        "{ hello: cancel.0 }");

      res = hlp_wait(exid, "launched", NULL, 2);

      flon_pp_execution(exid);

      expect(res != NULL);
      //flu_putf(fdja_todc(res));

      expect(0 == 1);
    }
  }
}

