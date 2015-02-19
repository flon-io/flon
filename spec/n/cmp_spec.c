
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

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
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

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares nulls (hit)"
      {
        exid = flon_generate_exid("n.cmp.eq.n.hit");

        hlp_launch(
          exid,
          "== \n"
          "  null\n"
          "  null\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "compares nulls (miss)"
      {
        exid = flon_generate_exid("n.cmp.eq.n.miss");

        hlp_launch(
          exid,
          "sequence\n"
          "  == \n"
          "    null\n"
          "    a\n"
          "  trace $(ret)\n"
          "  == \n"
          "    a\n"
          "    null\n"
          "  trace $(ret)\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: false, trace: [ false, false ] }");
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

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares floats (hit)"

      it "compares floats (miss)"
      {
        exid = flon_generate_exid("n.cmp.eq.f.miss");

        hlp_launch(
          exid,
          "== \n"
          "  12.3\n"
          "  12.0\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }
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

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares nulls (hit)"
      {
        exid = flon_generate_exid("n.cmp.neq.n.hit");

        hlp_launch(
          exid,
          "sequence\n"
          "  !=\n"
          "    null\n"
          "    a\n"
          "  trace $(ret)\n"
          "  !=\n"
          "    a\n"
          "    null\n"
          "  trace $(ret)\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: true, trace: [ true, true ] }");
      }

      it "compares nulls (miss)"
      {
        exid = flon_generate_exid("n.cmp.eq.n.miss");

        hlp_launch(
          exid,
          "!= \n"
          "  null\n"
          "  null\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

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

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "compares integers (miss)"
      {
        exid = flon_generate_exid("n.cmp.neq.i.miss");

        hlp_launch(
          exid,
          "!=\n"
          "  123\n"
          "  123\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares floats (hit)"
      {
        exid = flon_generate_exid("n.cmp.neq.f.hit");

        hlp_launch(
          exid,
          "!=\n"
          "  12.3\n"
          "  12.0\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "compares floats (miss)"
    }

    context "=~"
    {
      it "matches strings (hit)"
      {
        exid = flon_generate_exid("n.cmp.match.s.hit");

        hlp_launch(
          exid,
          "=~\n"
          "  toto\n"
          "  t.t.\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "matches strings (miss)"
      {
        exid = flon_generate_exid("n.cmp.match.s.hit");

        hlp_launch(
          exid,
          "=~\n"
          "  poto\n"
          "  t.t.\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "matches ^ at the beginning"
      {
        exid = flon_generate_exid("n.cmp.match.caret");

        hlp_launch(
          exid,
          "sequence\n"
          "  =~\n"
          "    poto\n"
          "    ^po\n"
          "  trace $(ret)\n"
          "  =~\n"
          "    poto\n"
          "    ^no\n"
          "  trace $(ret)\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: false, trace: [ true, false ] }");
      }

      it "matches $ at the end"
      {
        exid = flon_generate_exid("n.cmp.match.dollar");

        hlp_launch(
          exid,
          "sequence\n"
          "  =~\n"
          "    poto\n"
          "    'to$'\n"
          "  trace $(ret)\n"
          "  =~\n"
          "    poto\n"
          "    'no$'\n"
          "  trace $(ret)\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: false, trace: [ true, false ] }");
      }

      it "matches objects (hit)"
      it "matches objects (miss)"

      it "accepts /.../ regexes"
      {
        exid = flon_generate_exid("n.cmp.match.slash");

        hlp_launch(
          exid,
          "=~\n"
          "  toto\n"
          "  /(t.)+/\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "accepts /.../i regexes"
      {
        exid = flon_generate_exid("n.cmp.match.slash.i");

        hlp_launch(
          exid,
          "=~\n"
          "  TOto\n"
          "  /(t.)+/i\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "fails when it cannot compile the regex"
      {
        exid = flon_generate_exid("n.cmp.match.fail");

        hlp_launch(
          exid,
          "=~\n"
          "  poto\n"
          "  xxx(yyy\n"
          "",
          "{}");

        result = hlp_wait(exid, "failed", NULL, 3);

        expect(result != NULL);
        //fdja_putdc(result);

        expect(fdja_ld(result, "payload") ===f ""
          "{ ret: false }");
        expect(fdja_ls(result, "error.msg") ===f ""
          "regex compilation failed: Unmatched ( or \\("
          " in >xxx(yyy<");
      }

      it "sets matches in variables..."
    }

    context "!~"
    {
      it "doesn't match strings (hit)"
      {
        exid = flon_generate_exid("n.cmp.nomatch.s.hit");

        hlp_launch(
          exid,
          "!~\n"
          "  toto\n"
          "  r.r.\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: true }");
      }

      it "doesn't match strings (miss)"
      {
        exid = flon_generate_exid("n.cmp.nomatch.s.hit");

        hlp_launch(
          exid,
          "!~\n"
          "  toto\n"
          "  t.t.\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);

        expect(fdja_ld(result, "payload") ===f "{ ret: false }");
      }

      it "unmatches objects (hit)"
      it "unmatches objects (miss)"
    }
  }
}

