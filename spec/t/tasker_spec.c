
//
// specifying flon-tasker
//
// Fri Oct  3 11:24:25 JST 2014
//

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "djan.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_tasker.h"

#include "flon_helpers.h"


context "flon-tasker"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");

    char *s = NULL;
    fdja_value *v = NULL;

    char *exid = NULL;
    char *path = NULL;
  }
  after each
  {
    free(s);
    fdja_free(v);

    free(exid);
    free(path);
  }

  describe "flon_task()"
  {
    it "tasks"
    {
      exid = flon_generate_exid("t.test");
      char *nid = "0_0_7";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: stamp, from: executor }\n"
        "tree: [ task, { _0: stamp }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { hello: world }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 0);

      r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-%s.json", exid, nid, 3);
      expect(r i== 1);
      //
      flu_msleep(300);

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/tsk/tsk_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it

      expect(flu_fstat("var/spool/dis/tsk_%s-%s.json", exid, nid) c== 'f');

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(fdja_ls(v, "hello", NULL) ===f "world");
      expect(fdja_l(v, "stamp") != NULL);

      flu_unlink("var/spool/tsk/tsk_%s-%s.json", exid, nid);
      flu_unlink("var/spool/dis/ret_%s-%s.json", exid, nid);
    }

    it "moves non-identifiable tasks to var/spool/rejected/"
    {
      exid = flon_generate_exid("t.test");
      char *nid = "nada-nada-nada";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "tstate: created\n"
        "taskee: stamp\n"
        "tree: [ task, { _0: stamp }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { a: b }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 1);

      expect(flu_fstat("var/spool/rejected/tsk_%s-%s.json", exid, nid) c== 'f');
    }

    it "moves non-parseable tasks to var/spool/rejected/"
    {
      exid = flon_generate_exid("t.test");
      char *nid = "0_2";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(path, "nada - nada - nada");

      int r = flon_task(path);

      expect(r i== 1);

      expect(flu_fstat("var/spool/rejected/tsk_%s-%s.json", exid, nid) c== 'f');
    }

    it "returns the task as failed if it can't find the taskee"
    {
      exid = flon_generate_exid("t.test.bogus.cant_find_taskee");
      char *nid = "0_5";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: nada, from: executor }\n"
        "tree: [ task, { _0: nada }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { hello: bogus }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 1);

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(v != NULL);
      expect(fdja_ld(v, "point") ===f "task");
      expect(fdja_ld(v, "task.state") ===f "failed");
      expect(fdja_ld(v, "task.event") ===f "offering");
      expect(fdja_ld(v, "task.for") ===f "nada");
      expect(fdja_ld(v, "payload") ===f "{ hello: bogus }");
    }

    it "returns the task as failed if the taskee doesn't have a flon.json"
    {
      exid = flon_generate_exid("t.test.bogus.noflonjson");
      char *nid = "0_6";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: noflonjson, from: executor }\n"
        "tree: [ task, { _0: noflonjson }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { hello: bogus }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 1);

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(v != NULL);
      expect(fdja_ld(v, "point") ===f "task");
      expect(fdja_ld(v, "task.state") ===f "failed");
      expect(fdja_ld(v, "task.event") ===f "offering");
      expect(fdja_ld(v, "task.for") ===f "noflonjson");
      expect(fdja_ld(v, "payload") ===f "{ hello: bogus }");
    }

    it "returns the task as failed if the taskee doesn't have 'cmd' key"
    {
      exid = flon_generate_exid("t.test.bogus.nocmdkey");
      char *nid = "0_7";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: nocmdkey, from: executor }\n"
        "tree: [ task, { _0: norunkey }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { hello: bogus }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 1);

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(v != NULL);
      expect(fdja_ld(v, "point") ===f "task");
      expect(fdja_ld(v, "task.state") ===f "failed");
      expect(fdja_ld(v, "task.event") ===f "offering");
      expect(fdja_ld(v, "task.for") ===f "nocmdkey");
      expect(fdja_ld(v, "payload") ===f "{ hello: bogus }");
    }
  }

  describe "flon_lookup_tasker_path()"
  {
    it "returns null if it doesn't find"
    {
      expect(flon_lookup_tasker_path("t.test.asia.japan", "nada", 1) == NULL);
    }

    it "returns the tasker with the most specific domain"
    {
      expect(flon_lookup_tasker_path("t.test.asia.japan", "stamp", 1) ===f ""
        "usr/local/tsk/t.test.asia.japan/stamp");
      expect(flon_lookup_tasker_path("t.test.asia.philippines", "stamp", 1) ===f "usr/local/tsk/t.test/stamp");
    }

    it "returns the tasker in the 'any' domain by default"
    {
      expect(flon_lookup_tasker_path("xtest.europe.france", "stamp", 1) ===f ""
        "usr/local/tsk/any/stamp");
    }

    context "and the _ offerer"
    {
      it "returns _ if present"
      {
        expect(flon_lookup_tasker_path("z.test.offerer", "nada", 1) ===f ""
          "usr/local/tsk/z.test.offerer/_");
      }

      it "doesn't return _ when created == 0"
      {
        expect(flon_lookup_tasker_path("z.test.offerer", "nada", 0) == NULL);
      }

      it "looks up from downstream domains"
      {
        expect(flon_lookup_tasker_path("z.test.offerer.a", "nada", 1) ===f ""
          "usr/local/tsk/z.test.offerer/_");
      }

      it "stops looking if there is an empty _ offerer"
      {
        expect(flon_lookup_tasker_path("z.test.offerer.x.y", "nada", 1) == NULL);
      }
    }
  }
}

