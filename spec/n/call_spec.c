
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
        "    trace $(nid)\n"
        "  call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ trace: [ 0_0_0-1 ] }");
    }

    it "calls (wrapping the define in a sequence)"
    {
      exid = flon_generate_exid("n.call.vanilla");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace a\n"
        "    trace b\n"
        "  call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ trace: [ a, b ] }");
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

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ a0: egg, a1: bacon, trace: [ [ sub, lettuce ] ] }");
    }

    it "maps named arguments"
    {
      exid = flon_generate_exid("n.call.maps.named");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub a0 a1\n"
        "    trace $(v.b)\n"
        "    trace $(v.args)\n"
        "  call sub a1: egg a0: bacon cheese a2: lettuce v.b: b\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ a1: egg, a0: bacon, a2: lettuce, trace: [ b, [ sub, cheese ] ] }");
    }

    it "maps arguments to fields"

    it "it accepts URIs"
  }
}

