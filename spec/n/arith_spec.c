
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
        exid = flon_generate_exid("n.plus.addi");

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

    describe "-"
    {
      it "substracts doubles"
      {
        exid = flon_generate_exid("n.minus.subd");

        hlp_launch(
          exid,
          "- 1000 2.0 3.0e2\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: 698.000000 }");
            // 6 zeros? :-(
      }
    }

    describe "/"
    {
      it "raises on / 0"
      {
        exid = flon_generate_exid("n.div.zeroi");

        hlp_launch(
          exid,
          "/ 7 0\n"
          "",
          "{}");

        result = hlp_wait(exid, "failed", NULL, 3);
        //flon_pp_execution(exid);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ls(result, "error.msg", NULL) ===f "division by zero");
      }

      //it "raises on / 0.0"
      it "returns infinity on / 0.0"
      {
        exid = flon_generate_exid("n.div.zerod");

        hlp_launch(
          exid,
          "/ 7.1 0.000000\n"
          "",
          "{}");

        //result = hlp_wait(exid, "failed", NULL, 3);
        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        //expect(fdja_ls(result, "error.msg", NULL) ===f "division by zero");
        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: inf }");
      }
    }
  }
}

