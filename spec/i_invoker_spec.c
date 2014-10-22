
//
// specifying flon-invoker
//
// Fri Oct  3 11:24:25 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
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

      flon_invoke(path);

      sleep(1);

      expect(flu_canopath(".") $==f "/tst/");

      char *s = flu_readall("var/spool/dis/ret_%s-%s.json", exid, nid);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, ",\"stamp\":\"") != NULL);

      flu_unlink("var/spool/inv/inv_%s-%s.json", exid, nid);
      flu_unlink("var/spool/dis/ret_%s-%s.json", exid, nid);

      //s = flu_readall("var/log/inv/%s.txt", id);
      ////printf(">>>\n%s<<<\n", s);
      //expect(s != NULL);
      //expect(strstr(s, " stamp.rb over.") != NULL);
      //
      //flu_unlink("var/log/inv/%s.txt", id);
    }
  }
}

