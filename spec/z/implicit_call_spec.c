
//
// specifying flon
//
// Sun Jan 11 07:05:43 JST 2015
//

#include "fl_ids.h"
#include "fl_tools.h"
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
      exid = flon_generate_exid("z.implicit_call");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub color\n"
        "    trace '$(color) $(nid)'\n"
        "  sub yellow\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      flon_pp_execution(exid);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ color: yellow, trace: [ \"yellow 0_0_0-1\" ] }");
    }
  }
}

