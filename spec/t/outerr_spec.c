
//
// specifying flon taskers
//
// Wed Mar  4 13:26:02 JST 2015
//

#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_tasker.h"


context "tasker:"
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
    fdja_free(v);
    free(exid);
    //free(nid); // no, it's not on the heap
    free(path);
  }

  describe "outerr"
  {
    it "outputs everything to stderr "
    {
      exid = flon_generate_exid("t.test.outerr");
      nid = "0_3";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: outerr, from: executor }\n"
        "tree: [ task, { _0: outerr }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {}\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 0);

      flu_msleep(770);

      expect(flu_fstat("var/spool/dis/tsk_%s-%s.json", exid, nid) c== 0);

      //r = hlp_wait_for_file('f', "var/spool/dis/tsk_%s-%s.json", exid, nid, 4);
      //expect(r i== 1);

      //v = fdja_parse_f("var/spool/dis/tsk_%s-%s.json", exid, nid);
      //fdja_putdc(v);

      //expect(v != NULL);
      //expect(fdja_ls(v, "hello", NULL) ===f ""
      //  "outparam");
      //expect(fdja_ls(v, "tasker_out", NULL) ===F flu_canopath(
      //  "var/spool/dis/tsk_%s-%s.json", exid, nid));
    }
  }
}

