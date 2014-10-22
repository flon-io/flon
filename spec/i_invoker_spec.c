
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
      char *id = flon_generate_exid("itest");
      char *path = flu_sprintf("var/spool/inv/inv_%s.json", id);

      flu_writeall(
        path,
        "invoke: [ stamp, {}, [] ]\n"
        "id: %s\n"
        "payload: {\n"
          "hello: world\n"
        "}\n",
        id
      );

      flon_invoke(path);

      sleep(1);

      expect(flu_canopath(".") $==f "/tst/");

      char *s = flu_readall("var/spool/dis/ret_%s.json", id);
      //printf(">>>\n%s<<<\n", s);
      expect(s != NULL);
      expect(strstr(s, ",\"stamp\":\"") != NULL);

      flu_unlink("var/spool/inv/inv_%s.json", id);
      flu_unlink("var/spool/dis/ret_%s.json", id);

      //s = flu_readall("var/log/inv/%s.txt", id);
      ////printf(">>>\n%s<<<\n", s);
      //expect(s != NULL);
      //expect(strstr(s, " stamp.rb over.") != NULL);
      //
      //flu_unlink("var/log/inv/%s.txt", id);
    }
  }
}

