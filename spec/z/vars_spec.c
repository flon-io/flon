
//
// specifying flon
//
// Mon Feb 16 10:07:16 JST 2015
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and vars:"
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

  describe "domain vars"
  {
    it "are visible from the executions of the domain"
    {
      exid = flon_generate_exid("z.test.domain");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace $(v.city)\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ Dover ] }");
    }

    it "are overriden by local vars"
    {
      exid = flon_generate_exid("z.test.domain");

      hlp_launch(
        exid,
        "sequence\n"
        "  set v.city: Dublin\n"
        "  trace $(v.city)\n"
        "  trace $(dv.city)\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //puts(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: Dublin, trace: [ Dublin, Dover ] }");
    }
  }
}

