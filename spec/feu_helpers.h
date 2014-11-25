
//
// helpers for feu specs
// (specs that run a dispatcher)
//
// Fri Oct 24 12:53:40 JST 2014
//

#include "djan.h"


void hlp_dispatcher_start();

void hlp_launch(char *exid, char *flow, char *payload);
fdja_value *hlp_wait(char *exid, char *action, char *nid, int maxsec);

fdja_value *hlp_read_run_json(char *exid);
void hlp_cat_inv_log(char *exid);

