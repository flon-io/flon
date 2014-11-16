
//
// specifying flon invokers
//
// Mon Nov 17 06:00:25 JST 2014
//

#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_invoker.h"


context "invoker: mirror"
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
      path = flu_sprintf("var/spool/inv/inv_%s-%s.json", exid, nid);

      flu_writeall(
        path,
        "invoke: [ mirror, {}, [] ]\n"
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

      expect(flu_fstat("var/spool/inv/inv_%s-%s.json", exid, nid) c== 'f');
        // it's still here, it's the dispatcher's work to nuke it,

      expect(flu_fstat("var/spool/dis/ret_%s-%s.json", exid, nid) c== 'f');
        // the null participant nuked it

      v = fdja_parse_f("var/spool/dis/ret_%s-%s.json", exid, nid);

      expect(v != NULL);

      //flu_putf(fdja_todc(v));
      expect(fdja_ls(v, "-e", NULL) ===f exid);
      expect(fdja_ls(v, "-n", NULL) ===f nid);
    }
  }
}

