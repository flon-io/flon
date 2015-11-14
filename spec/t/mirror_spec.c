
//
// specifying flon taskers
//
// Mon Nov 17 06:00:25 JST 2014
//

#include "flutim.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_tasker.h"

#include "flon_helpers.h"


describe "tasker:"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");

    char *exid = NULL;
    char *nid = NULL;
    char *path = NULL;

    fdja_value *v = NULL;
  }
  after each
  {
    free(exid);
    //free(nid); // no, it's not on the heap
    free(path);

    if (v) fdja_free(v);
  }

  describe "mirror"
  {
    it "repeats its command line args in its output"
    {
      exid = flon_generate_exid("ttest.mirror.0");
      nid = "0_1";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: mirror, from: executor }\n"
        "tree: [ task, { _0: mirror }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {\n"
          "hello: world\n"
          "danger0: \"; xrm ../nada0\"\n"
          "danger1: \"\\\"; xrm ../nada1\"\n"
        "}\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 0);

      r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-%s.json", exid, nid, 7);
      expect(r i== 1);

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/tsk/tsk_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it,
        // but since there is no answer...

      expect(flu_fstat("var/spool/dis/tsk_%s-%s.json", exid, nid) c== 'f');

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //
      if (v == NULL)
      {
        char *s = flu_readall("var/spool/dis/tsk_%s-%s.json", exid, nid);
        printf("var/spool/dis/tsk_%s-%s.json:\n", exid, nid);
        printf(">>\n%s\n<<\n", s);
      }
      //
      expect(v != NULL);

      //fdja_putdc(v);
      expect(fdja_ls(v, "-e", NULL) ===f exid);
      expect(fdja_ls(v, "-n", NULL) ===f nid);
      expect(fdja_ls(v, "-x", NULL) ===f "; xrm ../nada0");
      expect(fdja_ls(v, "-y", NULL) ===f "\"; xrm ../nada1");
    }
  }

  describe "mirrora"
  {
    it "requires the whole task"
    {
      exid = flon_generate_exid("ttest.mirrora.0");
      nid = "0_1";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: mirrora, from: executor }\n"
        "tree: [ task, { _0: mirrora }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {\n"
          "hello: \"mirror a\"\n"
        "}\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r == 0);

      r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-%s.json", exid, nid, 7);
      expect(r i== 1);

      flu_msleep(350);
        // give it time to write the file, especially when Valgrind is in

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/tsk/tsk_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it,
        // but since there is no answer...

      expect(flu_fstat("var/spool/dis/tsk_%s-%s.json", exid, nid) c== 'f');

      //char *s = flu_readall("var/spool/dis/ret_%s-%s.json", exid, nid);
      //printf("ret >%s<\n", s);
      //free(s);

      v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      expect(v != NULL);
      expect(fdja_ls(v, "exid", NULL) ===f exid);
      expect(fdja_ls(v, "payload.hello", NULL) ===f "mirror a");
    }
  }
}

