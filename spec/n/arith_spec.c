
//
// specifying instructions
//
// Sun Feb  8 06:13:35 JST 2015
//

#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "instruction:"
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

  context "arith:"
  {

    describe "+"
    {
      it "adds integers"
      {
        exid = flon_generate_exid("n.plus.adds");

        hlp_launch(
          exid,
          "+ 1 2 3\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: 6 }");
      }
    }

    describe "/"
    {
      it "raises on / 0"
      it "raises on / 0.0"
    }
  }
}

