
//
// specifying flon
//
// Mon Feb  2 06:49:00 JST 2015
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "flon and taskers:"
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

  describe "the taskmaster"
  {
    it "tasks a specific domain tasker"
    {
      exid = flon_generate_exid("ttest.asia.japan");

      hlp_launch(
        exid,
        "task stamp\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ stamp: \"ttest.asia.japan stamp\" }");
    }

    it "tasks a less specific domain tasker"
    {
      exid = flon_generate_exid("ttest.europe.france");

      hlp_launch(
        exid,
        "task stamp\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ stamp: \"ttest stamp\" }");
    }

    it "tasks an 'any' tasker"
    {
      exid = flon_generate_exid("xtest.europe.france");

      hlp_launch(
        exid,
        "task stamp\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "payload.stamp") ^==f "20");
    }
  }

  describe "mirrora"
  {
    it "mirrors the whole task"
    {
      exid = flon_generate_exid("ztest.mirrora");

      hlp_launch(
        exid,
        "sequence\n"
        "  set f.a: 1\n"
        "  task mirrora\n"
        "  trace $(f.a)\n"
        "",
        "{ a: 0 }");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f "{ a: 1, ret: 1, trace: [ 1 ] }");
    }
  }
}

