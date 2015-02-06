
//
// specifying instructions
//
// Fri Feb  6 16:01:22 JST 2015
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

  describe "else"
  {
    it "doesn't trigger when 'ret' not false"
    {
      exid = flon_generate_exid("n.else.no");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  else\n"
        "    trace b\n"
        "    trace c\n"
        "  trace d\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, d ] }");
    }

    it "triggers when 'ret' false"
    {
      exid = flon_generate_exid("n.else.yes");

      hlp_launch(
        exid,
        "sequence\n"
        "  false\n"
        "  trace a\n"
        "  else\n"
        "    trace b\n"
        "    trace c\n"
        "  trace d\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false, trace: [ a, b, c, d ] }");
    }
  }
}

