
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
    int r;

    char *exid = NULL;
    char *fep = NULL;

    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }
  after each
  {
    if (fep) free(fep);
    if (exid) free(exid);
  }

  describe "flon_execute()"
  {
    it "executes an invocation"
    {
      exid = flon_generate_exid("xtest.i");
      fep = flon_exid_path(exid);

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "{"
          "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/exe_%s.json", exid) == 0);
      expect(flu_fstat("var/run/%s/processed/exe_%s.json", fep, exid) == 'f');

      expect(flu_fstat("var/spool/dis/inv_%s-0.json", exid) == 'f');

      fdja_value *v = fdja_parse_f("var/spool/dis/inv_%s-0.json", exid);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);
      expect(fdja_ls(v, "nid", NULL) ===f "0");
      expect(fdja_ls(v, "payload.hello", NULL) ===f "world");
      expect(fdja_ls(v, "payload.args.color", NULL) ===f "blue");
      fdja_free(v);

      //puts(fdja_f_todc("var/run/%s/run.json", fep));
      v = fdja_parse_f("var/run/%s/run.json", fep);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);

      expect(fdja_to_json(fdja_l(v, "nodes.0.tree")) ===f ""
        "[\"invoke\",{\"_0\":\"stamp\",\"color\":\"blue\"},[]]");

      fdja_free(v);
    }

    it "executes an invocation return"
    {
      // at first let's start an execution, with an invocation

      exid = flon_generate_exid("xtest.ir");
      fep = flon_exid_path(exid);

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "{"
          "execute: [ invoke, { _0: stamp, color: blue }, [] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      //puts(flu_readall("var/run/%s.json", exid));

      // let's manually return to the execution

      expect(flu_unlink("var/spool/dis/inv_%s-0.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/rcv_%s-0.json", exid,
        "{"
          "receive: 1\n"
          "exid: %s\n"
          "nid: 0\n"
          "payload: {\n"
            "hello: hiroshima\n"
          "}\n"
        "}",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/%s-0.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/rcv_%s-0.json", exid) == 0);
      expect(flu_fstat("var/run/%s.json", exid) == 0);
      expect(flu_fstat("var/archive/%s/run.json", fep) == 'f');
    }

    it "passes the return to the parent node"
    {
      fdja_value *v = NULL;

      exid = flon_generate_exid("xtest.pn");
      fep = flon_exid_path(exid);

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "{"
          "execute:\n"
          "  [ sequence, {}, [\n"
          "    [ invoke, { _0: stamp, color: blue }, [] ]\n"
          "    [ invoke, { _0: stamp, color: green }, [] ]\n"
          "  ] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        exid
      );

      // let it flow towards "blue"

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/dis/inv_%s-0_0.json", exid) == 'f');

      //puts(flu_readall("var/spool/dis/inv_%s-0.0.json", exid));

      v = fdja_parse_f("var/spool/dis/inv_%s-0_0.json", exid);

      expect(fdja_to_json(fdja_l(v, "invoke", NULL)) ===F fdja_vj(""
        "[ invoke, { _0: stamp, color: blue }, [] ]"));
      expect(fdja_to_json(fdja_l(v, "payload", NULL)) ===F fdja_vj(""
        "{ hello: world, args: { _0: stamp, color: blue } }"));

      fdja_free(v);

      // check the "execution"

      v = fdja_parse_f("var/run/%s/run.json", fep);
      //puts(fdja_todc(v));

      expect(fdja_lj(v, "nodes.0_0.nid", NULL) ===f "\"0_0\"");
      expect(fdja_lj(v, "nodes.0_0.p", NULL) ===f "\"0\"");
      expect(fdja_lj(v, "nodes.0_0.t", NULL) ===f "\"invoke\"");
      expect(fdja_lj(v, "nodes.0_0.c", NULL) ^==f "\"20");

      fdja_free(v);

      char *s = flu_readall("var/run/%s/msgs.log", fep);
      expect(s != NULL);
      expect(s >== "color:blue}");
      expect(s >== "color:green}");
      expect(s >== "\n");
      expect(s >== exid);
      free(s);

      // inject ret_ back, towards "green"

      expect(flu_unlink("var/spool/dis/inv_%s-0_0.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0_0.json", exid,
        "{"
          "receive: 1\n"
          "exid: %s\n"
          "nid: 0_0\n"
          "payload: {\n"
            "hello: chuugoku\n"
          "}\n"
        "}",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/ret_%s-0_0.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/ret_%s-0_0.json", exid) == 0);

      expect(flu_fstat("var/spool/dis/inv_%s-0_1.json", exid) == 'f');

      v = fdja_parse_f("var/spool/dis/inv_%s-0_1.json", exid);
      //puts(fdja_todc(v));

      expect(fdja_lj(v, "invoke", NULL) ===F fdja_vj(""
        "[ invoke, { _0: stamp, color: green }, [] ]"));
      expect(fdja_lj(v, "payload", NULL) ===F fdja_vj(""
        "{ hello: chuugoku, args: { _0: stamp, color: green } }"));

      fdja_free(v);

      // check the "execution"

      v = fdja_parse_f("var/run/%s/run.json", fep);
      //puts(fdja_to_pretty_djan(v));

      expect(fdja_lj(v, "nodes.0_1.nid", NULL) ===f "\"0_1\"");
      expect(fdja_lj(v, "nodes.0_1.p", NULL) ===f "\"0\"");
      expect(fdja_lj(v, "nodes.0_1.t", NULL) ===f "\"invoke\"");
      expect(fdja_lj(v, "nodes.0_1.c", NULL) ^==f "\"20");

      fdja_free(v);

      // inject ret_ back, towards "eox" (end of execution)

      expect(flu_unlink("var/spool/dis/inv_%s-0_1.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0_1.json", exid,
        "{"
          "receive: 1\n"
          "exid: %s\n"
          "nid: 0_1\n"
          "payload: {\n"
            "hello: staabakusu\n"
          "}\n"
        "}",
        exid
      );

      r = flon_execute(exid);

      expect(r == 0);

      expect(flu_fstat("var/spool/exe/ret_%s-0_1.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/ret_%s-0_1.json", exid) == 0);

      //printf("var/archive/%s/run.json\n", fep);
      expect(flu_fstat("var/run/%s/run.json", fep) == 0);
      expect(flu_fstat("var/archive/%s/run.json", fep) == 'f');

      // check the archived/ execution

      v = fdja_parse_f("var/archive/%s/run.json", fep);
      //puts(fdja_todc(v));

      expect(fdja_lj(v, "nodes", NULL) ===F fdja_vj("{}"));
      expect(fdja_lj(v, "errors", NULL) ===F fdja_vj("{}"));

      fdja_free(v);
    }
  }
}

