
//
// specifying flon-executor
//
// Wed Sep 24 06:20:10 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_executor.h"


context "flon-executor"
{
  before each
  {
    flon_executor_reset();

    int r;

    char *exid = NULL;

    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }
  after each
  {
    if (exid) free(exid);
  }

  describe "flon_execute()"
  {
    it "executes an invocation"
    {
      exid = flon_generate_exid("xtest.i");

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/exe_%s.json", exid) == 0);
      expect(flu_fstat("var/spool/processed/exe_%s.json", exid) == 'f');

      expect(flu_fstat("var/spool/inv/inv_%s-0.json", exid) == 'f');

      fdja_value *v = fdja_parse_f("var/spool/inv/inv_%s-0.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);
      expect(fdja_ls(v, "nid", NULL) ===f "0");
      expect(fdja_ls(v, "payload.hello", NULL) ===f "world");
      expect(fdja_ls(v, "payload.args.color", NULL) ===f "blue");
      fdja_free(v);

      //puts(flu_readall("var/run/%s.json", exid));
      v = fdja_parse_f("var/run/%s.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);

      expect(fdja_to_json(fdja_l(v, "nodes.0.tree")) ===f ""
        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"blue\"},[]]");

      fdja_free(v);

      expect(flu_fstat("var/spool/inv/inv_%s-0.json", exid) == 'f');
    }

    it "executes an invocation return"
    {
      // at first let's start an execution, with an invocation

      exid = flon_generate_exid("xtest.ir");

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      flon_executor_reset();

      //puts(flu_readall("var/run/%s.json", exid));

      // let's manually return to the execution

      expect(flu_unlink("var/spool/inv/inv_%s-0.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/rcv_%s-0.json", exid,
        "receive: 1\n"
        "exid: %s\n"
        "nid: 0\n"
        "payload: {\n"
          "hello: hiroshima\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/%s-0.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/rcv_%s-0.json", exid) == 0);
      expect(flu_fstat("var/run/%s.json", exid) == 0);
      expect(flu_fstat("var/run/processed/%s.json", exid) == 'f');
    }

    it "passes the return to the parent node"
    {
      fdja_value *v = NULL;

      exid = flon_generate_exid("xtest.pn");

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "execute:\n"
        "  [ sequence, {}, [\n"
        "    [ invoke, { _0: stamp, color: blue }, [] ]\n"
        "    [ invoke, { _0: stamp, color: green }, [] ]\n"
        "  ] ]\n"
        "exid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid
      );

      // let it flow towards "blue"

      r = flon_execute(exid);

      expect(r == 0);

      flon_executor_reset();

      expect(flu_fstat("var/spool/inv/inv_%s-0_0.json", exid) == 'f');

      //puts(flu_readall("var/spool/inv/inv_%s-0.0.json", exid));

      v = fdja_parse_f("var/spool/inv/inv_%s-0_0.json", exid);

      expect(fdja_to_json(fdja_l(v, "invoke", NULL)) ===F fdja_vj(""
        "[ invoke, { _0: stamp, color: blue }, [] ]"));
      expect(fdja_to_json(fdja_l(v, "payload", NULL)) ===F fdja_vj(""
        "{ hello: world, args: { _0: stamp, color: blue } }"));

      fdja_free(v);

      // check the "execution"

      v = fdja_parse_f("var/run/%s.json", exid);
      //puts(fdja_to_pretty_djan(v));

      expect(
        fdja_lj(v, "nodes.0_0", NULL) ===F fdja_vj("{ nid: 0_0, t: invoke }"));

      fdja_free(v);

      // inject ret_ back, towards "green"

      expect(flu_unlink("var/spool/inv/inv_%s-0_0.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0_0.json", exid,
        "receive: 1\n"
        "exid: %s\n"
        "nid: 0_0\n"
        "payload: {\n"
          "hello: chuugoku\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      flon_executor_reset();

      expect(flu_fstat("var/spool/exe/ret_%s-0_0.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/ret_%s-0_0.json", exid) == 0);

      expect(flu_fstat("var/spool/inv/inv_%s-0_1.json", exid) == 'f');

      v = fdja_parse_f("var/spool/inv/inv_%s-0_1.json", exid);
      //puts(fdja_to_pretty_djan(v));

      expect(fdja_lj(v, "invoke", NULL) ===F fdja_vj(""
        "[ invoke, { _0: stamp, color: green }, [] ]"));
      expect(fdja_lj(v, "payload", NULL) ===F fdja_vj(""
        "{ hello: chuugoku, args: { _0: stamp, color: green } }"));

      fdja_free(v);

      // check the "execution"

      v = fdja_parse_f("var/run/%s.json", exid);
      //puts(fdja_to_pretty_djan(v));

      expect(
        fdja_lj(v, "nodes.0_1", NULL) ===F fdja_vj("{ nid: 0_1, t: invoke }"));

      fdja_free(v);

      // inject ret_ back, towards "eox" (end of execution)

      expect(flu_unlink("var/spool/inv/inv_%s-0_1.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0_1.json", exid,
        "receive: 1\n"
        "exid: %s\n"
        "nid: 0_1\n"
        "payload: {\n"
          "hello: staabakusu\n"
        "}\n",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      //flon_executor_reset();

      expect(flu_fstat("var/spool/exe/ret_%s-0_1.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/ret_%s-0_1.json", exid) == 0);

      expect(flu_fstat("var/run/%s.json", exid) == 0);
      expect(flu_fstat("var/run/processed/%s.json", exid) == 'f');

      // check the processed/ execution

      v = fdja_parse_f("var/run/processed/%s.json", exid);
      //puts(fdja_to_pretty_djan(v));

      expect(fdja_lj(v, "nodes", NULL) ===F fdja_vj("{}"));
      expect(fdja_lj(v, "errors", NULL) ===F fdja_vj("{}"));

      fdja_free(v);
    }
  }
}

