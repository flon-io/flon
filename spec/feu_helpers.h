
//
// helpers for n/ and z/ specs
// (specs that run a dispatcher)
//
// Fri Oct 24 12:53:40 JST 2014
//


#include "djan.h"


void hlp_dispatcher_start();

void hlp_dispatcher_sighup();
  // when valgrinding and valgrind is right in charge, sighup terminates it...
  // maybe when half a second before sending...

void hlp_launch(char *exid, char *flow, char *payload);
void hlp_launch_v(char *exid, char *flow, char *payload, char *vars);
int hlp_cancel(char *exid, char *nid);

fdja_value *hlp_wait(char *exid, char *action, char *nid, int maxsec);

fdja_value *hlp_read_run_json(char *exid);
fdja_value *hlp_read_archive_run_json(char *exid);
fdja_value *hlp_read_node(char *exid, char *nid);
fdja_value *hlp_read_timer(char *exid, char *nid, char *type, char *ts);
fdja_value *hlp_read_tsk_log(char *exid);

double hlp_determine_delta(char *exid);

void hlp_cat_tsk_log(char *exid);
char *hlp_last_msg(char *exid);

void hlp_reset_tst(char flavour);

size_t hlp_count_jsons(const char *path, ...);

