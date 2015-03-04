
//
// specifying instructions
//
// Thu Oct 30 12:39:39 JST 2014
//

#include "flutim.h"
#include "flutil.h"
#include "fl_ids.h"
#include "fl_tools.h"
#include "feu_helpers.h"


context "instruction:"
{
  before all
  {
    hlp_dispatcher_start();
  }

  before each
  {
    char *exid = NULL;
    char *fep = NULL;
    fdja_value *result = NULL;
    fdja_value *v = NULL;
    char *s = NULL;
  }
  after each
  {
    free(exid);
    free(fep);
    fdja_free(result);
    fdja_free(v);
    free(s);
  }

  describe "task"
  {
    it "tasks an external piece of code"
    {
      exid = flon_generate_exid("n.task.main");

      hlp_launch(
        exid,
        "task stamp\n"
        "",
        "{ hello: task }");

      result = hlp_wait(exid, "receive", "0", 9);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_ls(result, "point", NULL) ===f "receive");
      expect(fdja_ls(result, "nid", NULL) ===f "0");
      expect(fdja_ls(result, "from", NULL) == NULL);

      expect(fdja_ls(result, "payload.hello", NULL) ===f "task");
      expect(fdja_ls(result, "payload.stamp", NULL) ^==f "20");

      expect(fdja_l(result, "payload.args") == NULL);
    }

    it "passes arguments"
    {
      exid = flon_generate_exid("n.task.expand");

      hlp_launch(
        exid,
        "task copyargs\n"
        "",
        "{}");

      result = hlp_wait(exid, "terminated", NULL, 9);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_tod(fdja_l(result, "payload.args1")) ===f ""
        "{ _0: copyargs }");
    }

    it "expands its arguments"
    {
      exid = flon_generate_exid("n.task.expand");

      hlp_launch(
        exid,
        "task copyargs $(air), swiss: $(air.0), luft: $(air.1)\n"
        "",
        "{ air: [ sr, lh, em ] }");

      result = hlp_wait(exid, "terminated", NULL, 9);

      expect(result != NULL);
      //fdja_putdc(result);

      expect(fdja_l(result, "payload.args") == NULL);

      expect(fdja_ld(result, "payload.args1") ===f ""
        "{ _0: copyargs, _1: [ sr, lh, em ], swiss: sr, luft: lh }");
    }

    it "sets the node as 'failed' if the tasker doesn't exist"
    {
      exid = flon_generate_exid("n.task.unknown_tasker");

      hlp_launch(
        exid,
        "task nada\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      v = hlp_read_run_json(exid);
      //fdja_putdc(v);

      expect(fdja_ls(v, "nodes.0.status", NULL) ===f ""
        "failed");
      expect(fdja_ls(v, "nodes.0.errors.0.msg", NULL) ===f ""
        "didn't find tasker 'nada' (domain n.task.unknown_tasker)");
    }

    it "doesn't redispatch if the 'offered' receipt is for the same taskee"
    {
      exid = flon_generate_exid("n.task.offering");
      fep = flon_exid_path(exid);

      hlp_launch(
        exid,
        "task trash\n"
        "",
        "{}");

      result = hlp_wait(exid, "receive", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      flu_msleep(1400);

      v = hlp_read_run_json(exid);
      //fdja_putdc(v);

      expect(fdja_ls(v, "nodes.0.taskee", NULL) ===f ""
        "trash");

      expect(hlp_count_msgs(exid) zu== 3);
    }

    it "handles task refusal"
    {
      exid = flon_generate_exid("n.test.task.refusal");
      fep = flon_exid_path(exid);

      hlp_launch(
        exid,
        "task refuser\n"
        "",
        "{}");

      result = hlp_wait(exid, "receive", NULL, 7);

      expect(result != NULL);
      //fdja_putdc(result);

      flu_msleep(1400);

      s = flu_pline("grep receive var/run/%s/msg.log | wc -l", fep);
      size_t receive_count = strtoll(s, NULL, 10);

      expect(receive_count > 1);
    }

    it "tracks the current 'taskee'"
    {
      exid = flon_generate_exid("n.test.task.taskee");

      fep = flon_exid_path(exid);

      hlp_launch(
        exid,
        "task bounce0\n"
        "",
        "{}");

      result = hlp_wait(exid, "failed", NULL, 7);
      expect(result != NULL);

      expect(fdja_ls(result, "error.msg", NULL) ===f ""
        "didn't find tasker 'bounce1' (domain n.test.task.taskee)");

      v = hlp_read_node(exid, "0");
      //fdja_putdc(v);

      expect(fdja_ls(v, "taskee", NULL) ===f ""
        "bounce1");
    }
  }
}

