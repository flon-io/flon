
//
// specifying flon-executor
//
// Wed Sep 24 06:20:10 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"
#include "fl_common.h"
#include "fl_executor.h"


context "flon-executor"
{
  before each
  {
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
    free(fep);
    free(exid);
  }

  describe "flon_execute()"
  {
    it "executes a task instruction exe"
    {
      exid = flon_generate_exid("xtest.t");
      fep = flon_exid_path(exid);

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "{"
          "point: execute\n"
          "tree: [ task, { _0: stamp, color: blue }, [] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        exid
      );

      flon_execute(exid);

      expect(flu_fstat("var/spool/exe/exe_%s.json", exid) == 0);
      expect(flu_fstat("var/run/%s/processed/exe_%s.json", fep, exid) == 'f');

      expect(flu_fstat("var/spool/dis/tsk_%s-0.json", exid) == 'f');

      fdja_value *v = fdja_parse_f("var/spool/dis/tsk_%s-0.json", exid);

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

      expect(fdja_ld(v, "nodes.0.tree") ===f ""
        "[ task, { _0: stamp, color: blue }, [] ]");

      fdja_free(v);
    }

    it "executes task instruction rcv"
    {
      // at first let's start an execution, with a task

      exid = flon_generate_exid("xtest.tr");
      fep = flon_exid_path(exid);

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "{"
          "point: execute\n"
          "tree: [ task, { _0: stamp, color: blue }, [] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: world\n"
          "}\n"
        "}",
        exid
      );

      flon_execute(exid);

      //puts(flu_readall("var/run/%s.json", exid));

      // let's manually return to the execution

      expect(flu_unlink("var/spool/dis/tsk_%s-0.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/rcv_%s-0.json", exid,
        "{"
          "point: receive\n"
          "exid: %s\n"
          "nid: 0\n"
          "payload: {\n"
            "hello: hiroshima\n"
          "}\n"
        "}",
        exid
      );

      flon_execute(exid);

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
          "point: execute\n"
          "tree:\n"
          "  [ sequence, {}, 1, [\n"
          "    [ task, { _0: stamp, color: blue }, 2, [] ]\n"
          "    [ task, { _0: stamp, color: green }, 3, [] ]\n"
          "  ] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: xtest.pn\n"
          "}\n"
        "}",
        exid
      );

      // let it flow towards "blue"

      flon_execute(exid);

      //flon_pp_execution(exid);

      expect(flu_fstat("var/spool/dis/tsk_%s-0_0.json", exid) == 'f');

      //puts(flu_readall("var/spool/dis/tsk_%s-0.0.json", exid));

      v = fdja_parse_f("var/spool/dis/tsk_%s-0_0.json", exid);

      expect(fdja_ls(v, "point", NULL) ===f ""
        "task");
      expect(fdja_to_json(fdja_l(v, "tree", NULL)) ===F fdja_vj(""
        "[ task, { _0: stamp, color: blue }, 2, [] ]"));
      expect(fdja_to_json(fdja_l(v, "payload", NULL)) ===F fdja_vj(""
        "{ hello: xtest.pn, args: { _0: stamp, color: blue } }"));

      fdja_free(v);

      // check the "execution"

      v = fdja_parse_f("var/run/%s/run.json", fep);
      //puts(fdja_todc(v));

      expect(fdja_ld(v, "nodes.0_0.nid", NULL) ===f "0_0");
      expect(fdja_ld(v, "nodes.0_0.parent", NULL) ===f "\"0\"");
      expect(fdja_ld(v, "nodes.0_0.inst", NULL) ===f "task");
      expect(fdja_lj(v, "nodes.0_0.created", NULL) ^==f "\"20");

      fdja_free(v);

      char *s = flu_readall("var/run/%s/msgs.log", fep);
      expect(s != NULL);
      expect(s >== "color:blue}");
      expect(s >== "color:green}");
      expect(s >== "\n");
      expect(s >== exid);
      free(s);

      // inject ret_ back, towards "green"

      expect(flu_unlink("var/spool/dis/tsk_%s-0_0.json", exid) i== 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0_0.json", exid,
        "{"
          "point: receive\n"
          "exid: %s\n"
          "nid: 0_0\n"
          "payload: {\n"
            "hello: chuugoku\n"
          "}\n"
        "}",
        exid
      );

      flon_execute(exid);

      expect(flu_fstat("var/spool/exe/ret_%s-0_0.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/ret_%s-0_0.json", exid) == 0);

      expect(flu_fstat("var/spool/dis/tsk_%s-0_1.json", exid) == 'f');

      v = fdja_parse_f("var/spool/dis/tsk_%s-0_1.json", exid);
      //puts(fdja_todc(v));

      expect(fdja_ls(v, "point", NULL) ===f ""
        "task");
      expect(fdja_lj(v, "tree", NULL) ===F fdja_vj(""
        "[ task, { _0: stamp, color: green }, 3, [] ]"));
      expect(fdja_lj(v, "payload", NULL) ===F fdja_vj(""
        "{ hello: chuugoku, args: { _0: stamp, color: green } }"));

      fdja_free(v);

      // check the "execution"

      v = fdja_parse_f("var/run/%s/run.json", fep);
      //puts(fdja_to_pretty_djan(v));

      expect(fdja_ld(v, "nodes.0_1.nid") ===f "0_1");
      expect(fdja_ld(v, "nodes.0_1.parent") ===f "\"0\"");
      expect(fdja_ld(v, "nodes.0_1.inst") ===f "task");
      expect(fdja_lj(v, "nodes.0_1.created", NULL) ^==f "\"20");

      fdja_free(v);

      // inject ret_ back, towards "eox" (end of execution)

      expect(flu_unlink("var/spool/dis/tsk_%s-0_1.json", exid) == 0);

      flu_writeall(
        "var/spool/exe/ret_%s-0_1.json", exid,
        "{"
          "point: receive\n"
          "exid: %s\n"
          "nid: 0_1\n"
          "payload: {\n"
            "hello: staabakusu\n"
          "}\n"
        "}",
        exid
      );

      flon_execute(exid);

      expect(flu_fstat("var/spool/exe/ret_%s-0_1.json", exid) == 0);
      expect(flu_fstat("var/spool/rejected/ret_%s-0_1.json", exid) == 0);

      //printf("var/archive/%s/run.json\n", fep);
      expect(flu_fstat("var/run/%s/run.json", fep) == 0);
      expect(flu_fstat("var/archive/%s/run.json", fep) == 'f');

      // check the archived/ execution

      v = fdja_parse_f("var/archive/%s/run.json", fep);
      //puts(fdja_todc(v));

      expect(fdja_lj(v, "nodes", NULL) ===F fdja_vj("{}"));

      fdja_free(v);
    }

    it "expands dollar notation"
    {
      exid = flon_generate_exid("xtest.i");
      fep = flon_exid_path(exid);

      flu_writeall(
        "var/spool/exe/exe_%s.json", exid,
        "{"
          "point: execute\n"
          "tree: [ trace, { _0: \"hello $(recipient)\" }, [] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "recipient: world\n"
          "}\n"
        "}",
        exid
      );

      flon_execute(exid);

      //flon_pp_execution(exid);

      char *s = flu_pline("tail -1 var/archive/%s/msgs.log", fep);

      expect(s >===f ",trace:[\"hello world\"]");
    }
  }
}

