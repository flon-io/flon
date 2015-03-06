
//
// specifying flon
//
// Sun Jan 11 07:05:43 JST 2015
//

#include "fl_ids.h"
#include "feu_helpers.h"


context "flon and definitions:"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    fdja_value *result = NULL;
    fdja_value *v = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(result);
    fdja_free(v);
  }

  describe "referring a definition as instruction name"
  {
    it "calls it"
    {
      exid = flon_generate_exid("z.icall");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub color\n"
        "    trace '$(color) $(nid)'\n"
        "  sub yellow\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ color: yellow, trace: [ \"yellow 0_0_0-1\" ] }");
    }

    it "expands and call"
    {
      exid = flon_generate_exid("z.icall.expand");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub color\n"
        "    trace '$(color) $(nid)'\n"
        "  $(x) green\n"
        "",
        "{ x: sub }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ x: sub, color: green, trace: [ \"green 0_0_0-1\" ] }");
    }
  }
}

