
//
// specifying instructions
//
// Mon Jan 12 11:41:31 JST 2015
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

  describe "cmp"
  {
    context ">"
    {
      it "compares integers (hit)"
      {
        exid = flon_generate_exid("n.cmp.0");

        hlp_launch(
          exid,
          ">\n"
          "  val $(x)\n"
          "  val 3\n"
          "",
          "{ x: 4 }");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ x: 4, ret: true }");
      }

      it "compares integers (miss)"
      {
        exid = flon_generate_exid("n.cmp.1");

        hlp_launch(
          exid,
          ">\n"
          "  val $(x)\n"
          "  val 8\n"
          "",
          "{ x: 7 }");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ x: 7, ret: false }");
      }

      it "accepts a one-liner  > $(a) b"
      {
        exid = flon_generate_exid("n.cmp.2");

        hlp_launch(
          exid,
          "> $(x) 8\n"
          "",
          "{ x: 9 }");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ x: 9, ret: true }");
      }

      it "accepts a one-liner  $(a) > b"
      {
        exid = flon_generate_exid("n.cmp.3");

        hlp_launch(
          exid,
          "$(x) > 8\n"
          "",
          "{ x: 10 }");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ x: 10, ret: true }");
      }
    }

    context "=="
    {
      it "compares objects (hit)"
      {
        exid = flon_generate_exid("n.cmp.eq.o.hit");

        hlp_launch(
          exid,
          "== \n"
          "  [ 1, 2, 3 ]\n"
          "  [ 1, 2, 3 ]\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: true }");
      }

      it "compares objects (miss)"
      {
        exid = flon_generate_exid("n.cmp.eq.o.miss");

        hlp_launch(
          exid,
          "== \n"
          "  [ 1, 2, 3 ]\n"
          "  [ 1, 2 ]\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: false }");
      }

      it "compares integers (hit)"
      {
        exid = flon_generate_exid("n.cmp.eq.i.hit");

        hlp_launch(
          exid,
          "== \n"
          "  123\n"
          "  123\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "compares integers (miss)"
      {
        exid = flon_generate_exid("n.cmp.eq.i.miss");

        hlp_launch(
          exid,
          "== \n"
          "  123\n"
          "  12\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares floats (hit)"
      it "compares floats (miss)"
    }

    context "!="
    {
      it "compares objects (hit)"
      {
        exid = flon_generate_exid("n.cmp.neq.o.hit");

        hlp_launch(
          exid,
          "!= \n"
          "  [ 1, 2, 3 ]\n"
          "  [ 1, 2 ]\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "compares objects (miss)"
      {
        exid = flon_generate_exid("n.cmp.neq.o.miss");

        hlp_launch(
          exid,
          "!= \n"
          "  [ 1, 2, 3 ]\n"
          "  [ 1, 2, 3 ]\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares integers (hit)"
      {
        exid = flon_generate_exid("n.cmp.neq.i.hit");

        hlp_launch(
          exid,
          "!= \n"
          "  123\n"
          "  12\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "compares integers (miss)"
      {
        exid = flon_generate_exid("n.cmp.neq.i.miss");

        hlp_launch(
          exid,
          "!= \n"
          "  123\n"
          "  123\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //flu_putf(fdja_todc(result));

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares floats (hit)"
      it "compares floats (miss)"
    }

    context "=~"
    {
      it "matches strings (hit)"
      it "matches strings (miss)"
      it "matches objects (hit)"
      it "matches objects (miss)"
    }

    context "!~"
    {
      it "unmatches strings (hit)"
      it "unmatches strings (miss)"
      it "unmatches objects (hit)"
      it "unmatches objects (miss)"
    }
  }
}

