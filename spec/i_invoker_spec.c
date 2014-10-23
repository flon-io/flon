
//
// specifying flon-invoker
//
// Fri Oct  3 11:24:25 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "djan.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_invoker.h"


context "flon-invoker"
{
  before each
  {
    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }

  describe "flon_invoke()"
  {
    it "invokes"
    {
      char *exid = flon_generate_exid("itest");
      char *nid = "0_0_7";
      char *path = flu_sprintf("var/spool/inv/inv_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "invoke: [ stamp, {}, [] ]\n"
        "exid: %s\n"
        "nid: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        exid, nid
      );

      int r = flon_invoke(path);

      expect(r == 0);

      sleep(1);

      expect(flu_canopath(".") $==f "/tst/");

      expect(flu_fstat("var/spool/inv/inv_%s-%s.json", exid, nid) == 'f');
        // it's still here, it's the dispatcher's work to nuke it

      expect(flu_fstat("var/spool/dis/ret_%s-%s.json", exid, nid) == 'f');

      fdja_value *v = fdja_parse_f("var/spool/dis/ret_%s-%s.json", exid, nid);
      //puts(fdja_to_pretty_djan(v));

      expect(fdja_ls(v, "hello", NULL) ===f "world");
      expect(fdja_l(v, "stamp") != NULL);

      fdja_free(v);

      flu_unlink("var/spool/inv/inv_%s-%s.json", exid, nid);
      flu_unlink("var/spool/dis/ret_%s-%s.json", exid, nid);
    }
  }
}

