
//
// specifying instructions
//
// Sat Feb  7 06:04:04 JST 2015
//

#include "fl_ids.h"
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

  describe "elsif / elif"
  {
    it "doesn't trigger when 'ret' not false"
    {
      exid = flon_generate_exid("n.elsif.no");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  elsif\n"
        "    4 > 3\n"
        "    trace b\n"
        "    trace c\n"
        "  trace d\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, d ] }");
    }

    it "triggers when 'ret' false"
    {
      exid = flon_generate_exid("n.elsif.yes");

      hlp_launch(
        exid,
        "sequence\n"
        "  false\n"
        "  trace a\n"
        "  elsif\n"
        "    3 > 2\n"
        "    trace b\n"
        "    trace c\n"
        "  trace d\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ ret: true, trace: [ a, b, c, d ] }");
    }

    it "fits well right after an 'if'"
    {
      exid = flon_generate_exid("n.elsif.postif");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  if\n"
        "    5 > 4\n"
        "    trace b\n"
        "    trace c\n"
        "  elif\n"
        "    true\n"
        "    trace d\n"
        "    trace e\n"
        "  if\n"
        "    3 > 4\n"
        "    trace f\n"
        "    trace g\n"
        "  elsif\n"
        "    true\n"
        "    trace h\n"
        "    trace i\n"
        "  trace j\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, b, c, h, i, j ], ret: true }");
    }

    it "passes if it evaluates to false"
    {
      exid = flon_generate_exid("n.elsif.pass");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  if\n"
        "    3 > 4\n"
        "    trace b\n"
        "    trace c\n"
        "  elif\n"
        "    3 > 5\n"
        "    trace d\n"
        "    trace e\n"
        "  elif\n"
        "    3 > 2\n"
        "    trace f\n"
        "    trace g\n"
        "  trace h\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, f, g, h ], ret: true }");
    }

    it "leverages rewrite"
    {
      exid = flon_generate_exid("n.elsif.rewritten");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  if 3 > 4\n"
        "    trace b\n"
        "    trace c\n"
        "  elif 3 > 5\n"
        "    trace d\n"
        "    trace e\n"
        "  elsif 3 > 2\n"
        "    trace f\n"
        "    trace g\n"
        "  trace h\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, f, g, h ], ret: true }");
    }

    it "leverages the 'else if' rewrite"
    {
      exid = flon_generate_exid("n.else_if.rewritten");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  if 3 > 4\n"
        "    trace b\n"
        "    trace c\n"
        "  else if 3 > 2\n"
        "    trace d\n"
        "    trace e\n"
        "  trace f\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, d, e, f ], ret: true }");
    }

    it "leverages the 'then' rewrite"
    {
      exid = flon_generate_exid("n.else_if.then.rewrite");

      hlp_launch(
        exid,
        "sequence\n"
        "  trace a\n"
        "  # ...\n"
        "  if 3 > 4 then trace b\n"
        "  else if 3 > 2 then trace c\n"
        "  else trace d\n"
        "  # ...\n"
        "  if 3 > 4 then trace e\n"
        "  else if 3 > 5 then trace f\n"
        "  else trace g\n"
        "  # ...\n"
        "  trace h\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ld(result, "payload") ===f ""
        "{ trace: [ a, c, g, h ], ret: false }");
    }
  }
}

