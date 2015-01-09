
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

  describe "an instruction"
  {
    it "might expand $(stuff) when executed"
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
      //fdja_putdc(result);

      expect(fdja_lj(result, "payload.trace") ===F fdja_vj(""
        "[ 'green hornet' ]"));
    }
  }

  describe "$(exid)"
  {
    it "is expanded to the execution id"
    {
      exid = flon_generate_exid("z.dollar.exid");

      hlp_launch(
        exid,
        "trace $(exid)\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload.trace.0")) ===f exid);
    }
  }

  describe "$(nid)"
  {
    it "is expanded to the node id"
    {
      exid = flon_generate_exid("z.dollar.nid");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace $(nid)\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload.trace.0")) ===f "0_0");
    }

    it "is expanded to the correct node id (subexecution)"
    {
      exid = flon_generate_exid("z.dollar.nid");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace $(nid)\n"
        "  call sub\n"
        "  call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload.trace.0")) ===f "0_0_0-1");
      expect(fdja_tod(fdja_l(result, "payload.trace.1")) ===f "0_0_0-2");
    }
  }

  describe "$(exnid) or $(enid)"
  {
    it "is expanded to execution id + node id"
    {
      exid = flon_generate_exid("z.dollar.exnid");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace $(enid)\n"
        "  trace $(exnid)\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      char *exnid0 = flu_sprintf("%s-0_0", exid);
      expect(fdja_tod(fdja_l(result, "payload.trace.0")) ===F exnid0);

      char *exnid1 = flu_sprintf("%s-0_1", exid);
      expect(fdja_tod(fdja_l(result, "payload.trace.1")) ===F exnid1);
    }
  }
}

