
//
// specifying instructions
//
// Fri Feb  6 13:51:17 JST 2015
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

  describe "unlesse"
  {
    it "goes on when empty"
    {
      exid = flon_generate_exid("n.unlesse.empty");

      hlp_launch(
        exid,
        "unlesse\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{}");
    }

    it "goes on when false and no then branch"
    {
      exid = flon_generate_exid("n.unlesse.then.no.then");

      hlp_launch(
        exid,
        "unlesse\n"
        "  4 > 5\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true }");
    }

    it "goes on when true and no else branch"
    {
      exid = flon_generate_exid("n.unlesse.else.no.else");

      hlp_launch(
        exid,
        "unlesse\n"
        "  4 > 3\n"
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
      exid = flon_generate_exid("n.unlesse.then");

      hlp_launch(
        exid,
        "unlesse\n"
        "  4 > 5\n"
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
      exid = flon_generate_exid("n.unlesse.else");

      hlp_launch(
        exid,
        "unlesse\n"
        "  4 > 3\n"
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

