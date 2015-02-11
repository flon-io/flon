
//
// specifying instructions
//
// Wed Feb 11 10:27:20 JST 2015
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

  describe "cand"
  {
    it "ands (hit)"
//    {
//      exid = flon_generate_exid("n.ao.and.0");
//
//      hlp_launch(
//        exid,
//        "cand\n"
//        "  sequence\n"
//        "    trace 'a $(nid)'\n"
//        "    trace 'a $(nid)'\n"
//        "    true\n"
//        "  sequence\n"
//        "    trace 'b $(nid)'\n"
//        "    trace 'b $(nid)'\n"
//        "    true\n"
//        "",
//        "{}");
//
//      result = hlp_wait(exid, "terminated", NULL, 7);
//      //flon_pp_execution(exid);
//
//      expect(result != NULL);
//      //flu_putf(fdja_todc(result));
//
//      expect(fdja_ld(result, "payload.trace") ===f ""
//        "[ \"a 0_0_0\", \"b 0_1_0\", \"a 0_0_1\", \"a 0_1_1\" ]");
//    }

    it "ands (miss)"
  }

  describe "cor"
  {
    it "ors (hit)"
    it "ors (miss)"
  }
}

