
//
// specifying instructions
//
// Thu Jan  8 10:05:51 JST 2015
//

#include "flutil.h"
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

  describe "call"
  {
    it "calls"
    {
      exid = flon_generate_exid("n.call.vanilla");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace $(nid)\n"
        "  call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ trace: [ 0_0_0-1 ] }");
    }

    it "calls (wrapping the define in a sequence)"
    {
      exid = flon_generate_exid("n.call.vanilla");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace 'a $(nid)'\n"
        "    trace 'b $(nid)'\n"
        "  call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ trace: [ \"a 0_0_0-1\", \"b 0_0_1-1\" ] }");
    }

    it "increments the counter"
    {
      exid = flon_generate_exid("n.call.vanilla");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub\n"
        "    trace '$(f.x) $(nid)'\n"
        "  call sub f.x: a\n"
        "  call sub f.x: b\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ x: b, trace: [ \"a 0_0_0-1\", \"b 0_0_0-2\" ] }");
    }

    it "maps arguments"
    {
      exid = flon_generate_exid("n.call.maps");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub a0 a1\n"
        "    trace $(v.args)\n"
        "  call sub egg bacon lettuce\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ a0: egg, a1: bacon, trace: [ [ sub, lettuce ] ] }");
    }

    it "maps named arguments"
    {
      exid = flon_generate_exid("n.call.maps.named");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub a0 a1\n"
        "    trace $(v.b)\n"
        "    trace $(v.args)\n"
        "  call sub a1: egg a0: bacon cheese a2: lettuce v.b: b\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ a1: egg, a0: bacon, a2: lettuce, trace: [ b, [ sub, cheese ] ] }");
    }

    it "maps to variables when the define requires it"
    {
      exid = flon_generate_exid("n.call.maps.defvars");

      hlp_launch(
        exid,
        "sequence\n"
        "  define sub v.a0 v.a1\n"
        "    trace '$(v.a0) $(v.a1)'\n"
        "  call sub red green\n"
        "  call sub v.a1: red, v.a0: green\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload")) ===f ""
        "{ trace: [ \"red green\", \"green red\" ] }");
    }

    it "fails if there is no corresponding define"
    {
      exid = flon_generate_exid("n.call.nosub");

      hlp_launch(
        exid,
        "call sub\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "error.msg", NULL) ===f "not callable 'sub'");
    }

    it "fails gracefully when given nothing to call"
    {
      exid = flon_generate_exid("n.call.nothing");

      hlp_launch(
        exid,
        "call\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 3);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "error.msg", NULL) ===f "nothing to call: {}");
    }

    context "libraries"
    {
      it "calls libraries"
      {
        exid = flon_generate_exid("n.call.lib");

        hlp_launch(
          exid,
          "sequence\n"
          "  call adders.rad # 'import'\n"
          "  add3 7\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL);
        //fdja_putdc(result);

        expect(fdja_ld(result, "payload", NULL) ===f ""
          "{ ret: 10 }");
      }

      it "favours the longest [sub]domain"
      {
        exid = flon_generate_exid("n.call.longest");

        hlp_launch(
          exid,
          "sequence\n"
          "  call extras.rad # 'import'\n"
          "  sayhello\n"
          "",
          "{}");

        result = hlp_wait(exid, "terminated", NULL, 3);

        expect(result != NULL); //fdja_putdc(result);

        expect(fdja_ld(result, "payload", NULL) ===f ""
          "{ trace: [ \"hello n.call\" ] }");
      }

      it "fails if the .rad lib can't be found"
      {
        exid = flon_generate_exid("n.call.missing");

        hlp_launch(
          exid,
          "sequence\n"
          "  call nada.rad # 'import'\n"
          "  sayhello\n"
          "",
          "{}");

        result = hlp_wait(exid, "failed", NULL, 3);

        expect(result != NULL); //fdja_putdc(result);

        expect(fdja_ld(result, "payload", NULL) ===f ""
          "{}");
        expect(fdja_ls(result, "error.msg", NULL) ===f ""
          "couldn't find lib 'nada.rad'");
      }
    }

    context "uris"
    {
      it "calls rad files behind URIs"
    }
  }
}

