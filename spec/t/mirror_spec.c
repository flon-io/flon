
//
// specifying flon taskers
//
// Mon Nov 17 06:00:25 JST 2014
//

#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_tasker.h"

#include "flon_helpers.h"


context "tasker: mirror"
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
      exid = flon_generate_exid("itest-mirror-0");
      nid = "0_1";
      path = flu_sprintf("var/spool/tsk/tsk_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "point: task\n"
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

      expect(r == 0);

      r = hlp_wait_for_file('f', "var/spool/dis/ret_%s-%s.json", exid, nid, 3);
      expect(r i== 1);

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/tsk/tsk_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it,
        // but since there is no answer...

      expect(flu_fstat("var/spool/dis/ret_%s-%s.json", exid, nid) c== 'f');

      //char *s = flu_readall("var/spool/dis/ret_%s-%s.json", exid, nid);
      //puts(s);
      //free(s);

      v = fdja_parse_f("var/spool/dis/ret_%s-%s.json", exid, nid);
      expect(v != NULL);

      //flu_putf(fdja_todc(v));
      expect(fdja_ls(v, "-e", NULL) ===f exid);
      expect(fdja_ls(v, "-n", NULL) ===f nid);
      expect(fdja_ls(v, "-x", NULL) ===f "; xrm ../nada0");
      expect(fdja_ls(v, "-y", NULL) ===f "\"; xrm ../nada1");
    }
  }
}

