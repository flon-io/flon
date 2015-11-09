
//
// specifying flon-dispatcher
//
// Mon Oct  6 16:13:47 JST 2014
//

#include "flutil.h"
#include "flutim.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_common.h"
#include "fl_dispatcher.h"

#include "flon_helpers.h"


context "flon-dispatcher"
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
    char *fep = NULL;
    char *nid = NULL;
    char *name = NULL;
    char *path = NULL;
    char *s = NULL;
  }
  after each
  {
    free(exid);
    free(fep);
    free(nid);
    free(name);
    free(path);
    free(s);
  }

  describe "flon_dispatch()"
  {
    it "rejects files it doesn't understand"
    {
      exid = flon_generate_exid("dtest.rju");
      name = flu_sprintf("tsk_%s.json", exid);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(path, "NADA");
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== -1); // -1 rejected / 1 seen, failed / 2 dispatched

      s = flu_readall("var/spool/rejected/tsk_%s.json", exid);
      expect(s >== "NADA");
      expect(s >== "# reason: couldn't parse json (2");

      flu_unlink("var/spool/rejected/tsk_%s.json", exid);
    }

    it "rejects files it doesn't know how to dispatch"
    {
      exid = flon_generate_exid("dtest.rjk");
      name = flu_sprintf("nada_%s.json", exid);
      path = flu_sprintf("var/spool/dis/%s", name);

      int r = flu_writeall(path, "{ na: da }", exid);
      expect(r i== 1);

      r = flon_dispatch(name);
      expect(r i== -1); // -1 rejected / 1 seen, failed / 2 dispatched

      s = flu_readall("var/spool/rejected/nada_%s.json", exid);
      expect(s ^== "{ na: da }\n# reason: unknown file prefix (2");

      flu_unlink("var/spool/rejected/tsk_%s.json", exid);
    }

    it "doesn't launch a new executor if the previous is still here"
    {
      int r;
      exid = flon_generate_exid("d_test.xid");
      fep = flon_exid_path(exid);
      name = flu_sprintf("exe_%s.json", exid);

      r = flu_writeall(
        "var/spool/dis/exe_%s.json", exid,
        "{"
          "point: execute\n"
          "tree:"
            "[ sequence {} [ [ sequence {} [ [ trace { _0: a } [] ] ] ] ] ]\n"
          "exid: %s\n"
          "payload: {\n"
            "hello: d_test.xid\n"
          "}\n"
        "}", exid
      );
      expect(r i== 1);

      r = flu_writeall("var/run/%s.pid", exid, "%i", getpid());
      expect(r i== 1);
        //
        // pass the current pid (existing process to prevent executor from
        // being forked...

      r = flon_dispatch(name);
      expect(r i== 2); // -1 rejected / 1 seen, failed / 2 dispatched

      //printf("var/run/%s\n", fep);
      expect(flu_fstat("var/run/%s", fep) == 0);
      expect(flu_fstat("var/archive/%s", fep) == 0);
        // execution never ran
    }
  }
}

