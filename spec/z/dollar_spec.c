
//
// specifying flon
//
// Wed Nov 26 15:12:04 JST 2014
//

#include "flutil.h"
#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and $(dollar):"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    fdja_value *result = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(result);
  }

  describe "an execution"
  {
    it "expands $(stuff) when calling an instruction"
    {
      exid = flon_generate_exid("z.dollar.expand");

      hlp_launch(
        exid,
        "trace $(msg)\n"
        "",
        "{ msg: \"green hornet\" }");

      result = hlp_wait(exid, "terminated", NULL, 2);

      flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_lj(result, "payload.trace") ===F fdja_vj(""
        "[ 'green hornet' ]"));
    }
  }
}

