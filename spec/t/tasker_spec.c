
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
    char *nid = NULL;
    char *path = NULL;
  }
  after each
  {
    free(s);
    fdja_free(v);

    free(exid);
    //free(nid); // no need
    free(path);
  }

  describe "flon_task()"
  {
    it "tasks"
    {
      exid = flon_generate_exid("ttest");
      nid = "0_0_7";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "state: created\n"
        "taskee: stamp\n"
        "tree: [ task, { _0: stamp }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r == 0);

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

    it "returns the task as failed if it can't find the taskee"
    {
      exid = flon_generate_exid("ttest.cant_find_taskee");
      nid = "0_5";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "state: created\n"
        "taskee: nada\n"
        "tree: [ task, { _0: nada }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 1);

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(v != NULL);
      expect(fdja_ld(v, "point") ===f "task");
      expect(fdja_ld(v, "state") ===f "failed");
      expect(fdja_ld(v, "taskee") ===f "nada");
      expect(fdja_ld(v, "on") ===f "offer");
      expect(fdja_ld(v, "payload") ===f "{ hello: world }");
    }
  }

  describe "flon_lookup_tasker()"
  {
    it "returns null if it doesn't find"
    {
      expect(flon_lookup_tasker("ttest.asia.japan", "nada") == NULL);
    }

    it "returns the tasker with the most specific domain"
    {
      expect(flon_lookup_tasker("ttest.asia.japan", "stamp") ===f ""
        "usr/local/tsk/ttest.asia.japan/stamp");
      expect(flon_lookup_tasker("ttest.asia.philippines", "stamp") ===f ""
        "usr/local/tsk/ttest/stamp");
    }

    it "returns the tasker in the 'any' domain by default"
    {
      expect(flon_lookup_tasker("xtest.europe.france", "stamp") ===f ""
        "usr/local/tsk/any/stamp");
    }
  }
}

