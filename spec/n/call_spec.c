
//
// specifying flon-dispatcher
//
// Thu Jan  8 10:05:51 JST 2015
//

#include "flutil.h"
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

  describe "call"
  {
    it "calls
    {
      exid = flon_generate_exid("n.call.vanilla");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace a\n"
        "  call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload")) ===f "{ trace: [ a ] }");
    }

    it "fails if there is no corresponding define"
    {
      exid = flon_generate_exid("n.call.nosub");

      hlp_launch(
        exid,
        "call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));
    }

    it "maps arguments"
    {
      exid = flon_generate_exid("n.call.maps");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub a0 a1\n"
        "    trace $(v.args)\n"
        "  call sub egg bacon lettuce\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ a0: egg, a1: bacon, trace: [ lettuce ] }");
    }

    it "it accepts URIs"
  }
}

