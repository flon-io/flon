
//
// specifying instructions
//
// Thu Feb 12 06:09:49 JST 2015
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

  describe "map"
  {
    it "iterates over an array"
//    {
//      exid = flon_generate_exid("n.map.array");
//
//      hlp_launch(
//        exid,
//        "map [ 1 2 3 ]\n"
//        "  $(i) + $(ret)\n"
//        "",
//        "{ i: 4 }");
//
//      result = hlp_wait(exid, "terminated", NULL, 7);
//
//      //flon_pp_execution(exid);
//
//      expect(result != NULL);
//      //puts(fdja_todc(result));
//
//      //expect(fdja_ls(result, "point", NULL) ===f "receive");
//      //expect(fdja_ls(result, "nid", NULL) ===f "0");
//      //expect(fdja_ls(result, "from", NULL) == NULL);
//      //expect(fdja_ls(result, "payload.hello", NULL) ===f "task");
//      //expect(fdja_ls(result, "payload.stamp", NULL) ^==f "20");
//      //expect(fdja_l(result, "payload.args") == NULL);
//    }

    it "iterates over an object"
    it "iterates over $(f.ret) by default"
    it "iterates and maps to a callable"
  }
}

