
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
  }
  after each
  {
    free(exid);
    fdja_free(result);
    fdja_free(v);
  }

  describe "an _ offerer"
  {
    it "sees its offer logged in msgs.log"
  }

  describe "a .rad _ offerer"
  {
    it "offers a task to a taskee"
    {
      exid = flon_generate_exid("ztest.offerer.rad");

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
  }

  describe "a non .rad _ offerer"
  {
    it "offers a task to a taskee"
  }
}

