
//
// specifying instructions
//
// Fri Feb  6 14:29:15 JST 2015
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

  describe "if"
  {
    it "goes on when empty"
    {
      exid = flon_generate_exid("n.if.empty");

      hlp_launch(
        exid,
        "if\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));
    }

    it "goes on when true and no then children"
    {
      exid = flon_generate_exid("n.if.true.no.children");

      hlp_launch(
        exid,
        "if\n"
        "  5 > 4\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true }");
    }

    it "goes on when false and no children"
    {
      exid = flon_generate_exid("n.if.false.no.children");

      hlp_launch(
        exid,
        "if\n"
        "  3 > 4\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");
    }

    it "triggers the children when true"
    {
      exid = flon_generate_exid("n.if.true");

      hlp_launch(
        exid,
        "sequence\n"
        "  if\n"
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
        "{ ret: true, trace: [ a, b, c ] }");
    }

    it "skips the children when false"
    {
      exid = flon_generate_exid("n.if.false");

      hlp_launch(
        exid,
        "sequence\n"
        "  if\n"
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
        "{ ret: false, trace: [ c ] }");
    }
  }
}

