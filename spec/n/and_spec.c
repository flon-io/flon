
//
// specifying flon-dispatcher
//
// Mon Jan 19 14:01:07 JST 2015
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

  describe "and"
  {
    it "ands (hit)"
    {
      exid = flon_generate_exid("n.ao.and.0");

      hlp_launch(
        exid,
        "and\n"
        "  val true\n"
        "  val true\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true }");
    }

    it "ands (miss)"
    {
      exid = flon_generate_exid("n.ao.and.1");

      hlp_launch(
        exid,
        "and\n"
        "  val true\n"
        "  val false\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");
    }
  }

  describe "or"
  {
    it "ors (hit)"
    {
      exid = flon_generate_exid("n.ao.or.0");

      hlp_launch(
        exid,
        "or\n"
        "  val false\n"
        "  val true\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true }");
    }

    it "ors (miss)"
    {
      exid = flon_generate_exid("n.ao.or.1");

      hlp_launch(
        exid,
        "or\n"
        "  val false\n"
        "  val false\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");
    }
  }
}

