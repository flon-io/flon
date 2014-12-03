
//
// helpers for feu specs
// (specs that run a dispatcher)
//
// Fri Oct 24 12:53:40 JST 2014
//

#include "djan.h"


void hlp_dispatcher_start();
void hlp_dispatcher_sighup();

void hlp_launch(char *exid, char *flow, char *payload);
fdja_value *hlp_wait(char *exid, char *action, char *nid, int maxsec);

fdja_value *hlp_read_run_json(char *exid);
fdja_value *hlp_read_node(char *exid, char *nid);

double hlp_determine_delta(char *exid);

void hlp_cat_inv_log(char *exid);
char *hlp_last_msg(char *exid);

void hlp_reset_tst();

size_t hlp_count_jsons(const char *path, ...);

