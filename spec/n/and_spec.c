
//
// specifying instructions
//
// Mon Jan 19 14:01:07 JST 2015
//

#include "fl_ids.h"
#include "fl_paths.h"
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

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result));

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

      result = hlp_wait(exid, "terminated", NULL, 7);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");
    }

    it "ands (miss 2/3)"
    {
      exid = flon_generate_exid("n.ao.and.1");

      hlp_launch(
        exid,
        "and\n"
        "  val true\n"
        "  val false\n"
        "  val true\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");

      char *fep = flon_exid_path(exid);
      //flu_system("cat var/archive/%s/msg.log", fep);
      //flu_system("wc -l var/archive/%s/msg.log", fep);

      char *lc = flu_pline("wc -l var/archive/%s/msg.log", fep);
      char *space = strchr(lc, ' '); if (space) *space = 0;

      expect(lc ===f "9");

      free(fep);
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

      result = hlp_wait(exid, "terminated", NULL, 7);
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

      result = hlp_wait(exid, "terminated", NULL, 7);
      //flon_pp_execution(exid);

      expect(result != NULL);
      //flu_putf(fdja_todc(result));

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: false }");
    }
  }
}

