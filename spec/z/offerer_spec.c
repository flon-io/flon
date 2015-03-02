
//
// specifying flon
//
// Thu Feb 26 14:11:58 JST 2015
//

#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "task offering:"
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
    fdja_value *l = NULL;
  }
  after each
  {
    free(exid);
    fdja_free(result);
    fdja_free(v);
    fdja_free(l);
  }

  describe "an _ offerer"
  {
    it "sees its offer logged in msg.log"
    {
      exid = flon_generate_exid("z.test.offerer.rad");

      hlp_launch(
        exid,
        "task ostamp\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      l = hlp_read_tsk_log(exid);
      //fdja_putdc(l);

      expect(fdja_size(l) zu== 3);

      fdja_value *offer = fdja_at(l, 1);

      expect(fdja_ld(offer, "task") ===f ""
        "{ "
          "state: offered, "
          "event: offering, "
          "from: usr/local/tsk/z.test.offerer/_/offerer.rad, "
          "for: stamp "
        "}");
    }
  }

  describe "a .rad _ offerer"
  {
    it "offers a task to a taskee"
    {
      exid = flon_generate_exid("z.test.offerer.rad");

      hlp_launch(
        exid,
        "task ostamp\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "payload.stamp", NULL) ^==f "20");
    }

    it "has access to domain variables"
    {
      exid = flon_generate_exid("z.test.offerer.rad");

      hlp_launch(
        exid,
        "task state\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "error.msg", NULL) ===f ""
        "didn't find tasker 'Texas' (domain z.test.offerer.rad)");
    }
  }

  describe "a non .rad _ offerer"
  {
    it "offers a task to a taskee"
    {
      exid = flon_generate_exid("z.test.offerer.nonrad");

      hlp_launch(
        exid,
        "task ostamp\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "payload.stamp", NULL) ^==f "20");
    }
  }
}

