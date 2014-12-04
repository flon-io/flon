
//
// specifying flon-executor
//
// Thu Dec  4 07:33:39 JST 2014
//

#include "flutil.h"
#include "gajeta.h"
#include "fl_ids.h"
#include "fl_paths.h"
#include "fl_tools.h"
#include "fl_common.h"
#include "fl_executor.h"


context "flon-executor"
{
  before each
  {
    char *exid = NULL;
    char *fep = NULL;
    fdja_value *v = NULL;

    fgaj_conf_get()->logger = fgaj_grey_logger;
    fgaj_conf_get()->level = 5;
    fgaj_conf_get()->out = stderr;
    fgaj_conf_get()->params = "5p";

    chdir("../tst");
    flon_configure(".");
  }
  after each
  {
    free(fep);
    free(exid);
    fdja_free(v);
  }

  describe "flon_schedule_msg()"
  {
    it "schedules an at msg"
    {
      exid = flon_generate_exid("xtest.fsm.at");
      execution_id = exid;

      flon_schedule_msg(
        "at", "20141224.203030", "0_0",
        fdja_v("[ wait, { _0: $(my_at)h }, [ 0 ]]"),
        fdja_v("[ wait, { _0: 1h }, [ 0 ]]"),
        fdja_v("{ point: receive, exid: %s, nid: 0_0 }", exid)
      );

      //flon_pp_execution(exid);

      v = fdja_parse_f("var/spool/dis/sch_%s-0_0.json", exid);

      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "point", NULL) ===f "schedule");
      expect(fdja_ls(v, "at", NULL) ===f "20141224.203030");

      expect(fdja_lj(v, "tree0") ===F fdja_vj(""
        "[ wait, { _0: $(my_at)h }, [ 0 ] ]"));
      expect(fdja_lj(v, "tree1") ===F fdja_vj(""
        "[ wait, { _0: 1h }, [ 0 ] ]"));
      expect(fdja_lj(v, "msg") ===F fdja_vj(""
        "{ point: receive, exid: %s, nid: 0_0 }", exid));
    }

    it "schedules a cron msg"
    {
      exid = flon_generate_exid("xtest.fsm.cron");
      execution_id = exid;

      flon_schedule_msg(
        "cron", "* * * * *", "0_0",
        fdja_v("[ cron, { _0: $(my_cron) }, [ 1 ]]"),
        fdja_v("[ cron, { _0: \"* * * * *\" }, [ 1 ]]"),
        fdja_v("{ point: execute, exid: %s, nid: 0_0, tree: [] }", exid)
      );

      //flon_pp_execution(exid);

      v = fdja_parse_f("var/spool/dis/sch_%s-0_0.json", exid);

      //flu_putf(fdja_todc(v));

      expect(fdja_ls(v, "point", NULL) ===f "schedule");
      expect(fdja_ls(v, "cron", NULL) ===f "* * * * *");

      expect(fdja_lj(v, "tree0") ===F fdja_vj(""
        "[ cron, { _0: $(my_cron) }, [ 1 ] ]"));
      expect(fdja_lj(v, "tree1") ===F fdja_vj(""
        "[ cron, { _0: \"* * * * *\" }, [ 1 ] ]"));
      expect(fdja_lj(v, "msg") ===F fdja_vj(""
        "{ point: execute, exid: %s, nid: 0_0, tree: [] }", exid));
    }
  }
}

