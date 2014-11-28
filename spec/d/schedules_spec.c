
//
// specifying flon-dispatcher
//
// Fri Nov 28 10:47:15 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_common.h"
#include "fl_dispatcher.h"


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

//  describe "flon_dispatch()"
//  {
//    it "stores schedules in timers_at.json"
//    {
//      exid = flon_generate_exid("dtest.sch.one");
//      fep = flon_exid_path(exid);
//      name = flu_sprintf("sch_%s-0_2.json", exid);
//
//      int r = flu_writeall(
//        "var/spool/dis/%s", name,
//        "{"
//          "point: schedule\n"
//          "at: \"20141128.105313\"\n"
//          "exid: %s\n"
//          "nid: 0_2\n"
//          "msg: {\n"
//            "point: receive\n"
//            "exid: %s\n"
//            "nid: 0_2\n"
//          "}\n"
//        "}", exid, exid
//      );
//      expect(r i== 1);
//
//      r = flon_dispatch(name);
//
//      flu_system("tree var/ -I www");
//
//      expect(r i== 2);
//
//      //sleep(1);
//
//      //fdja_value *v = fdja_parse_f("var/spool/dis/ret_%s-0_2.json", exid);
//      //expect(v != NULL);
//      ////flu_putf(fdja_todc(v));
//      //expect(fdja_l(v, "stamp", NULL) != NULL);
//      //fdja_free(v);
//
//      //s = flu_readall("var/log/%s/inv_%s-0_2.log", fep, exid);
//      ////printf(">>>\n%s<<<\n", s);
//      //expect(s != NULL);
//      //expect(s >== " invoked >ruby stamp.rb<");
//      //expect(s >== " stamp.rb over.");
//
//      //expect(flu_fstat("var/spool/dis/%s", name) == 0);
//
//      //flu_unlink("var/spool/dis/ret_%s-0_2.json", exid);
//      //flu_unlink("var/spool/inv/inv_%s-0_2.json", exid);
//      //flu_unlink("var/log/inv/%s-0_2.txt", exid);
//    }
//  }
}

