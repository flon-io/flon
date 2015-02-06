
//
// specifying instructions
//
// Fri Feb  6 15:20:55 JST 2015
//

#include "fl_ids.h"
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

  describe "unless"
  {
    it "goes on when empty"
    {
      exid = flon_generate_exid("n.if.empty");

      hlp_launch(
        exid,
        "unless\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));
    }

    it "goes on when false and no children"
    {
      exid = flon_generate_exid("n.unless.false.no.children");

      hlp_launch(
        exid,
        "unless\n"
        "  4 > 5\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");
    }

    it "goes on when true and no children"
    {
      exid = flon_generate_exid("n.unless.true.no.children");

      hlp_launch(
        exid,
        "unless\n"
        "  4 > 3\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true }");
    }

    it "triggers the children when false"
    {
      exid = flon_generate_exid("n.unless.false");

      hlp_launch(
        exid,
        "sequence\n"
        "  unless\n"
        "    4 > 5\n"
        "    trace a\n"
        "    trace b\n"
        "  trace c\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false, trace: [ a, b, c ] }");
    }

    it "skips the children when true"
    {
      exid = flon_generate_exid("n.unless.true");

      hlp_launch(
        exid,
        "sequence\n"
        "  unless\n"
        "    5 > 4\n"
        "    trace a\n"
        "    trace b\n"
        "  trace c\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true, trace: [ c ] }");
    }
  }
}

