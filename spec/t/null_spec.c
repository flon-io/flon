
//
// specifying flon taskers
//
// Sat Nov 15 17:50:46 JST 2014
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
  }
  after each
  {
    free(exid);
    //free(nid); // no, it's not on the heap
    free(path);
  }

  describe "null"
  {
    it "swallows any workitem"
    {
      exid = flon_generate_exid("t.test.null.0");
      nid = "0_1";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
        "task: { state: created, for: 'null', from: executor }\n"
        "tree: [ task, { _0: 'null' }, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: { hello: world }\n",
        exid, nid
      );

      int r = flon_task(path);

      expect(r i== 0);

      flu_msleep(490);

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/tsk/tsk_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it,
        // but since there is no answer...

      expect(flu_fstat("var/spool/dis/tsk_%s-%s.json", exid, nid) c== 0);
        // the null tasker prevented it
    }
  }
}

