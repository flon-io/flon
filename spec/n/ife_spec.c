
//
// specifying instructions
//
// Thu Jan 22 05:40:21 JST 2015
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

  describe "ife"
  {
    it "goes on when empty"
    {
      exid = flon_generate_exid("n.ife.empty");

      hlp_launch(
        exid,
        "ife\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));
    }

    it "goes on when true and no then branch"
    {
      exid = flon_generate_exid("n.ife.then.no.then");

      hlp_launch(
        exid,
        "ife\n"
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

    it "goes on when false and no else branch"
    {
      exid = flon_generate_exid("n.ife.else.no.else");

      hlp_launch(
        exid,
        "ife\n"
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

    it "triggers the then branch"
    {
      exid = flon_generate_exid("n.ife.then");

      hlp_launch(
        exid,
        "ife\n"
        "  5 > 4\n"
        "  'then'\n"
        "  'else'\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: then }");
    }

    it "triggers the else branch"
    {
      exid = flon_generate_exid("n.ife.else");

      hlp_launch(
        exid,
        "ife\n"
        "  3 > 4\n"
        "  'then'\n"
        "  'else'\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: else }");
    }
  }
}

